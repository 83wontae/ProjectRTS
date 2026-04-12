// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PlacementComponent.h"
#include "Actors/PreviewBuilding.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

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

void UPlacementComponent::StartPlacement(FName BuildingRowName)
{
    // 1. 데이터 테이블에서 정보 로드 (생략)

    bIsPlacementMode = true;
}

void UPlacementComponent::ConfirmPlacement()
{
    if (bCanPlaceCurrent && CurrentBuildingData.BuildingClass)
    {
        GetWorld()->SpawnActor<AActor>(CurrentBuildingData.BuildingClass, GetOwner()->GetActorLocation(), FRotator::ZeroRotator);
        GetOwner()->Destroy();
    }
}

FVector UPlacementComponent::SnapToGrid(FVector InLocation)
{
    // FloorToFloat을 사용하면 0~99.9 사이의 모든 값은 0이 됩니다.
    // 거기에 0.5(절반)를 더하면 해당 타일의 정중앙에 고정됩니다.
    float SnappedX = FMath::FloorToFloat(InLocation.X / TileSize) * TileSize + (TileSize * 0.5f);
    float SnappedY = FMath::FloorToFloat(InLocation.Y / TileSize) * TileSize + (TileSize * 0.5f);

    // [팁] Z값은 지면 높이에 고정하는 것이 좋습니다. 
    // 마우스가 유닛을 가리킬 때 건물이 공중에 뜨는 것을 방지합니다.
    return FVector(SnappedX, SnappedY, InLocation.Z);
}

bool UPlacementComponent::CheckCanPlace(FVector InLocation)
{
    // 1. 해당 위치에 다른 액터가 있는지 Overlap 체크
    // 2. 지형이 너무 가파르지 않은지 체크
    // 3. GameState의 점유 그리드 맵 확인
    return true;
}
