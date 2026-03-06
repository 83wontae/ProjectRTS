// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SkillComponent.h"
#include "GameFramework/Character.h"
#include "Components/StateComponent.h"

// Sets default values for this component's properties
USkillComponent::USkillComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void USkillComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerChar = Cast<ACharacter>(GetOwner());
}

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 쿨타임 수치 감소 처리
    for (auto It = m_CooldownMap.CreateIterator(); It; ++It)
    {
        It.Value() -= DeltaTime;
        if (It.Value() <= 0.0f)
        {
            It.RemoveCurrent();
        }
        else
        {
            // UI 갱신을 위한 이벤트 호출
            OnCooldownUpdated.Broadcast(It.Key(), It.Value());
        }
    }
}

bool USkillComponent::UseSkill(FName SkillName)
{
    if (IsInCooldown(SkillName)) return false;

    const FST_Skill* SkillData = m_SkillMap.Find(SkillName);
    if (!SkillData) return false;

    // StateComponent와 연동하여 마나(자원) 확인 로직 추가 가능
    /*
    UStateComponent* StateComp = GetOwner()->FindComponentByClass<UStateComponent>();
    if (StateComp && StateComp->m_CurMp < SkillData->ManaCost) return false;
    */

    // 1. 애니메이션 재생
    if (OwnerChar && SkillData->SkillAnim)
    {
        OwnerChar->PlayAnimMontage(SkillData->SkillAnim);
    }

    // 2. 쿨타임 적용
    if (SkillData->CooldownTime > 0.0f)
    {
        m_CooldownMap.Add(SkillName, SkillData->CooldownTime);
    }

    // 3. 시전 알림
    OnSkillStarted.Broadcast(SkillName);

    return true;
}

bool USkillComponent::IsInCooldown(FName SkillName) const
{
    return m_CooldownMap.Contains(SkillName);
}

void USkillComponent::AddSkill(FName SkillName, const FST_Skill& SkillData)
{
    m_SkillMap.Add(SkillName, SkillData);
}

