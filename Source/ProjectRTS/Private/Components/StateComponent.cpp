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

	UDataTable* JobDataTable = RtsSettings::GetJobTable(GetOwner());
	if (!JobDataTable) {
		UE_LOG(LogTemp, Error, TEXT("[%s] StateComponent: JobDataTable is null! Please check RtsGameSettings."), *GetOwner()->GetName());
		return;
	}

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

    UDataTable* JobDataTable = RtsSettings::GetJobTable(GetOwner());
    if (!JobDataTable) return;

    m_CurrentExp += Amount;

    double RequiredExp = GetRequiredExpForLevel(m_CurrentLevel);

    while (m_CurrentExp >= RequiredExp)
    {
        m_CurrentExp -= RequiredExp;

        const FST_UnitJob* JobData = JobDataTable ? JobDataTable->FindRow<FST_UnitJob>(m_JobRowName, TEXT("")) : nullptr;
        if (JobData)
        {
            m_AccumulatedAttributes = m_AccumulatedAttributes + JobData->GrowthAttributes;
        }

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

void UStateComponent::RefreshFinalStats()
{
    m_CurrentAttributes = m_OriginAttributes + m_AccumulatedAttributes;

    UpdateCombatStats();
}

void UStateComponent::UpdateCombatStats()
{
    FST_CombatStats NewBase;
    NewBase.Attack = (m_CurrentAttributes.Strength * 1.0) + (m_CurrentAttributes.Agility * 0.2);
    NewBase.MaxHp = (m_CurrentAttributes.Stamina * 10.0) + (m_CurrentAttributes.Strength * 2.0);
    NewBase.Defend = (m_CurrentAttributes.Agility * 1.2);
    NewBase.Speed = 500.0 + (m_CurrentAttributes.Agility * 0.1);

    m_BaseCombatStats = NewBase;

    RecalculateTotalStats();
}

void UStateComponent::ChangeJob(FName NewJobRowName)
{
    m_JobRowName = NewJobRowName;
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

void UStateComponent::UpdateDebugWidget()
{
    UDebugWidgetComponent* DebugComp = GetOwner()->FindComponentByClass<UDebugWidgetComponent>();
    if (!DebugComp) return;

    TArray<FString> StatLogs;
    StatLogs.Add(FString::Printf(TEXT("Lv.%d | %s"), m_CurrentLevel, *m_JobRowName.ToString()));
    StatLogs.Add(FString::Printf(TEXT("STR:%.1f AGI:%.1f INT:%.1f"), m_CurrentAttributes.Strength, m_CurrentAttributes.Agility, m_CurrentAttributes.Intelligence));
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

FST_UnitSaveData UStateComponent::GetFullSaveData()
{
    FST_UnitSaveData SaveData;

    // 1. 기초 정보 및 성장 데이터 추출
    if (ARtsUnitCharacter* Owner = Cast<ARtsUnitCharacter>(GetOwner()))
    {
        SaveData.UnitRowName = Owner->GetUnitRowName(); //
    }

    SaveData.Level = m_CurrentLevel; //
    SaveData.CurrentExp = m_CurrentExp; //
    SaveData.CurrentJob = m_JobRowName; //
    SaveData.AccumulatedAttributes = m_AccumulatedAttributes; //

    // 2. 장비 정보 추출 (EquipComponent 활용)
    if (UEquipComponent* EquipComp = GetOwner()->FindComponentByClass<UEquipComponent>())
    {
        SaveData.HandR = EquipComp->m_RightWeaponName; //
        SaveData.HandL = EquipComp->m_LeftWeaponName; //
    }

    return SaveData;
}

void UStateComponent::LoadFromFullSaveData(const FST_UnitSaveData& InData)
{
    // 1. 성장 데이터 복구
    m_CurrentLevel = InData.Level;
    m_CurrentExp = InData.CurrentExp;
    m_JobRowName = InData.CurrentJob;
    m_AccumulatedAttributes = InData.AccumulatedAttributes;

    // 2. 최종 스탯 재계산 및 UI 갱신
    RefreshFinalStats();
    m_CurHp = m_TotalCombatStats.MaxHp;
    OnRep_CurHp();

    // 3. 장비 복구 (EquipComponent에 명령)
    if (UEquipComponent* EquipComp = GetOwner()->FindComponentByClass<UEquipComponent>())
    {
        // 무기 이름만 넣어주면 EquipToWeapon이 제약사항을 확인하여 장착합니다.
        if (!InData.HandR.IsNone()) EquipComp->EquipToWeapon(InData.HandR);
        if (!InData.HandL.IsNone()) EquipComp->EquipToWeapon(InData.HandL);
    }
}