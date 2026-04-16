// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/StateComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UStateComponent::UStateComponent() :m_CurHp(100.0)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false; // Tick은 가급적 끕니다
	SetIsReplicatedByDefault(true); // 컴포넌트 복제 활성화
}


// Called when the game starts
void UStateComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<ACharacter>(GetOwner());
}

void UStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UStateComponent, m_CurHp);
}

void UStateComponent::SetBaseStats(const FST_Unit& UnitData)
{
    m_BaseStats.Attack = UnitData.Attack;
    m_BaseStats.MaxHp = UnitData.MaxHp;
    m_BaseStats.Defend = UnitData.Defend;
    m_BaseStats.Speed = UnitData.Speed;

    RecalculateTotalStats();
}

void UStateComponent::SetEquipStats(const FST_UnitStats& NewEquipStats)
{
    m_EquipStats = NewEquipStats;
    RecalculateTotalStats();
}

void UStateComponent::RecalculateTotalStats()
{
    // 1. 기초 스탯과 장비 스탯 합산
    m_TotalStats = m_BaseStats + m_EquipStats;

    // 2. 최대 체력이 변경됨에 따른 현재 체력 보정
    m_CurHp = FMath::Min(m_CurHp, m_TotalStats.MaxHp);

    // --- [화면 출력 로그 추가] ---
    if (GEngine)
    {
        FString ActorName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
        FColor DisplayColor = FColor::Cyan; // 출력 색상 설정
        float Duration = 5.0f;           // 화면 유지 시간 (5초)
        uint64 Key = (uint64)GetOwner()->GetUniqueID(); // 동일 액터의 로그는 덮어쓰도록 키 지정

        // 포맷팅된 문자열 생성
        FString DebugHeader = FString::Printf(TEXT("=== [%s] Stats Updated ==="), *ActorName);
        FString AttackLog = FString::Printf(TEXT("Attack : %.1f (Base:%.1f + Equip:%.1f)"),
            m_TotalStats.Attack, m_BaseStats.Attack, m_EquipStats.Attack);
        FString HpLog = FString::Printf(TEXT("MaxHP  : %.1f (Base:%.1f + Equip:%.1f)"),
            m_TotalStats.MaxHp, m_BaseStats.MaxHp, m_EquipStats.MaxHp);
        FString DefLog = FString::Printf(TEXT("Defend : %.1f (Base:%.1f + Equip:%.1f)"),
            m_TotalStats.Defend, m_BaseStats.Defend, m_EquipStats.Defend);

        // 화면에 출력 (키 값을 다르게 주면 여러 줄로 표시됩니다)
        GEngine->AddOnScreenDebugMessage(Key + 0, Duration, DisplayColor, DebugHeader);
        GEngine->AddOnScreenDebugMessage(Key + 1, Duration, FColor::White, AttackLog);
        GEngine->AddOnScreenDebugMessage(Key + 2, Duration, FColor::White, HpLog);
        GEngine->AddOnScreenDebugMessage(Key + 3, Duration, FColor::White, DefLog);
    }

    // 콘솔 로그
    UE_LOG(LogTemp, Warning, TEXT("[%s] Total Attack: %.2f"), *GetOwner()->GetName(), m_TotalStats.Attack);

    OnRep_CurHp();
}

void UStateComponent::AddDamage(double Damage)
{
    if (IsDeath() || GetOwnerRole() != ROLE_Authority) return;

    // 3. m_MaxHp 대신 m_TotalStats.MaxHp 사용
    m_CurHp = FMath::Clamp(m_CurHp - Damage, 0.0, m_TotalStats.MaxHp);

    OnRep_CurHp();

    if (IsDeath())
    {
        HandleDeath();
    }
}

bool UStateComponent::IsDeath() const
{
    return m_CurHp <= 0.0;
}

void UStateComponent::OnRep_CurHp()
{
    // HP 변경 이벤트 브로드캐스트
    EventDispatcher_UpdateHp.Broadcast(m_CurHp, m_TotalStats.MaxHp);
}

void UStateComponent::HandleDeath()
{
    // 사망 이벤트 호출
    EventDispatcher_EventDeath.Broadcast();

    // 일정 시간 후 파괴 예약 (예: 3초)
    GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UStateComponent::DestroyDelay, 3.0f, false);
}

void UStateComponent::UpdateUnitData(const FST_Unit& NewData)
{
    // 데이터 테이블의 구조체 값을 실제 컴포넌트 변수에 반영 
    m_CurHp = m_TotalStats.MaxHp;

	SetBaseStats(NewData); // BaseStats 업데이트 및 TotalStats 재계산

    OnRep_CurHp();
}

void UStateComponent::DestroyDelay()
{
    if (GetOwner())
    {
        GetOwner()->Destroy();
    }
}