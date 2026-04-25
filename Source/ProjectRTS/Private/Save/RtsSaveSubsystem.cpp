// Fill out your copyright notice in the Description page of Project Settings.

#include "Save/RtsSaveSubsystem.h"
#include "Save/RtsSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Core/RtsUnitCharacter.h"
#include "Components/StateComponent.h"
#include "Components/RecruitmentComponent.h"

void URtsSaveSubsystem::SaveGame(const FString& SlotName, int32 UserIndex)
{
    URtsSaveGame* SaveGameObject = Cast<URtsSaveGame>(UGameplayStatics::CreateSaveGameObject(URtsSaveGame::StaticClass()));
    if (!SaveGameObject) return;

    // 1. 로컬 플레이어 컨트롤러와 고용 컴포넌트 찾기
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        URecruitmentComponent* RecruitComp = PC->FindComponentByClass<URecruitmentComponent>();
        if (RecruitComp)
        {
            // 2. 컴포넌트의 리스트를 세이브 데이터로 복사
            for (const FST_UnitSaveData& Unit : RecruitComp->GetRecruitedUnits())
            {
                FST_UnitSaveData SaveUnit;
                SaveUnit.UnitRowName = Unit.UnitRowName;
                SaveUnit.HandR = Unit.HandR;
                SaveUnit.HandL = Unit.HandL;

                SaveGameObject->SaveData.RecruitedUnits.Add(SaveUnit);
            }
        }
    }

    // 3. 파일 저장
    UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, UserIndex);
    UE_LOG(LogTemp, Log, TEXT("Roster Saved! Total Units: %d"), SaveGameObject->SaveData.RecruitedUnits.Num());
}

void URtsSaveSubsystem::LoadGame(const FString& SlotName, int32 UserIndex)
{
    URtsSaveGame* SaveGameObject = Cast<URtsSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    if (!SaveGameObject) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        URecruitmentComponent* RecruitComp = PC->FindComponentByClass<URecruitmentComponent>();
        if (RecruitComp)
        {
            // 기존 리스트 비우기
            RecruitComp->ClearRoster();

            // 세이브 파일의 데이터를 컴포넌트로 복구
            for (const FST_UnitSaveData& SaveUnit : SaveGameObject->SaveData.RecruitedUnits)
            {
                RecruitComp->AddUnitToRoster(SaveUnit.UnitRowName, SaveUnit.HandR, SaveUnit.HandL);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Roster Loaded Successfully."));
}

void URtsSaveSubsystem::SpawnUnitFromSaveData(const FST_UnitSaveData& UnitData)
{
    // 데이터 테이블에서 UnitRowName에 해당하는 클래스 정보를 가져와 스폰 로직을 진행합니다.
    // 1레벨로 생성한 뒤 StateComp->LoadFromSaveData(UnitData)를 호출하여 
    // 무한 성장 수치를 복구합니다.
}