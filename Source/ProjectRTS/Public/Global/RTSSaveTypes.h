// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Global/RTSUnitTypes.h" // FST_Attributes를 사용하기 위해 포함
#include "Global/RTSItemTypes.h" // ItemInstance / ItemSlot / InventorySlot 정의
#include "RTSSaveTypes.generated.h"

/** * 개별 인재(유닛)의 저장 데이터
 * 유닛의 고유 식별자와 무한 성장으로 쌓아온 결과물을 기록합니다.
 */
USTRUCT(BlueprintType)
struct FST_UnitSaveRecord
{
    GENERATED_BODY()

    FST_UnitSaveRecord()
    {
        EquipSlots.SetNum(static_cast<int32>(EUnitEquipSlot::Max));
    }

    static int32 GetEquipSlotIndex(EUnitEquipSlot SlotType)
    {
        return static_cast<int32>(SlotType);
    }

    FST_ItemSlot& GetEquipSlot(EUnitEquipSlot SlotType)
    {
        const int32 Index = GetEquipSlotIndex(SlotType);
        if (!EquipSlots.IsValidIndex(Index))
        {
            EquipSlots.SetNum(static_cast<int32>(EUnitEquipSlot::Max));
        }
        return EquipSlots[Index];
    }

    const FST_ItemSlot& GetEquipSlot(EUnitEquipSlot SlotType) const
    {
        static const FST_ItemSlot EmptySlot;
        const int32 Index = GetEquipSlotIndex(SlotType);
        return EquipSlots.IsValidIndex(Index) ? EquipSlots[Index] : EmptySlot;
    }

    /** 생성된 유니크한 유닛을 식별하기 위한 ID (세이브/로드, 네트워크 매핑에 사용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid UniqueId;

    // 1. 기초 고용 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName UnitRowName;

    /** 장착 슬롯 배열 (EUnitEquipSlot 인덱스 사용, PrimaryAssetId 기반) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FST_ItemSlot> EquipSlots;

    /** 인벤토리 슬롯 배열 (PrimaryAssetId 기반) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FST_InventorySlot> InventorySlots;

    /**
     * 이 유닛이 보유한 개별 인스턴스 아이템 맵.
     * 슬롯이 InstanceId를 가리키는 경우 이 맵에서 상태를 찾습니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FGuid, FST_ItemInstance> ItemInstances;

    // 2. 실시간 성장 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double CurrentExp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentJob;

    /** 무한 성장의 핵심: 누적된 순수 속성치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FST_Attributes AccumulatedAttributes;
};

/** * 전체 게임 세이브 데이터 구조
 * 보유한 인재 리스트와 자원, 진행도 등을 담습니다.
 */
USTRUCT(BlueprintType)
struct FST_SaveGameData
{
    GENERATED_BODY()

    /** 플레이어가 보유한 유닛(인재) 목록 (세이브용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FST_UnitSaveRecord> SavedUnitRoster;

    /** 보유 자원 (금광, 식량 등) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Gold;

    /** 현재 진행 중인 스테이지 또는 시나리오 정보 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentStageRowName;
};
