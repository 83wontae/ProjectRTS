// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkillComponent.h"
#include "Interface/UnitInterface.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Execute Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn()) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIC->GetPawn();

	// --- [1. 본인 사망 체크] ---
	if (ControlledPawn->Implements<UUnitInterface>() && IUnitInterface::Execute_IsDeath(ControlledPawn))
	{
		return EBTNodeResult::Failed;
	}

	USkillComponent* SkillComp = ControlledPawn->FindComponentByClass<USkillComponent>();
	if (!SkillComp) return EBTNodeResult::Failed;

	// 블랙보드에서 타겟 가져오기
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));

	// --- [2. 타겟 유효성 및 사망 체크] ---
	// 타겟이 없거나 이미 죽었다면 공격을 시작하지 않습니다.
	if (!Target || (Target->Implements<UUnitInterface>() && IUnitInterface::Execute_IsDeath(Target)))
	{
		return EBTNodeResult::Failed;
	}

	bool bSuccess = SkillComp->ExecuteBestAttack(Target);
	if (bSuccess)
	{
		return EBTNodeResult::InProgress; // 지연 태스크로 전환하여 Tick에서 감시
	}

	return EBTNodeResult::Failed;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* ControlledPawn = AIC->GetPawn();

	// --- [3. 공격 도중 본인 사망 체크] ---
	if (ControlledPawn->Implements<UUnitInterface>() && IUnitInterface::Execute_IsDeath(ControlledPawn))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// --- [4. 공격 도중 타겟 사망 체크] ---
	// 휘두르는 도중에 타겟이 먼저 죽었다면 태스크를 중단하거나 성공으로 마무리합니다.
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target || (Target->Implements<UUnitInterface>() && IUnitInterface::Execute_IsDeath(Target)))
	{
		// 타겟이 죽었으므로 공격을 더 지속할 이유가 없음
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	USkillComponent* SkillComp = ControlledPawn->FindComponentByClass<USkillComponent>();
	if (SkillComp && !SkillComp->bIsAttacking)
	{
		// 정상적으로 공격 애니메이션이 끝난 경우
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}