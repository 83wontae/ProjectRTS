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

    if (!bIsPlacementMode) return; // 배치 모드가 아닐 때는 무시

    AActor* Owner = GetOwner();
    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    if (Owner && PC)
    {
        FHitResult Hit;
        if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
        {
            // 1. 그리드 스냅 및 위치 업데이트 (AActor 기능만 사용)
            FVector SnappedLoc = SnapToGrid(Hit.Location);
            Owner->SetActorLocation(SnappedLoc);

            // 2. 설치 가능 여부 상태만 업데이트
            bCanPlaceCurrent = CheckCanPlace(SnappedLoc);

            // 더 이상 APreviewBuilding으로 Cast하여 함수를 호출하지 않습니다.
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

    return FVector(GridX + OffsetX, GridY + OffsetY, InLocation.Z);
}

bool UPlacementComponent::CheckCanPlace(FVector InLocation)
{
    // 1. 해당 위치에 다른 액터가 있는지 Overlap 체크
    // 2. 지형이 너무 가파르지 않은지 체크
    // 3. GameState의 점유 그리드 맵 확인
    return true;
}
