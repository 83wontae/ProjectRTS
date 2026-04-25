#include "Components/StateComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/DebugWidgetComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/RtsUnitCharacter.h"
#include "Components/EquipComponent.h" // Àåºñ Á¤º¸ Á¢±ÙÀ» À§ÇØ Ãß°¡

UStateComponent::UStateComponent() : m_CurHp(100.0)
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UStateComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerChar = Cast<ACharacter>(GetOwner());

    // --- [ì¶”ê?] ?œì‘ ??ì´ˆê¸° ì§ì—… ?¤ì • ë°??¤íƒ¯ ê³„ì‚° ---
    // ê¸°ë³¸ ì§ì—…???¤ì •?˜ì–´ ?ˆì? ?Šë‹¤ë©?'Novice' ?±ìœ¼ë¡?ì´ˆê¸°??
    if (m_JobRowName.IsNone())
    {
        m_JobRowName = TEXT("Novice");
    }

    // ì´ˆê¸° (1,1,1,1) ?ì„± ë°??„íˆ¬ ?¥ë ¥ì¹?ê³„ì‚° ?˜í–‰
    RefreshFinalStats();

    // ?œì‘ ??ì²´ë ¥??ìµœë? ì²´ë ¥?¼ë¡œ ?¤ì •
    m_CurHp = m_TotalCombatStats.MaxHp;
    OnRep_CurHp();
}

void UStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // ê¸°ì¡´ ?™ê¸°????ª© ? ì?
    DOREPLIFETIME(UStateComponent, m_CurHp);
    DOREPLIFETIME(UStateComponent, m_AggroTarget);
}

double UStateComponent::GetRequiredExpForLevel(int32 Level) const
{
    // [ê³µì‹] ?”êµ¬ ê²½í—˜ì¹?= 100 * (?„ì¬?ˆë²¨^2)
    // ì§€??2)ë¥??’ì´ë©??„ë°˜ ?ˆë²¨?…ì´ ???˜ë“¤?´ì§‘?ˆë‹¤.
   return 5.0 * FMath::Pow((float)Level, 2.0f);
}

double UStateComponent::CalculateExpReward() const
{
    // [ê³µì‹] ë³´ìƒ ê²½í—˜ì¹?= ê¸°ì´ˆê°?* (1.0 + (?ˆë²¨ * 0.1))
    // ì¦? ?ˆë²¨???’ì? ? ë‹›???¡ì„?˜ë¡ ??ë§ì? ê²½í—˜ì¹˜ë? ?»ìŠµ?ˆë‹¤.
    return m_BaseExpReward * (1.0 + (m_CurrentLevel * 0.1));
}

void UStateComponent::AddExp(double Amount)
{
    if (Amount <= 0.0) return;
    m_CurrentExp += Amount;

    double RequiredExp = GetRequiredExpForLevel(m_CurrentLevel);

    // 1. ?ˆë²¨??ë£¨í”„ ?œì‘
    while (m_CurrentExp >= RequiredExp)
    {
        m_CurrentExp -= RequiredExp;

        // ?„ì¬ ì§ì—…???±ì¥ì¹˜ë? ?„ì 
        const FST_UnitJob* JobData = JobDataTable ? JobDataTable->FindRow<FST_UnitJob>(m_JobRowName, TEXT("")) : nullptr;
        if (JobData)
        {
            m_AccumulatedAttributes = m_AccumulatedAttributes + JobData->GrowthAttributes;
        }

        m_CurrentLevel++;

        // --- [?¸ë¦¬ê²Œì´???Œë¦¼: ?ˆë²¨?? ---
        // ë¸”ë£¨?„ë¦°?¸ì—???ˆë²¨???´í™?¸ë‚˜ ?¬ìš´?œë? ?¬ìƒ?????ˆìŠµ?ˆë‹¤.
        if (EventDispatcher_LevelUp.IsBound())
        {
            EventDispatcher_LevelUp.Broadcast(m_CurrentLevel);
        }

        RefreshFinalStats();

        // ?¤ìŒ ?ˆë²¨ ?”êµ¬ì¹?ê°±ì‹ 
        RequiredExp = GetRequiredExpForLevel(m_CurrentLevel);
    }

    // --- [?¸ë¦¬ê²Œì´???Œë¦¼: ê²½í—˜ì¹?ë³€ê²? ---
    // UI??ê²½í—˜ì¹?ë°?Progress Bar)ë¥??…ë°?´íŠ¸?˜ëŠ” ???¬ìš©?©ë‹ˆ??
    if (EventDispatcher_ExpChanged.IsBound())
    {
        EventDispatcher_ExpChanged.Broadcast(m_CurrentExp, RequiredExp);
    }

    UpdateDebugWidget();
}

