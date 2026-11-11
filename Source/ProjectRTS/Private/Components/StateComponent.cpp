#include "Components/StateComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/DebugWidgetComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/RtsUnitCharacter.h"
#include "Components/EquipComponent.h" // 장비 정보 접근을 위해 추가
#include "Global/RtsGameSettings.h"
#include "Global/RTSUnitJobDefinition.h"

UStateComponent::UStateComponent() : m_CurHp(100.0)
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UStateComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerChar = Cast<ACharacter>(GetOwner());

    if (m_JobRowName.IsNone()) m_JobRowName = TEXT("Novice");

    URTSUnitJobDefinition* JobDefinition = RtsSettings::GetJobDefinition(GetOwner(), m_JobRowName);
    if (!JobDefinition) {
        UE_LOG(LogTemp, Error, TEXT("[%s] StateComponent: JobDefinition '%s' not found."), *GetOwner()->GetName(), *m_JobRowName.ToString());
		return;
	}

    EnsureCurrentJobProficiencyInitialized();

    RefreshFinalStats();

    m_CurHp = m_TotalCombatStats.MaxHp;
    OnRep_CurHp();
}

void UStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UStateComponent, m_CurHp);
    DOREPLIFETIME(UStateComponent, m_AggroTarget);
}

double UStateComponent::GetRequiredExpForLevel(int32 Level) const
{
   return 5.0 * FMath::Pow((float)Level, 2.0f);
}

double UStateComponent::CalculateExpReward() const
{
    return m_BaseExpReward * (1.0 + (m_CurrentLevel * 0.1));
}

void UStateComponent::AddExp(double Amount)
{
    if (Amount <= 0.0) return;

    m_CurrentExp += Amount;

    double RequiredExp = GetRequiredExpForLevel(m_CurrentLevel);

    while (m_CurrentExp >= RequiredExp)
    {
        m_CurrentExp -= RequiredExp;

        m_CurrentLevel++;

        if (EventDispatcher_LevelUp.IsBound())
        {
            EventDispatcher_LevelUp.Broadcast(m_CurrentLevel);
        }

        RefreshFinalStats();

        RequiredExp = GetRequiredExpForLevel(m_CurrentLevel);
    }

    if (EventDispatcher_ExpChanged.IsBound())
    {
        EventDispatcher_ExpChanged.Broadcast(m_CurrentExp, RequiredExp);
    }

    UpdateDebugWidget();
}

void UStateComponent::AddStatExp(EUnitStatType StatType, double Amount)
{
    ApplyStatExpToAttribute(StatType, Amount);
}

void UStateComponent::AddStrengthActionExp(double Amount)
{
    ApplyStatExpToAttribute(EUnitStatType::STR, Amount);
}

void UStateComponent::AddDexterityActionExp(double Amount)
{
    ApplyStatExpToAttribute(EUnitStatType::DEX, Amount);
}

void UStateComponent::AddIntelligenceActionExp(double Amount)
{
    ApplyStatExpToAttribute(EUnitStatType::INT, Amount);
}

void UStateComponent::ApplyStatExpToAttribute(EUnitStatType StatType, double Amount)
{
    if (Amount <= 0.0)
    {
        return;
    }

    double RequiredExp = 100.0;
    constexpr double MinStatValue = 1.0;
    constexpr double MaxStatValue = 999.0;

    double* TargetExp = nullptr;
    double* TargetAccumulated = nullptr;

    switch (StatType)
    {
    case EUnitStatType::STR:
        TargetExp = &m_StrExp;
        TargetAccumulated = &m_AccumulatedAttributes.Strength;
        RequiredExp = FMath::Max(1.0, m_StrExpPerPoint);
        break;
    case EUnitStatType::DEX:
        TargetExp = &m_DexExp;
        TargetAccumulated = &m_AccumulatedAttributes.Dexterity;
        RequiredExp = FMath::Max(1.0, m_DexExpPerPoint);
        break;
    case EUnitStatType::INT:
        TargetExp = &m_IntExp;
        TargetAccumulated = &m_AccumulatedAttributes.Intelligence;
        RequiredExp = FMath::Max(1.0, m_IntExpPerPoint);
        break;
    default:
        return;
    }

    if (!TargetExp || !TargetAccumulated)
    {
        return;
    }

    *TargetExp += Amount;

    while (*TargetExp >= RequiredExp)
    {
        const double CurrentValue = FMath::Clamp(MinStatValue + *TargetAccumulated, MinStatValue, MaxStatValue);
        if (CurrentValue >= MaxStatValue)
        {
            *TargetExp = 0.0;
            break;
        }

        *TargetExp -= RequiredExp;
        *TargetAccumulated += 1.0;
    }

    m_AccumulatedAttributes.Strength = FMath::Clamp(m_AccumulatedAttributes.Strength, 0.0, MaxStatValue - MinStatValue);
    m_AccumulatedAttributes.Dexterity = FMath::Clamp(m_AccumulatedAttributes.Dexterity, 0.0, MaxStatValue - MinStatValue);
    m_AccumulatedAttributes.Intelligence = FMath::Clamp(m_AccumulatedAttributes.Intelligence, 0.0, MaxStatValue - MinStatValue);

    RefreshFinalStats();
    UpdateDebugWidget();
}

