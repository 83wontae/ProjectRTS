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
    // bind to equipment update events globally (subscribe later when actors register)
}

void URecruitmentComponent::AddUnitToRosterFromSave(const FST_UnitSaveRecord& SaveData)
{
    if (SaveData.UnitRowName.IsNone()) return;

    FST_UnitSaveRecord NewUnit = SaveData;
    // 만약 세이브에 UniqueId가 비어있다면 새로 생성
    if (!NewUnit.UniqueId.IsValid()) NewUnit.UniqueId = FGuid::NewGuid();

    SavedUnitRoster.Add(NewUnit);

    if (!UniqueIdToActor.Contains(NewUnit.UniqueId))
    {
        UniqueIdToActor.Add(NewUnit.UniqueId, nullptr);
    }

    // ActiveUnits 맵에도 동기화
    FST_RuntimeUnitInfo ActiveInfo;
    ActiveInfo.UnitRowName = NewUnit.UnitRowName;
    ActiveInfo.CurrentJobRowName = NewUnit.CurrentJob;
    ActiveInfo.GrowthData.Level = NewUnit.Level;
    ActiveInfo.GrowthData.CurrentExp = NewUnit.CurrentExp;
    ActiveInfo.GrowthData.Name = NewUnit.Name;
    ActiveInfo.GrowthData.Attributes = NewUnit.AccumulatedAttributes;
    // Use slot RowName for runtime info
    ActiveInfo.EquipHandR = NewUnit.HandRSlot.RowName;
    ActiveInfo.EquipHandL = NewUnit.HandLSlot.RowName;
    // CurrentHp는 로드 시 StateComponent에서 초기화될 수 있으므로 0으로 둡니다.
    ActiveInfo.CurrentHp = 0.0;

    RuntimeUnitsById.Add(NewUnit.UniqueId, ActiveInfo);

    // ensure UniqueIdToActor has an entry (empty) for consistency
    if (!UniqueIdToActor.Contains(NewUnit.UniqueId))
    {
        UniqueIdToActor.Add(NewUnit.UniqueId, nullptr);
    }

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
    // Store as slots: populate RowName for new slot fields
    NewUnit.HandRSlot.RowName = HandR;
    NewUnit.HandLSlot.RowName = HandL;
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

void URecruitmentComponent::RegisterActorForUniqueId(const FGuid& Id, AActor* Actor)
{
    if (!Id.IsValid() || !Actor) return;

    UniqueIdToActor.Add(Id, Actor);
    ActorToUniqueId.Add(Actor, Id);

    // bind to actor destroyed to cleanup mappings
    Actor->OnDestroyed.AddDynamic(this, &URecruitmentComponent::HandleActorDestroyed);

    // If actor has an EquipComponent, bind to its update delegates for real-time sync
    if (Actor)
    {
        UEquipComponent* EquipComp = Actor->FindComponentByClass<UEquipComponent>();
        if (EquipComp)
        {
            // set UniqueId on EquipComponent so it can broadcast directly
            EquipComp->SetOwnerUniqueId(Id);
            // Bind to the UniqueId-aware delegates
            EquipComp->OnUpdateWeaponWithId.AddDynamic(this, &URecruitmentComponent::OnAnyEquipWeaponUpdated);
            EquipComp->OnUpdateArmorWithId.AddDynamic(this, &URecruitmentComponent::OnAnyEquipArmorUpdated);
        }
    }
}

bool URecruitmentComponent::TryGetActorByUniqueId(const FGuid& Id, AActor*& OutActor) const
{
    const TWeakObjectPtr<AActor>* Found = UniqueIdToActor.Find(Id);
    if (Found && Found->IsValid())
    {
        OutActor = Found->Get();
        return true;
    }
    OutActor = nullptr;
    return false;
}

void URecruitmentComponent::UnregisterActorForUniqueId(const FGuid& Id)
{
    if (!Id.IsValid()) return;

    const TWeakObjectPtr<AActor>* Found = UniqueIdToActor.Find(Id);
    if (Found && Found->IsValid())
    {
        AActor* Actor = Found->Get();
        // remove reverse map entry
        ActorToUniqueId.Remove(Actor);
        // remove binding - safe even if already unbound
        Actor->OnDestroyed.RemoveAll(this);
    }
    UniqueIdToActor.Remove(Id);
}

void URecruitmentComponent::OnAnyEquipWeaponUpdated(const FGuid& UniqueId, EWeaponSlot Slot, const FST_Weapon& NewWeapon)
{
    // Relay the unique-id-aware event to Blueprint listeners
    if (OnUnitEquipmentChanged.IsBound()) OnUnitEquipmentChanged.Broadcast(UniqueId);
}

void URecruitmentComponent::OnAnyEquipArmorUpdated(const FGuid& UniqueId, EEquipType Type, const FST_Armor& NewArmor)
{
    if (OnUnitEquipmentChanged.IsBound()) OnUnitEquipmentChanged.Broadcast(UniqueId);
}

void URecruitmentComponent::HandleActorDestroyed(AActor* DestroyedActor)
{
    if (!DestroyedActor) return;

    const FGuid* FoundId = ActorToUniqueId.Find(DestroyedActor);
    if (FoundId)
    {
        UniqueIdToActor.Remove(*FoundId);
        ActorToUniqueId.Remove(DestroyedActor);
    }
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
            // ensure actor's UniqueId is set and register mapping
            SpawnedUnit->UniqueId = RecruitData.UniqueId;
            RegisterActorForUniqueId(RecruitData.UniqueId, SpawnedUnit);
        }
    }
}