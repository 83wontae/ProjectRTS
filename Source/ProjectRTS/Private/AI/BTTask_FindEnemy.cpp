// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_FindEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkillComponent.h"
#include "Components/StateComponent.h"
#include "Interface/UnitInterface.h"

UBTTask_FindEnemy::UBTTask_FindEnemy()
{
	NodeName = TEXT("Find Enemy In Attack Area");
}

EBTNodeResult::Type UBTTask_FindEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	// --- [1. 유닛 본인 사망 체크 추가] ---
	// 본인이 이미 죽었다면 더 이상 적을 찾을 필요가 없습니다.
	if (Pawn->Implements<UUnitInterface>())
	{
		if (IUnitInterface::Execute_IsDeath(Pawn))
		{
			return EBTNodeResult::Failed;
		}
	}

	// 1. SkillComponent 가져오기
	USkillComponent* SkillComp = Pawn->FindComponentByClass<USkillComponent>();
	if (!SkillComp) return EBTNodeResult::Failed;

	// 2. SkillComponent에 구현된 탐색 로직 실행
	AActor* FoundTarget = SkillComp->FindBestTargetInRange();

	// --- [2. 찾은 타겟의 생존 여부 체크 추가] ---
	// 찾은 대상이 유효하고, 인터페이스를 가지고 있다면 죽었는지 확인합니다.
	if (FoundTarget)
	{
		if (FoundTarget->Implements<UUnitInterface>())
		{
			if (IUnitInterface::Execute_IsDeath(FoundTarget))
			{
				return EBTNodeResult::Failed; // 이미 죽은 적은 타겟팅하지 않음
			}
		}

		// 3. 블랙보드 키에 타겟 저장
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		if (BB == nullptr)
			return EBTNodeResult::Failed;

		UStateComponent* StateComp = Pawn->FindComponentByClass<UStateComponent>();
		if (StateComp == nullptr)
			return EBTNodeResult::Failed;

		BB->SetValueAsObject(TargetActorKey.SelectedKeyName, FoundTarget);
		StateComp->SetAggroTarget(FoundTarget); //
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}