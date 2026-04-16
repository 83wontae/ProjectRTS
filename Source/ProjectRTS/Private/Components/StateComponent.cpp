// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/StateComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UStateComponent::UStateComponent() :m_CurHp(100.0), m_MaxHp(100.0)
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

	m_CurHp = m_MaxHp; // 초기 체력 설정
}

void UStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UStateComponent, m_CurHp);
}

void UStateComponent::AddDamage(double Damage)
{
    if (IsDeath() || GetOwnerRole() != ROLE_Authority) return; // 서버에서만 처리

    // 체력 차감 및 클램핑
    m_CurHp = FMath::Clamp(m_CurHp - Damage, 0.0, m_MaxHp);

    // 서버에서 즉시 알림 실행
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
    EventDispatcher_UpdateHp.Broadcast(m_CurHp, m_MaxHp);
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
    m_MaxHp = NewData.MaxHp;
    m_CurHp = m_MaxHp;
    OnRep_CurHp();
}

void UStateComponent::DestroyDelay()
{
    if (GetOwner())
    {
        GetOwner()->Destroy();
    }
}