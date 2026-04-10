// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Global/ProjectRTSTypes.h"
#include "Components/ActorComponent.h"
#include "PlacementComponent.generated.h"

class APreviewBuilding;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API UPlacementComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlacementComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** 배치 모드 시작 */
    UFUNCTION(BlueprintCallable, Category = "RTS|Placement")
    void StartPlacement(FName BuildingRowName);

    /** 배치 확정 (클릭 시) */
    UFUNCTION(BlueprintCallable, Category = "RTS|Placement")
    void ConfirmPlacement();

protected:
    UPROPERTY(EditAnywhere, Category = "RTS|Settings")
    float TileSize = 100.0f;

    UPROPERTY()
    APreviewBuilding* PreviewActor; // 현재 마우스를 따라다니는 고스트 건물

    FST_Building CurrentBuildingData;
    bool bIsPlacementMode = false;

    // 매 프레임 설치 가능 여부를 갱신
    bool bCanPlaceCurrent = false;

    /** 타일 좌표 스냅 함수 */
    FVector SnapToGrid(FVector InLocation);
    bool CheckCanPlace(FVector InLocation);
};