void UStateComponent::RefreshFinalStats()
{
    m_CurrentAttributes = m_OriginAttributes + m_AccumulatedAttributes;
    m_CurrentAttributes.Strength = FMath::Clamp(m_CurrentAttributes.Strength, 1.0, 999.0);
    m_CurrentAttributes.Dexterity = FMath::Clamp(m_CurrentAttributes.Dexterity, 1.0, 999.0);
    m_CurrentAttributes.Intelligence = FMath::Clamp(m_CurrentAttributes.Intelligence, 1.0, 999.0);

    UpdateCombatStats();
}

void UStateComponent::UpdateCombatStats()
{
    FST_CombatStats NewBase;
    NewBase.Attack = (m_CurrentAttributes.Strength * 1.0) + (m_CurrentAttributes.Dexterity * 0.2);
    NewBase.MaxHp = 100.0 + m_CurrentAttributes.Strength;
    NewBase.Defend = (m_CurrentAttributes.Dexterity * 1.2);
    NewBase.Speed = 500.0 + (m_CurrentAttributes.Dexterity * 0.1);

    m_BaseCombatStats = bUseBaseCombatStatsOverride ? m_BaseCombatStatsOverride : NewBase;

    RecalculateTotalStats();
}

void UStateComponent::ChangeJob(FName NewJobRowName)
{
    m_JobRowName = NewJobRowName;
    EnsureCurrentJobProficiencyInitialized();
}

void UStateComponent::AddJobProficiencyExp(double Amount)
{
    if (Amount <= 0.0 || m_JobRowName.IsNone())
    {
        return;
    }

    EnsureCurrentJobProficiencyInitialized();

    URTSUnitJobDefinition* JobDefinition = RtsSettings::GetJobDefinition(GetOwner(), m_JobRowName);
    const double RequiredExp = (JobDefinition && JobDefinition->ProficiencyExpPerLevel > 0.0)
        ? JobDefinition->ProficiencyExpPerLevel
        : FMath::Max(1.0, m_DefaultJobProficiencyExpPerLevel);
    const int32 MaxLevel = (JobDefinition && JobDefinition->MaxProficiencyLevel > 0)
        ? JobDefinition->MaxProficiencyLevel
        : FMath::Max(1, m_DefaultJobMaxProficiencyLevel);

    FST_JobProgress& CurrentProgress = m_JobProgressMap.FindOrAdd(m_JobRowName);
    int32& CurrentLevel = CurrentProgress.Level;
    double& CurrentExp = CurrentProgress.CurrentExp;

    if (CurrentLevel >= MaxLevel)
    {
        CurrentLevel = MaxLevel;
        CurrentExp = 0.0;
        UpdateDebugWidget();
        return;
    }

    CurrentExp += Amount;
    while (CurrentExp >= RequiredExp && CurrentLevel < MaxLevel)
    {
        CurrentExp -= RequiredExp;
        CurrentLevel++;
    }

    if (CurrentLevel >= MaxLevel)
    {
        CurrentLevel = MaxLevel;
        CurrentExp = 0.0;
    }

    UpdateDebugWidget();
}

int32 UStateComponent::GetCurrentJobProficiencyLevel() const
{
    if (const FST_JobProgress* FoundProgress = m_JobProgressMap.Find(m_JobRowName))
    {
        return FoundProgress->Level;
    }
    return 1;
}

double UStateComponent::GetCurrentJobProficiencyExp() const
{
    if (const FST_JobProgress* FoundProgress = m_JobProgressMap.Find(m_JobRowName))
    {
        return FoundProgress->CurrentExp;
    }
    return 0.0;
}

void UStateComponent::EnsureCurrentJobProficiencyInitialized()
{
    if (m_JobRowName.IsNone())
    {
        return;
    }

    if (!m_JobProgressMap.Contains(m_JobRowName))
    {
        m_JobProgressMap.Add(m_JobRowName, FST_JobProgress());
    }
}

