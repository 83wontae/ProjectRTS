// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Global/ProjectRTSTypes.h"
#include "RtsSaveSubsystem.generated.h"

/**
 * 게임의 저장 및 불러오기 로직을 총괄하는 서브시스템입니다.
 */
UCLASS()
class PROJECTRTS_API URtsSaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** 현재 월드의 유닛 정보를 수집하여 파일로 저장합니다. */
    UFUNCTION(BlueprintCallable, Category = "RTS|Save")
    void SaveGame(const FString& SlotName, int32 UserIndex = 0);

    /** 파일을 읽어와 월드에 유닛들을 스폰하고 상태를 복구합니다. */
    UFUNCTION(BlueprintCallable, Category = "RTS|Save")
    void LoadGame(const FString& SlotName, int32 UserIndex = 0);

protected:
    /** 특정 유닛 하나를 세이브 데이터로부터 스폰하는 헬퍼 함수 */
    void SpawnUnitFromSaveData(const FST_UnitSaveData& UnitData);

    /** 유닛 정보를 담고 있는 데이터 테이블 (유닛 클래스 참조용) */
    UPROPERTY()
    class UDataTable* UnitDataTable;
};