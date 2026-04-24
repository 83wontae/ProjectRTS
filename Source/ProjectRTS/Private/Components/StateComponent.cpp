#include "Components/StateComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/DebugWidgetComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/RtsUnitCharacter.h"

UStateComponent::UStateComponent() : m_CurHp(100.0)
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UStateComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerChar = Cast<ACharacter>(GetOwner());

    // --- [추가] 시작 시 초기 직업 설정 및 스탯 계산 ---
    // 기본 직업이 설정되어 있지 않다면 'Novice' 등으로 초기화
    if (m_JobRowName.IsNone())
    {
        m_JobRowName = TEXT("Novice");
    }

    // 초기 (1,1,1,1) 속성 및 전투 능력치 계산 수행
    RefreshFinalStats();

    // 시작 시 체력을 최대 체력으로 설정
    m_CurHp = m_TotalCombatStats.MaxHp;
    OnRep_CurHp();
}

void UStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // 기존 동기화 항목 유지
    DOREPLIFETIME(UStateComponent, m_CurHp);
    DOREPLIFETIME(UStateComponent, m_AggroTarget);
}

double UStateComponent::GetRequiredExpForLevel(int32 Level) const
{
    // [공식] 요구 경험치 = 100 * (현재레벨^2)
    // 지수(2)를 높이면 후반 레벨업이 더 힘들어집니다.
   return 5.0 * FMath::Pow((float)Level, 2.0f);
}

double UStateComponent::CalculateExpReward() const
{
    // [공식] 보상 경험치 = 기초값 * (1.0 + (레벨 * 0.1))
    // 즉, 레벨이 높은 유닛을 잡을수록 더 많은 경험치를 얻습니다.
    return m_BaseExpReward * (1.0 + (m_CurrentLevel * 0.1));
}

void UStateComponent::AddExp(double Amount)
{
    if (Amount <= 0.0) return;
    m_CurrentExp += Amount;

    double RequiredExp = GetRequiredExpForLevel(m_CurrentLevel);

    // 1. 레벨업 루프 시작
    while (m_CurrentExp >= RequiredExp)
    {
        m_CurrentExp -= RequiredExp;

        // 현재 직업의 성장치를 누적
        const FST_UnitJob* JobData = JobDataTable ? JobDataTable->FindRow<FST_UnitJob>(m_JobRowName, TEXT("")) : nullptr;
        if (JobData)
        {
            m_AccumulatedAttributes = m_AccumulatedAttributes + JobData->GrowthAttributes;
        }

        m_CurrentLevel++;

        // --- [델리게이트 알림: 레벨업] ---
        // 블루프린트에서 레벨업 이펙트나 사운드를 재생할 수 있습니다.
        if (EventDispatcher_LevelUp.IsBound())
        {
            EventDispatcher_LevelUp.Broadcast(m_CurrentLevel);
        }

        RefreshFinalStats();

        // 다음 레벨 요구치 갱신
        RequiredExp = GetRequiredExpForLevel(m_CurrentLevel);
    }

    // --- [델리게이트 알림: 경험치 변경] ---
    // UI의 경험치 바(Progress Bar)를 업데이트하는 데 사용됩니다.
    if (EventDispatcher_ExpChanged.IsBound())
    {
        EventDispatcher_ExpChanged.Broadcast(m_CurrentExp, RequiredExp);
    }

    UpdateDebugWidget();
}

void UStateComponent::RefreshFinalStats()
{
    // 최종 속성 = 기초(1,1,1,1) + 레벨업 누적 성장치
    m_CurrentAttributes = m_OriginAttributes + m_AccumulatedAttributes;

    UpdateCombatStats();
}

void UStateComponent::UpdateCombatStats()
{
    // [공식] 현재의 누적 속성(m_CurrentAttributes)을 기반으로 본체 전투력 계산
    FST_CombatStats NewBase;
    NewBase.Attack = (m_CurrentAttributes.Strength * 1.0) + (m_CurrentAttributes.Agility * 0.2);
    NewBase.MaxHp = (m_CurrentAttributes.Stamina * 10.0) + (m_CurrentAttributes.Strength * 2.0);
    NewBase.Defend = (m_CurrentAttributes.Agility * 1.2);
    NewBase.Speed = 500.0 + (m_CurrentAttributes.Agility * 0.1);

    m_BaseCombatStats = NewBase;

    // 본체 전투력이 바뀌었으니 최종 스탯도 다시 계산
    RecalculateTotalStats();
}

void UStateComponent::ChangeJob(FName NewJobRowName)
{
    // 전직해도 지금까지 쌓인 m_AccumulatedAttributes는 유지됨
    m_JobRowName = NewJobRowName;
}

void UStateComponent::AddDamage(AController* EventInstigator, double Damage)
{
    if (IsDeath() || GetOwnerRole() != ROLE_Authority) return;

    m_CurHp = FMath::Clamp(m_CurHp - Damage, 0.0, m_TotalCombatStats.MaxHp);
    OnRep_CurHp();

    if (IsDeath())
    {
        // 1. 가해자 식별 및 경험치 지급
        if (EventInstigator && EventInstigator->GetPawn())
        {
            UStateComponent* KillerState = EventInstigator->GetPawn()->FindComponentByClass<UStateComponent>();
            if (KillerState)
            {
                // 함수로 계산된 경험치 보상을 가해자에게 전달
                double FinalXP = CalculateExpReward();
                KillerState->AddExp(FinalXP);
            }
        }

        HandleDeath(); //
    }
}

