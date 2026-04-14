// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlacementComponent.h"
#include "Actors/PreviewBuilding.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Actors/Building.h"
#include "Core/RtsPlayerController.h"

// Sets default values for this component's properties
UPlacementComponent::UPlacementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called every frame
void UPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bIsPlacementMode) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    FVector WorldLoc, WorldDir;

    if (PC && PC->DeprojectMousePositionToWorld(WorldLoc, WorldDir))
    {
        FVector TraceEnd = WorldLoc + (WorldDir * 100000.0f);
        FHitResult Hit;

        // 1. [중요] 오직 'Ground' 오브젝트 타입만 찾습니다. (건물 무시)
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel1); // Ground 채널

        if (GetWorld()->LineTraceSingleByObjectType(Hit, WorldLoc, TraceEnd, ObjectParams))
        {
            // 2. 마우스가 찍은 바닥 좌표를 그리드에 맞게 보정 (중앙 정렬 포함)
            FVector SnappedLoc = SnapToGrid(Hit.Location);

            // 3. 고스트 건물을 마우스 위치(보정됨)로 이동
            GetOwner()->SetActorLocation(SnappedLoc);

            // 4. 설치 가능 여부 체크 (Overlap은 여전히 작동하여 빨간색 표시)
            bCanPlaceCurrent = CheckCanPlace(SnappedLoc);
        }
    }
}

bool UPlacementComponent::InitializePlacementData(FName BuildingRowName)
{
    if (!BuildingDataTable) return false;

    FST_Building* FoundData = BuildingDataTable->FindRow<FST_Building>(BuildingRowName, TEXT(""));
    if (FoundData)
    {
        CurrentBuildingData = *FoundData;
        CurrentRowName = BuildingRowName;
        return true;
    }
    return false;
}

void UPlacementComponent::ActivatePlacementMode()
{
    bIsPlacementMode = true;
}

void UPlacementComponent::ConfirmPlacement()
{
    // 1. [가드 클로저] 설치 불가능하거나 데이터가 유효하지 않으면 즉시 종료
    if (!bCanPlaceCurrent || !CurrentBuildingData.BuildingClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // 2. [가드 클로저] 실제 건물 생성 시작 (Deferred Spawn 활용)
    // 위치는 현재 고스트(Owner)의 위치를 사용합니다.
    FTransform SpawnTransform(FRotator::ZeroRotator, GetOwner()->GetActorLocation());
    ABuilding* NewBuilding = World->SpawnActorDeferred<ABuilding>(
        CurrentBuildingData.BuildingClass,
        SpawnTransform
    );

    if (NewBuilding)
    {
        // 3. 건물 액터가 생성 직후(BeginPlay 전)에 RowName을 전달합니다.
        NewBuilding->BuildingRowName = CurrentRowName;
        NewBuilding->FinishSpawning(SpawnTransform);
		NewBuilding->InitializeBuilding(); // 데이터 테이블 기반 초기화 함수 호출 (ABuilding에서 구현)
    }
}

FVector UPlacementComponent::SnapToGrid(FVector InLocation)
{
    // 1. 데이터 테이블에서 가져온 건물 크기 (기본값 1)
    int32 SizeX = CurrentBuildingData.GridSizeX > 0 ? CurrentBuildingData.GridSizeX : 1;
    int32 SizeY = CurrentBuildingData.GridSizeY > 0 ? CurrentBuildingData.GridSizeY : 1;

    // 2. 기본 그리드 좌표 (Floor 기준)
    float GridX = FMath::FloorToFloat(InLocation.X / TileSize) * TileSize;
    float GridY = FMath::FloorToFloat(InLocation.Y / TileSize) * TileSize;

    // 3. 짝수/홀수에 따른 오프셋 계산
    // 홀수 크기: 타일 중앙(+50) / 짝수 크기: 격자 선(+0)
    float OffsetX = (SizeX % 2 != 0) ? (TileSize * 0.5f) : 0.0f;
    float OffsetY = (SizeY % 2 != 0) ? (TileSize * 0.5f) : 0.0f;

    // 최종 좌표 (Z값은 바닥에서 살짝 띄워주면 데칼 깜빡임 방지에 좋습니다)
    return FVector(GridX + OffsetX, GridY + OffsetY, InLocation.Z + 2.0f);
}

bool UPlacementComponent::CheckCanPlace(FVector InLocation)
{
    // 1. [가드 클로저] 메쉬 컴포넌트가 없으면 기본적으로 true 반환
    UStaticMeshComponent* MeshComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp) return true;

    // 2. Overlap 체크: 현재 메쉬와 겹치는 액터들을 가져옴
    TArray<AActor*> OverlappingActors;
    MeshComp->GetOverlappingActors(OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        // 3. [가드 클로저] 자기 자신이나 바닥(Static Mesh Actor 등)이 아닌 다른 액터가 있으면 설치 불가
        // 특정 클래스(예: ATerrain)만 제외하도록 필터링을 강화할 수 있습니다.
        if (Actor && Actor != GetOwner())
        {
            return false;
        }
    }

    return true;
}
