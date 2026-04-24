// Fill out your copyright notice in the Description page of Project Settings.

#include "Save/RtsSaveSubsystem.h"
#include "Save/RtsSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Core/RtsUnitCharacter.h"
#include "Components/StateComponent.h"

void URtsSaveSubsystem::SaveGame(const FString& SlotName, int32 UserIndex)
{
    // 1. 새로운 세이브 객체 생성
    URtsSaveGame* SaveGameObject = Cast<URtsSaveGame>(UGameplayStatics::CreateSaveGameObject(URtsSaveGame::StaticClass()));
    if (!SaveGameObject) return;

    // 2. 월드 내 모든 ARtsUnitCharacter 탐색 및 데이터 수집
    for (TActorIterator<ARtsUnitCharacter> It(GetWorld()); It; ++It)
    {
        ARtsUnitCharacter* Unit = *It;
        if (Unit && Unit->StateComp && !Unit->IsDeath_Implementation()) // 살아있는 유닛만 저장
        {
            SaveGameObject->SaveData.RecruitedUnits.Add(Unit->StateComp->GetSaveData());
        }
    }

    // 3. 기타 게임 데이터 저장 (자원 등)
    // SaveGameObject->SaveData.Gold = CurrentGold;

    // 4. 파일로 쓰기
    UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, UserIndex);
    UE_LOG(LogTemp, Log, TEXT("Game Saved to Slot: %s"), *SlotName);
}

void URtsSaveSubsystem::LoadGame(const FString& SlotName, int32 UserIndex)
{
    // 1. 파일 읽기
    URtsSaveGame* SaveGameObject = Cast<URtsSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    if (!SaveGameObject) return;

    // 2. 기존 유닛 제거 (새로 스폰하여 복구할 예정이므로)
    for (TActorIterator<ARtsUnitCharacter> It(GetWorld()); It; ++It)
    {
        It->Destroy();
    }

    // 3. 저장된 데이터를 바탕으로 유닛 재생성
    for (const FST_UnitSaveData& UnitData : SaveGameObject->SaveData.RecruitedUnits)
    {
        SpawnUnitFromSaveData(UnitData);
    }
}

void URtsSaveSubsystem::SpawnUnitFromSaveData(const FST_UnitSaveData& UnitData)
{
    // 데이터 테이블에서 UnitRowName에 해당하는 클래스 정보를 가져와 스폰 로직을 진행합니다.
    // 1레벨로 생성한 뒤 StateComp->LoadFromSaveData(UnitData)를 호출하여 
    // 무한 성장 수치를 복구합니다.
}