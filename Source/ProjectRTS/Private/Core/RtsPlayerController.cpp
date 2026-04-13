// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RtsPlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h" // 필수 헤더
#include "Kismet/GameplayStatics.h"       // 필수 헤더
#include "Actors/PreviewBuilding.h"        // 고스트 건물 클래스 헤더
#include "AIController.h" // AI 이동을 위해 필요
#include "Blueprint/AIBlueprintHelperLibrary.h" // Simple Move to Location을 위해 필요
#include "NavigationSystem.h" // 네비게이션 시스템 관련 기능을 위해 필요
#include "Interface/UnitInterface.h" // 유닛 인터페이스 호출을 위해 필요
#include "GameFramework/Character.h" // Character 캐스팅을 위해 필요
#include "Core/RtsPlayer.h" // Pawn 참조
#include "Global/ProjectRTSTypes.h" // ERtsInputMode 열거형 참조
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/PlacementComponent.h"


ARtsPlayerController::ARtsPlayerController():CurrentInputMode(ERtsInputMode::Tactical)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARtsPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 초기 모드 설정 (기본 IMC 추가)
    SetRtsInputMode(ERtsInputMode::Tactical);
}

void ARtsPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

}

void ARtsPlayerController::SetRtsInputMode(ERtsInputMode NewMode)
{
    auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if (!Subsystem) return;

    // 일단 다 지우고 시작 (상태 초기화)
    Subsystem->RemoveMappingContext(TacticalContext);
    Subsystem->RemoveMappingContext(PlacementContext);

    // 조건별 실행
    if (NewMode == ERtsInputMode::Tactical && TacticalContext)
    {
        Subsystem->AddMappingContext(TacticalContext, 1);
    }
    else if (NewMode == ERtsInputMode::Placement && PlacementContext)
    {
        Subsystem->AddMappingContext(PlacementContext, 1);
    }

    CurrentInputMode = NewMode;
}

bool ARtsPlayerController::IsInSideDragRect(FVector LocationToTest, FVector2D Start, FVector2D End)
{
    FVector2D ScreenPos;
    // 2. 월드 좌표를 위젯/화면 좌표로 변환 (Player Viewport Relative = true와 동일한 역할)
    // ProjectWorldLocationToWidgetPosition의 C++ 내부 구현을 활용하거나 아래 함수 사용
    if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(this, LocationToTest, ScreenPos, true) == false)
        return false;

    // 3. 앞서 만든 판정 로직 적용
    float MinX = FMath::Min(Start.X, End.X);
    float MaxX = FMath::Max(Start.X, End.X);
    float MinY = FMath::Min(Start.Y, End.Y);
    float MaxY = FMath::Max(Start.Y, End.Y);

    return (ScreenPos.X >= MinX && ScreenPos.X <= MaxX && ScreenPos.Y >= MinY && ScreenPos.Y <= MaxY);
}

void ARtsPlayerController::GetActorsInsideDragRect(TSubclassOf<UInterface> TargetInterface, FVector2D Start, FVector2D End, TArray<AActor*>& OutActors)
{
    OutActors.Empty();

    if (!TargetInterface) return;

    // 1. 월드 내 모든 액터를 가져옵니다. 
    // (성능을 위해 실제로는 모든 액터보다는 유닛들의 부모 클래스나 필터링된 리스트를 쓰는 것이 좋습니다)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), TargetInterface, FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor)
        {
            // 2. 이미 구현된 IsInSideDragRect 호출 (이전 답변에서 만든 함수)
            if (IsInSideDragRect(Actor->GetActorLocation(), Start, End))
            {
                OutActors.Add(Actor);
            }
        }
    }
}

