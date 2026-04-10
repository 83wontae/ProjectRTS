// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlacementComponent.h"
#include "Actors/PreviewBuilding.h"

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

    if (bIsPlacementMode && PreviewActor)
    {
        APlayerController* PC = Cast<APlayerController>(GetOwner());
        FHitResult Hit;
        if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
        {
            FVector SnappedLoc = SnapToGrid(Hit.Location);
            PreviewActor->SetActorLocation(SnappedLoc);

            // 설치 가능 여부에 따라 고스트 색상 변경 (생략: 머티리얼 파라미터 조절)
            bCanPlaceCurrent = CheckCanPlace(SnappedLoc);
        }
    }
}

void UPlacementComponent::StartPlacement(FName BuildingRowName)
{
    // 1. 데이터 테이블에서 정보 로드 (생략)

    // 2. 고스트 액터 스폰
    FActorSpawnParameters SpawnParams;
    PreviewActor = GetWorld()->SpawnActor<APreviewBuilding>(APreviewBuilding::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

    if (PreviewActor)
    {
        PreviewActor->SetPreviewMesh(CurrentBuildingData.PreviewMesh);
        bIsPlacementMode = true;
    }
}

void UPlacementComponent::ConfirmPlacement()
{
    if (bIsPlacementMode && PreviewActor && bCanPlaceCurrent)
    {
        // 3. 고스트 액터 위치에 실제 건물 스폰
        FVector SpawnLoc = PreviewActor->GetActorLocation();
        GetWorld()->SpawnActor<AActor>(CurrentBuildingData.BuildingClass, SpawnLoc, FRotator::ZeroRotator);

        // 4. 고스트 제거 및 모드 종료
        PreviewActor->Destroy();
        PreviewActor = nullptr;
        bIsPlacementMode = false;
    }
}

FVector UPlacementComponent::SnapToGrid(FVector InLocation)
{
    // 타일 크기에 맞춰 좌표를 끊어줍니다.
    float SnappedX = FMath::RoundToInt(InLocation.X / TileSize) * TileSize;
    float SnappedY = FMath::RoundToInt(InLocation.Y / TileSize) * TileSize;

    // 건물의 Pivot 위치에 따라 Z값을 조정하거나 중심점을 맞춥니다.
    return FVector(SnappedX, SnappedY, InLocation.Z);
}

bool UPlacementComponent::CheckCanPlace(FVector InLocation)
{
    // 1. 해당 위치에 다른 액터가 있는지 Overlap 체크
    // 2. 지형이 너무 가파르지 않은지 체크
    // 3. GameState의 점유 그리드 맵 확인
    return true;
}