void UStateComponent::RefreshFinalStats()
{
    // ìµœì¢… ?ì„± = ê¸°ì´ˆ(1,1,1,1) + ?ˆë²¨???„ì  ?±ì¥ì¹?
    m_CurrentAttributes = m_OriginAttributes + m_AccumulatedAttributes;

    UpdateCombatStats();
}

void UStateComponent::UpdateCombatStats()
{
    // [ê³µì‹] ?„ì¬???„ì  ?ì„±(m_CurrentAttributes)??ê¸°ë°˜?¼ë¡œ ë³¸ì²´ ?„íˆ¬??ê³„ì‚°
    FST_CombatStats NewBase;
    NewBase.Attack = (m_CurrentAttributes.Strength * 1.0) + (m_CurrentAttributes.Agility * 0.2);
    NewBase.MaxHp = (m_CurrentAttributes.Stamina * 10.0) + (m_CurrentAttributes.Strength * 2.0);
    NewBase.Defend = (m_CurrentAttributes.Agility * 1.2);
    NewBase.Speed = 500.0 + (m_CurrentAttributes.Agility * 0.1);

    m_BaseCombatStats = NewBase;

    // ë³¸ì²´ ?„íˆ¬?¥ì´ ë°”ë€Œì—ˆ?¼ë‹ˆ ìµœì¢… ?¤íƒ¯???¤ì‹œ ê³„ì‚°
    RecalculateTotalStats();
}

void UStateComponent::ChangeJob(FName NewJobRowName)
{
    // ?„ì§?´ë„ ì§€ê¸ˆê¹Œì§€ ?“ì¸ m_AccumulatedAttributes??? ì???
    m_JobRowName = NewJobRowName;
}

void UStateComponent::AddDamage(AController* EventInstigator, double Damage)
{
    if (IsDeath() || GetOwnerRole() != ROLE_Authority) return;

    m_CurHp = FMath::Clamp(m_CurHp - Damage, 0.0, m_TotalCombatStats.MaxHp);
    OnRep_CurHp();

    if (IsDeath())
    {
        // 1. ê°€?´ì ?ë³„ ë°?ê²½í—˜ì¹?ì§€ê¸?
        if (EventInstigator && EventInstigator->GetPawn())
        {
            UStateComponent* KillerState = EventInstigator->GetPawn()->FindComponentByClass<UStateComponent>();
            if (KillerState)
            {
                // ?¨ìˆ˜ë¡?ê³„ì‚°??ê²½í—˜ì¹?ë³´ìƒ??ê°€?´ì?ê²Œ ?„ë‹¬
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
    // 1. ?ˆë¡œ???¥ë¹„ ?˜ì¹˜ ?€??
    m_EquipCombatStats = NewEquipStats;

    // 2. ìµœì¢… ?¤íƒ¯ ?¬ê³„???¸ì¶œ
    RecalculateTotalStats();
}

void UStateComponent::UpdateDebugWidget()
{
    // 1. [ê°€?? ë¨¸ë¦¬ ?„ì— ?¬ë¦° ?”ë²„ê·??„ì ¯ ì»´í¬?ŒíŠ¸ ì°¾ê¸°
    UDebugWidgetComponent* DebugComp = GetOwner()->FindComponentByClass<UDebugWidgetComponent>();
    if (!DebugComp) return;

    // 2. ì¶œë ¥??ë¬¸ì???ì„± (?ˆë²¨, ì§ì—…, ?„ì¬ ?ì„± ??
    TArray<FString> StatLogs;
    StatLogs.Add(FString::Printf(TEXT("Lv.%d | %s"), m_CurrentLevel, *m_JobRowName.ToString()));
    StatLogs.Add(FString::Printf(TEXT("STR:%.1f AGI:%.1f INT:%.1f"), m_CurrentAttributes.Strength, m_CurrentAttributes.Agility, m_CurrentAttributes.Intelligence));
    StatLogs.Add(FString::Printf(TEXT("ATK:%.1f HP:%.1f/%.1f"), m_TotalCombatStats.Attack, m_CurHp, m_TotalCombatStats.MaxHp));

    // 3. ?„ì ¯ ì»´í¬?ŒíŠ¸???„ë‹¬?˜ì—¬ ?”ë©´ ê°±ì‹ 
    DebugComp->UpdateLogList(StatLogs);
}

void UStateComponent::OnRep_CurHp()
{
    EventDispatcher_UpdateHp.Broadcast(m_CurHp, m_TotalCombatStats.MaxHp);
}

void UStateComponent::RecalculateTotalStats()
{
    // [?©ì‚°] ë³¸ì²´(Attributes ê¸°ë°˜) + ?¥ë¹„(Item ê¸°ë°˜)
    // ProjectRTSTypes.h???•ì˜??operator+ ê°€ ?¬ê¸°???¬ìš©?©ë‹ˆ??
    m_TotalCombatStats = m_BaseCombatStats + m_EquipCombatStats;

    // ìµœë? ì²´ë ¥ ë³€ê²½ì— ?°ë¥¸ ?„ì¬ HP ë³´ì • ë°??™ê¸°??
    m_CurHp = FMath::Min(m_CurHp, m_TotalCombatStats.MaxHp);
    OnRep_CurHp();

    // ?¥ë¹„ ?¥ì°©/?´ì œ ???„ì ¯ ë°?ë¡œê·¸ ê°±ì‹ 
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
    // 1. ë¸”ë™ë³´ë“œ ê°?ê°±ì‹  (AI?ê²Œ ì£½ì—ˆ?Œì„ ?Œë¦¼)
    if (AAIController* AICon = Cast<AAIController>(OwnerChar->GetController()))
    {
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsBool(TEXT("bIsDead"), true);
        }

        // ?„ì¬ ê²½ë¡œ ?ìƒ‰ ì¤‘ì?
        AICon->StopMovement();
    }

    // 2. ?´ë™ ì»´í¬?ŒíŠ¸ ë¹„í™œ?±í™” (ë¬¼ë¦¬???•ì?)
    if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement(); // MOVE_None ?íƒœë¡?ë³€ê²?
    }

    EventDispatcher_EventDeath.Broadcast();
    GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UStateComponent::DestroyDelay, 3.0f, false);
}