void ARtsPlayerController::StartPlacementMode(FName BuildingRowName)
{
    // 1. 이미 다른 고스트가 있다면 제거 (중복 생성 방지)
    ClearCurrentPreview();

    if (!PreviewBuildingClass) return;

    // 2. 고스트 스폰
    FActorSpawnParameters SpawnParams;
    CurrentPreviewActor = GetWorld()->SpawnActor<APreviewBuilding>(PreviewBuildingClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

    if (CurrentPreviewActor)
    {
        // 3. 생성된 객체에 RowName을 전달 (그러면 PreviewBuilding이 스스로 초기화함)
        CurrentPreviewActor->SetupPreviewBuilding(BuildingRowName);
    }

	SetRtsInputMode(ERtsInputMode::Placement);
}

void ARtsPlayerController::ClearCurrentPreview()
{
    if (CurrentPreviewActor)
    {
        CurrentPreviewActor->Destroy();
        CurrentPreviewActor = nullptr; // 포인터 초기화 중요!
    }
}

void ARtsPlayerController::ConfirmPlacement()
{
    // 1. [가드 클로저] 프리뷰 액터가 없으면 즉시 종료
    if (!CurrentPreviewActor) return;

    // 2. [가드 클로저] 배치 컴포넌트가 없거나 설치 불가능한 상태면 종료
    UPlacementComponent* PlacementComp = CurrentPreviewActor->FindComponentByClass<UPlacementComponent>();
    if (!PlacementComp || !PlacementComp->IsCanPlace()) return;

    PlacementComp->ConfirmPlacement();

    ClearCurrentPreview();
    SetRtsInputMode(ERtsInputMode::Tactical);
}

void ARtsPlayerController::UnSelectedAllUnits()
{
    // 1. 선택된 모든 액터를 순회 (For Each Loop 부분)
    for (AActor* Actor : SelectedActors)
    {
        // 2. 인터페이스를 구현하고 있는지 확인 (Cast To UnitInterface 부분)
        if (Actor && Actor->Implements<UUnitInterface>())
        {
            // 3. 선택 해제 함수 호출 (Event Unit Selected 부분, IsSelected = false)
            // C++에서 블루프린트 인터페이스 함수 호출 시 'Execute_함수명' 형식을 사용합니다.
            IUnitInterface::Execute_EventUnitSelected(Actor, false);
        }
    }

    // 4. 배열 비우기 (Clear 부분)
    SelectedActors.Empty();
}

void ARtsPlayerController::DoSelectUnits(const TArray<AActor*>& InActors)
{
    // 1. 기존에 선택된 유닛들의 상태를 해제하고 배열을 비웁니다.
    UnSelectedAllUnits();

    // 2. 입력받은 액터 배열을 현재 선택 목록으로 교체합니다. (SET Selected Actors)
    SelectedActors = InActors;

    // 3. 새롭게 선택된 액터들을 순회하며 선택 상태를 'true'로 알립니다.
    for (AActor* Actor : SelectedActors)
    {
        if (Actor && Actor->Implements<UUnitInterface>())
        {
            // IsSelected 인자를 true로 하여 인터페이스 함수를 호출합니다.
            IUnitInterface::Execute_EventUnitSelected(Actor, true);
        }
    }
}

void ARtsPlayerController::DoSelectUnitSingle()
{
    FHitResult Hit;
    // 1. 마우스 커서 아래의 가시성(Visibility) 채널 기반 히트 체크
    // bTraceComplex를 true로 설정하여 복잡한 충돌체도 정확히 체크합니다.
    if (GetHitResultUnderCursor(ECC_Visibility, true, Hit))
    {
        AActor* HitActor = Hit.GetActor();

        // 2. 히트된 액터가 유닛 인터페이스를 구현하고 있는지 확인
        if (HitActor && HitActor->Implements<UUnitInterface>())
        {
            // 3. 선택된 액터 배열에 추가 (ADD)
            SelectedActors.Add(HitActor);

            // 4. 해당 유닛에게 선택되었음을 알림 (Event Unit Selected, IsSelected = true)
            IUnitInterface::Execute_EventUnitSelected(HitActor, true);
        }
    }
}

void ARtsPlayerController::MoveUnits(FVector Goal)
{
    // 1. 현재 선택된 액터들을 순회합니다. (For Each Loop)
    for (AActor* Actor : SelectedActors)
    {
        // 2. 액터를 Character로 캐스팅합니다. (Cast To Character)
        ACharacter* SelectedChar = Cast<ACharacter>(Actor);
        if (SelectedChar)
        {
            // 3. 캐릭터의 컨트롤러를 가져옵니다. (Get Controller)
            AController* CharController = SelectedChar->GetController();
            if (CharController)
            {
                // 4. AI 기능을 사용하여 목표 지점으로 이동시킵니다. (Simple Move to Location)
                // 네비게이션 메시(NavMesh)가 깔려 있어야 작동합니다.
                UAIBlueprintHelperLibrary::SimpleMoveToLocation(CharController, Goal);
            }
        }
    }
}