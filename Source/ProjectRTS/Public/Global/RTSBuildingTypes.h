// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RTSBuildingTypes.generated.h"

/** * 건물 기초 데이터 (DT_Building용)
 * 건설될 건물의 클래스, 외형, 그리드 크기 등을 정의합니다.
 */
USTRUCT(BlueprintType)
struct FST_Building : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FString Name;

    /** 실제로 월드에 배치될 건물 액터 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TSubclassOf<class ABuilding> BuildingClass;

    /** 건설 전 '고스트(Ghost)' 상태로 보여줄 스태틱 메시 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    class UStaticMesh* BuildingMesh;

    /** 타일 시스템에서의 크기 (예: 1x1, 2x2) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 GridSizeX = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 GridSizeY = 1;
};