void UStateComponent::AddDamage(AController* EventInstigator, double Damage)
{
    if (IsDeath() || GetOwnerRole() != ROLE_Authority) return;

    m_CurHp = FMath::Clamp(m_CurHp - Damage, 0.0, m_TotalCombatStats.MaxHp);
    OnRep_CurHp();

    if (IsDeath())
    {
        if (EventInstigator && EventInstigator->GetPawn())
        {
            UStateComponent* KillerState = EventInstigator->GetPawn()->FindComponentByClass<UStateComponent>();
            if (KillerState)
            {
                double FinalXP = CalculateExpReward();
                KillerState->AddExp(FinalXP);
            }
        }

        HandleDeath();
    }
}

bool UStateComponent::IsDeath() const
{
    return m_CurHp <= 0.0;
}

void UStateComponent::SetEquipCombatStats(const FST_CombatStats& NewEquipStats)
{
    m_EquipCombatStats = NewEquipStats;

    RecalculateTotalStats();
}

void UStateComponent::SetBaseCombatStatsOverride(const FST_CombatStats& NewBaseCombatStats)
{
    bUseBaseCombatStatsOverride = true;
    m_BaseCombatStatsOverride = NewBaseCombatStats;
    m_BaseCombatStats = m_BaseCombatStatsOverride;

    RecalculateTotalStats();
    m_CurHp = m_TotalCombatStats.MaxHp;
    OnRep_CurHp();
}

void UStateComponent::UpdateDebugWidget()
{
    UDebugWidgetComponent* DebugComp = GetOwner()->FindComponentByClass<UDebugWidgetComponent>();
    if (!DebugComp) return;

    TArray<FString> StatLogs;
    StatLogs.Add(FString::Printf(TEXT("Lv.%d | %s"), m_CurrentLevel, *m_JobRowName.ToString()));
    StatLogs.Add(FString::Printf(TEXT("JOB PROF Lv.%d (%.0f EXP)"), GetCurrentJobProficiencyLevel(), GetCurrentJobProficiencyExp()));
    StatLogs.Add(FString::Printf(TEXT("STR:%.1f DEX:%.1f INT:%.1f"), m_CurrentAttributes.Strength, m_CurrentAttributes.Dexterity, m_CurrentAttributes.Intelligence));
    StatLogs.Add(FString::Printf(TEXT("EXP STR:%.0f DEX:%.0f INT:%.0f"), m_StrExp, m_DexExp, m_IntExp));
    StatLogs.Add(FString::Printf(TEXT("ATK:%.1f HP:%.1f/%.1f"), m_TotalCombatStats.Attack, m_CurHp, m_TotalCombatStats.MaxHp));

    DebugComp->UpdateLogList(StatLogs);
}

void UStateComponent::OnRep_CurHp()
{
    EventDispatcher_UpdateHp.Broadcast(m_CurHp, m_TotalCombatStats.MaxHp);
}

void UStateComponent::RecalculateTotalStats()
{
    m_TotalCombatStats = m_BaseCombatStats + m_EquipCombatStats;

    m_CurHp = FMath::Min(m_CurHp, m_TotalCombatStats.MaxHp);
    OnRep_CurHp();

    UpdateDebugWidget();

    if (GEngine)
    {
        uint64 Key = (uint64)GetOwner()->GetUniqueID() + 10;
        FString Msg = FString::Printf(TEXT("[%s] Total ATK: %.1f (Item: +%.1f)"),
            *GetOwner()->GetName(), m_TotalCombatStats.Attack, m_EquipCombatStats.Attack);
        GEngine->AddOnScreenDebugMessage(Key, 2.0f, FColor::Cyan, Msg);
    }
}

void UStateComponent::HandleDeath()
{
    if (AAIController* AICon = Cast<AAIController>(OwnerChar->GetController()))
    {
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsBool(TEXT("bIsDead"), true);
        }

        AICon->StopMovement();
    }

    if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement();
    }

    EventDispatcher_EventDeath.Broadcast();
    GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UStateComponent::DestroyDelay, 3.0f, false);
}

void UStateComponent::DestroyDelay()
{
    if (GetOwner()) GetOwner()->Destroy();
}

/** --- [저장 및 로드 시스템] --- */