void UStateComponent::DestroyDelay()
{
    if (GetOwner()) GetOwner()->Destroy();
}

/** --- [ÀúÀå ¹× ·Îµå ½Ã½ºÅÛ] --- */

FST_UnitSaveData UStateComponent::GetFullSaveData()
{
    FST_UnitSaveData SaveData;

    // 1. ±âÃÊ Á¤º¸ ¹× ¼ºÀå µ¥ÀÌÅÍ ÃßÃâ
    if (ARtsUnitCharacter* Owner = Cast<ARtsUnitCharacter>(GetOwner()))
    {
        SaveData.UnitRowName = Owner->GetUnitRowName(); //
    }

    SaveData.Level = m_CurrentLevel; //
    SaveData.CurrentExp = m_CurrentExp; //
    SaveData.CurrentJob = m_JobRowName; //
    SaveData.AccumulatedAttributes = m_AccumulatedAttributes; //

    // 2. Àåºñ Á¤º¸ ÃßÃâ (EquipComponent È°¿ë)
    if (UEquipComponent* EquipComp = GetOwner()->FindComponentByClass<UEquipComponent>())
    {
        SaveData.HandR = EquipComp->m_RightWeaponName; //
        SaveData.HandL = EquipComp->m_LeftWeaponName; //
    }

    return SaveData;
}

void UStateComponent::LoadFromFullSaveData(const FST_UnitSaveData& InData)
{
    // 1. ¼ºÀå µ¥ÀÌÅÍ º¹±¸
    m_CurrentLevel = InData.Level;
    m_CurrentExp = InData.CurrentExp;
    m_JobRowName = InData.CurrentJob;
    m_AccumulatedAttributes = InData.AccumulatedAttributes;

    // 2. ÃÖÁ¾ ½ºÅÈ Àç°è»ê ¹× UI °»½Å
    RefreshFinalStats();
    m_CurHp = m_TotalCombatStats.MaxHp;
    OnRep_CurHp();

    // 3. Àåºñ º¹±¸ (EquipComponent¿¡ ¸í·É)
    if (UEquipComponent* EquipComp = GetOwner()->FindComponentByClass<UEquipComponent>())
    {
        // ¹«±â ÀÌ¸§¸¸ ³Ö¾îÁÖ¸é EquipToWeaponÀÌ Á¦¾à»çÇ×À» È®ÀÎÇÏ¿© ÀåÂøÇÕ´Ï´Ù.
        if (!InData.HandR.IsNone()) EquipComp->EquipToWeapon(InData.HandR);
        if (!InData.HandL.IsNone()) EquipComp->EquipToWeapon(InData.HandL);
    }
}