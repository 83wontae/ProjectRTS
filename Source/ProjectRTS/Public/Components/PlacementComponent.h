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

    // 데이터를 외부(액터)에서 가져갈 수 있도록 Getter 추가
    const FST_Building& GetCurrentBuildingData() const { return CurrentBuildingData; }

    // 데이터 로드만 담당하는 함수로 변경
    bool InitializePlacementData(FName BuildingRowName);

    /** 배치 모드 시작 */
    void ActivatePlacementMode();

    /** 배치 확정 (클릭 시) */
    UFUNCTION(BlueprintCallable, Category = "RTS|Placement")
    void ConfirmPlacement();

    UFUNCTION(BlueprintCallable, Category = "RTS|Placement")
    bool IsCanPlace() const { return bCanPlaceCurrent; }

protected:
    // 에디터에서 할당할 건물 데이터 테이블
    UPROPERTY(EditAnywhere, Category = "RTS|Settings")
    class UDataTable* BuildingDataTable;

    UPROPERTY(EditAnywhere, Category = "RTS|Settings")
    float TileSize = 100.0f;

    UPROPERTY()
    APreviewBuilding* PreviewActor; // 현재 마우스를 따라다니는 고스트 건물

    // 현재 배치 중인 건물의 데이터 테이블 행 이름 저장
    FName CurrentRowName;

    FST_Building CurrentBuildingData;
    bool bIsPlacementMode = false;

    // 매 프레임 설치 가능 여부를 갱신
    bool bCanPlaceCurrent = false;

    /** 타일 좌표 스냅 함수 */
    FVector SnapToGrid(FVector InLocation);
    bool CheckCanPlace(FVector InLocation);
};