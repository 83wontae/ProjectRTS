// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/RecruitmentComponent.h"
#include "Misc/Guid.h"
#include "Core/RtsUnitCharacter.h"
#include "Components/StateComponent.h" // 추가
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Global/RtsGameSettings.h"
#include "Kismet/KismetMathLibrary.h"

URecruitmentComponent::URecruitmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URecruitmentComponent::AddUnitToRosterFromSave(const FST_UnitSaveRecord& SaveData)
{
    if (SaveData.UnitRowName.IsNone()) return;

    FST_UnitSaveRecord NewUnit = SaveData;
    // 만약 세이브에 UniqueId가 비어있다면 새로 생성
    if (!NewUnit.UniqueId.IsValid()) NewUnit.UniqueId = FGuid::NewGuid();

    SavedUnitRoster.Add(NewUnit);

    // ActiveUnits 맵에도 동기화
    FST_RuntimeUnitInfo ActiveInfo;
    ActiveInfo.UnitRowName = NewUnit.UnitRowName;
    ActiveInfo.CurrentJobRowName = NewUnit.CurrentJob;
    ActiveInfo.GrowthData.Level = NewUnit.Level;
    ActiveInfo.GrowthData.CurrentExp = NewUnit.CurrentExp;
    ActiveInfo.GrowthData.Name = NewUnit.Name;
    ActiveInfo.GrowthData.Attributes = NewUnit.AccumulatedAttributes;
    ActiveInfo.EquipHandR = NewUnit.HandR;
    ActiveInfo.EquipHandL = NewUnit.HandL;
    // CurrentHp는 로드 시 StateComponent에서 초기화될 수 있으므로 0으로 둡니다.
    ActiveInfo.CurrentHp = 0.0;

    RuntimeUnitsById.Add(NewUnit.UniqueId, ActiveInfo);

    if (OnUnitRecruited.IsBound())
    {
        OnUnitRecruited.Broadcast(NewUnit);
    }
}

void URecruitmentComponent::AddUnitToRoster(FName UnitRowName, FName HandR, FName HandL)
{
    if (UnitRowName.IsNone()) return;

    // 1. 초기화된 통합 구조체 생성
    FST_UnitSaveRecord NewUnit;
    NewUnit.UnitRowName = UnitRowName;
    NewUnit.HandR = HandR;
    NewUnit.HandL = HandL;
    NewUnit.Level = 1; // 신규 고용은 1레벨부터
    NewUnit.UniqueId = FGuid::NewGuid();

    SavedUnitRoster.Add(NewUnit);

    if (OnUnitRecruited.IsBound())
    {
        OnUnitRecruited.Broadcast(NewUnit);
    }
}

bool URecruitmentComponent::TryGetActiveUnit(const FGuid& Id, FST_RuntimeUnitInfo& OutInfo) const
{
    const FST_RuntimeUnitInfo* Found = RuntimeUnitsById.Find(Id);
    if (Found)
    {
        OutInfo = *Found;
        return true;
    }
    return false;
}

void URecruitmentComponent::SpawnRecruitedUnits(FName StartTag, float SpawnRadius)
{
    UDataTable* UnitDataTable = RtsSettings::GetUnitTable(GetOwner());
    if (!UnitDataTable) return;

    // 2. PlayerStart 찾기
    APlayerStart* TargetStart = nullptr;
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        if (It->PlayerStartTag == StartTag) { TargetStart = *It; break; }
    }
    if (!TargetStart) return;

    FVector CenterLocation = TargetStart->GetActorLocation();

    // 3. 리스트의 모든 유닛 순회하며 스폰
    for (const FST_UnitSaveRecord& RecruitData : SavedUnitRoster)
    {
		FST_Unit* UnitInfo = UnitDataTable->FindRow<FST_Unit>(RecruitData.UnitRowName, TEXT("SpawnRecruitedUnits"));
        // 유효성 및 클래스 설정 확인
        if (!UnitInfo || !UnitInfo->UnitClass) continue;

        // 4. 위치 계산
        FVector2D RandomCircle = FMath::RandPointInCircle(SpawnRadius);
        FVector SpawnLocation = CenterLocation + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // 5. [수정 포인트] 데이터 테이블에 정의된 클래스로 스폰을 실행합니다.
        ARtsUnitCharacter* SpawnedUnit = GetWorld()->SpawnActor<ARtsUnitCharacter>(
            UnitInfo->UnitClass,
            SpawnLocation,
            TargetStart->GetActorRotation(),
            SpawnParams
        );

        if (SpawnedUnit)
        {
            SpawnedUnit->SpawnDefaultController(); // AI 빙의

            // 6. 상태 및 장비 복구 (통합 데이터 주입)
            if (SpawnedUnit->StateComp)
            {
                SpawnedUnit->StateComp->LoadFromFullSaveData(RecruitData);
            }
        }
    }
}