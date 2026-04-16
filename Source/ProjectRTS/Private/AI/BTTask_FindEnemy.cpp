// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_FindEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkillComponent.h"
#include "Components/StateComponent.h"

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

	// 1. SkillComponent 가져오기
	USkillComponent* SkillComp = Pawn->FindComponentByClass<USkillComponent>();
	if (!SkillComp) return EBTNodeResult::Failed;

	// 2. SkillComponent에 구현된 탐색 로직 실행
	AActor* FoundTarget = SkillComp->FindBestTargetInRange();

	if (FoundTarget)
	{
		// 3. 블랙보드 키에 타겟 저장
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		if (BB == nullptr)
			return EBTNodeResult::Failed;

		UStateComponent* StateComp = Pawn->FindComponentByClass<UStateComponent>();
		if (StateComp == nullptr)
			return EBTNodeResult::Failed;

		BB->SetValueAsObject(TargetActorKey.SelectedKeyName, FoundTarget);
		StateComp->SetAggroTarget(FoundTarget);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}