bool UStateComponent::IsDeath() const
{
    return m_CurHp <= 0.0;
}

void UStateComponent::SetEquipCombatStats(const FST_CombatStats& NewEquipStats)
{
    // 1. 새로운 장비 수치 저장
    m_EquipCombatStats = NewEquipStats;

    // 2. 최종 스탯 재계산 호출
    RecalculateTotalStats();
}

void UStateComponent::UpdateDebugWidget()
{
    // 1. [가드] 머리 위에 달린 디버그 위젯 컴포넌트 찾기
    UDebugWidgetComponent* DebugComp = GetOwner()->FindComponentByClass<UDebugWidgetComponent>();
    if (!DebugComp) return;

    // 2. 출력할 문자열 생성 (레벨, 직업, 현재 속성 등)
    TArray<FString> StatLogs;
    StatLogs.Add(FString::Printf(TEXT("Lv.%d | %s"), m_CurrentLevel, *m_JobRowName.ToString()));
    StatLogs.Add(FString::Printf(TEXT("STR:%.1f AGI:%.1f INT:%.1f"), m_CurrentAttributes.Strength, m_CurrentAttributes.Agility, m_CurrentAttributes.Intelligence));
    StatLogs.Add(FString::Printf(TEXT("ATK:%.1f HP:%.1f/%.1f"), m_TotalCombatStats.Attack, m_CurHp, m_TotalCombatStats.MaxHp));

    // 3. 위젯 컴포넌트에 전달하여 화면 갱신
    DebugComp->UpdateLogList(StatLogs);
}

void UStateComponent::OnRep_CurHp()
{
    EventDispatcher_UpdateHp.Broadcast(m_CurHp, m_TotalCombatStats.MaxHp);
}

void UStateComponent::RecalculateTotalStats()
{
    // [합산] 본체(Attributes 기반) + 장비(Item 기반)
    // ProjectRTSTypes.h에 정의한 operator+ 가 여기서 사용됩니다.
    m_TotalCombatStats = m_BaseCombatStats + m_EquipCombatStats;

    // 최대 체력 변경에 따른 현재 HP 보정 및 동기화
    m_CurHp = FMath::Min(m_CurHp, m_TotalCombatStats.MaxHp);
    OnRep_CurHp();

    // 장비 장착/해제 시 위젯 및 로그 갱신
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
    // 1. 블랙보드 값 갱신 (AI에게 죽었음을 알림)
    if (AAIController* AICon = Cast<AAIController>(OwnerChar->GetController()))
    {
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsBool(TEXT("bIsDead"), true);
        }

        // 현재 경로 탐색 중지
        AICon->StopMovement();
    }

    // 2. 이동 컴포넌트 비활성화 (물리적 정지)
    if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement(); // MOVE_None 상태로 변경
    }

    EventDispatcher_EventDeath.Broadcast();
    GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UStateComponent::DestroyDelay, 3.0f, false);
}

void UStateComponent::DestroyDelay()
{
    if (GetOwner()) GetOwner()->Destroy();
}

/** --- [저장 및 로드 시스템] --- */

FST_UnitSaveData UStateComponent::GetSaveData()
{
    FST_UnitSaveData SaveData;

    // 1. 유닛 종류 식별 (Owner 액터에서 RowName 추출)
    // UnitRowName이 protected라면 RtsUnitCharacter에 Getter를 추가하거나, 
    // 아래처럼 직접 접근이 가능한 구조여야 합니다.
    if (ARtsUnitCharacter* RtsChar = Cast<ARtsUnitCharacter>(GetOwner()))
    {
        // RtsChar 내부의 UnitRowName을 저장합니다.
        // (참고: RtsUnitCharacter.h에 UnitRowName Getter가 있다고 가정)
        // SaveData.UnitRowName = RtsChar->GetUnitRowName(); 
    }

    // 2. 성장 데이터 복사
    SaveData.Level = m_CurrentLevel;
    SaveData.CurrentExp = m_CurrentExp;
    SaveData.CurrentJob = m_JobRowName;

    // 3. 무한 성장의 핵심인 누적 속성치 저장
    SaveData.AccumulatedAttributes = m_AccumulatedAttributes;

    return SaveData;
}

void UStateComponent::LoadFromSaveData(const FST_UnitSaveData& InData)
{
    // 1. 데이터 주입
    m_CurrentLevel = InData.Level;
    m_CurrentExp = InData.CurrentExp;
    m_JobRowName = InData.CurrentJob;
    m_AccumulatedAttributes = InData.AccumulatedAttributes;

    // 2. 주입된 속성을 바탕으로 최종 스탯 및 전투 능력치 재계산
    // 이 함수를 호출해야 m_BaseCombatStats와 m_TotalCombatStats가 갱신됩니다.
    RefreshFinalStats();

    // 3. 체력 복구 (최대 체력으로 설정하거나 저장된 체력치가 있다면 그것으로 설정)
    m_CurHp = m_TotalCombatStats.MaxHp;
    OnRep_CurHp();

    // 4. UI 및 디버그 정보 갱신
    UpdateDebugWidget();
}