FST_UnitSaveRecord UStateComponent::GetFullSaveData()
{
    FST_UnitSaveRecord SaveData;

    // 1. 기초 정보 및 성장 데이터 추출
    if (ARtsUnitCharacter* Owner = Cast<ARtsUnitCharacter>(GetOwner()))
    {
        SaveData.UnitDefinitionId = Owner->GetUnitDefinitionId(); //
    }

    SaveData.Level = m_CurrentLevel; //
    SaveData.CurrentExp = m_CurrentExp; //
    SaveData.CurrentJob = m_JobRowName; //
    SaveData.AccumulatedAttributes = m_AccumulatedAttributes; //
    SaveData.JobProgressMap = m_JobProgressMap;

    // 2. 장비 정보 추출 (EquipComponent 활용)
    if (UEquipComponent* EquipComp = GetOwner()->FindComponentByClass<UEquipComponent>())
    {
        SaveData.GetEquipSlot(EUnitEquipSlot::HandR) = EquipComp->GetEquipSlot(EUnitEquipSlot::HandR);
        SaveData.GetEquipSlot(EUnitEquipSlot::HandL) = EquipComp->GetEquipSlot(EUnitEquipSlot::HandL);
        SaveData.GetEquipSlot(EUnitEquipSlot::Head) = EquipComp->GetEquipSlot(EUnitEquipSlot::Head);
        SaveData.GetEquipSlot(EUnitEquipSlot::Body) = EquipComp->GetEquipSlot(EUnitEquipSlot::Body);
        SaveData.GetEquipSlot(EUnitEquipSlot::Horse) = EquipComp->GetEquipSlot(EUnitEquipSlot::Horse);

        // If any slot references an InstanceId, include the full instance entry from source-of-truth map
        const int32 MaxSlotCount = static_cast<int32>(EUnitEquipSlot::Max);
        for (int32 SlotIndex = 0; SlotIndex < MaxSlotCount; ++SlotIndex)
        {
            const EUnitEquipSlot SlotType = static_cast<EUnitEquipSlot>(SlotIndex);
            const FST_ItemSlot& Slot = SaveData.GetEquipSlot(SlotType);
            if (!Slot.InstanceId.IsValid())
            {
                continue;
            }

            FST_ItemInstance Inst;
            if (EquipComp->TryGetEquippedItemInstance(Slot.InstanceId, Inst))
            {
                SaveData.ItemInstances.Add(Inst.InstanceId, Inst);
            }
        }

        // Confirm saved armor slots for debugging
        if (GEngine && GetOwner())
        {
            FString HeadName = EquipComp->GetEquipSlotRowName(EUnitEquipSlot::Head).IsNone() ? TEXT("None") : EquipComp->GetEquipSlotRowName(EUnitEquipSlot::Head).ToString();
            FString BodyName = EquipComp->GetEquipSlotRowName(EUnitEquipSlot::Body).IsNone() ? TEXT("None") : EquipComp->GetEquipSlotRowName(EUnitEquipSlot::Body).ToString();
            UE_LOG(LogTemp, Log, TEXT("[%s] GetFullSaveData: HeadSlot='%s' BodySlot='%s'"), *GetOwner()->GetName(), *HeadName, *BodyName);
        }
    }

    return SaveData;
}

void UStateComponent::LoadFromFullSaveData(const FST_UnitSaveRecord& InData)
{
    // 1. 성장 데이터 복구
    m_CurrentLevel = InData.Level;
    m_CurrentExp = InData.CurrentExp;
    m_JobRowName = InData.CurrentJob.IsNone() ? FName(TEXT("Novice")) : InData.CurrentJob;
    m_AccumulatedAttributes = InData.AccumulatedAttributes;
    m_JobProgressMap = InData.JobProgressMap;
    EnsureCurrentJobProficiencyInitialized();

    // 2. 최종 스탯 재계산 및 UI 갱신
    RefreshFinalStats();
    m_CurHp = m_TotalCombatStats.MaxHp;
    OnRep_CurHp();

    // 3. 장착 정보 복구 (EquipSlots 배열 기반)
    if (UEquipComponent* EquipComp = GetOwner()->FindComponentByClass<UEquipComponent>())
    {
        EquipComp->ClearAllEquipSlots();

        const int32 MaxSlotCount = static_cast<int32>(EUnitEquipSlot::Max);
        const int32 LoopCount = FMath::Min(InData.EquipSlots.Num(), MaxSlotCount);

        for (int32 SlotIndex = 0; SlotIndex < LoopCount; ++SlotIndex)
        {
            const EUnitEquipSlot SlotType = static_cast<EUnitEquipSlot>(SlotIndex);
            const FST_ItemSlot& SavedSlot = InData.EquipSlots[SlotIndex];

            if (!SavedSlot.InstanceId.IsValid())
            {
                EquipComp->ClearEquipSlot(SlotType);
                continue;
            }

            FST_ItemInstance ItemInstance;
            if (const FST_ItemInstance* FoundInstance = InData.ItemInstances.Find(SavedSlot.InstanceId))
            {
                ItemInstance = *FoundInstance;
            }

            if (ItemInstance.ItemId.IsNone())
            {
                EquipComp->ClearEquipSlot(SlotType);
                continue;
            }
            if (!ItemInstance.InstanceId.IsValid())
            {
                ItemInstance.InstanceId = SavedSlot.InstanceId;
            }

            EquipComp->EquipItemInstance(ItemInstance, SlotType);
        }
    }
}