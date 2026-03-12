// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkillComponent.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Execute Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn())
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_Attack] Failed: Controller or Pawn is null."));
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIC->GetPawn();
	USkillComponent* SkillComp = ControlledPawn->FindComponentByClass<USkillComponent>();
	if (!SkillComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] BTTask_Attack: SkillComponent not found!"), *ControlledPawn->GetName());
		return EBTNodeResult::Failed;
	}

	// 1. 블랙보드에서 타겟 가져오기
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));
	FString TargetName = Target ? Target->GetName() : TEXT("NULL");

	UE_LOG(LogTemp, Log, TEXT("[%s] BTTask_Attack: Attempting attack on Target [%s]"), *ControlledPawn->GetName(), *TargetName);

	// 2. 공격 실행
	bool bSuccess = SkillComp->UseSkill(NAME_None, Target);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] BTTask_Attack: UseSkill Successful -> InProgress"), *ControlledPawn->GetName());
		return EBTNodeResult::InProgress;
	}
	else
	{
		// 실패 원인 분석 로그 (이미 공격 중인지, 쿨타임인지 등)
		UE_LOG(LogTemp, Error, TEXT("[%s] BTTask_Attack: UseSkill Failed! (Check: bIsAttacking=%s, SkillMap empty?)"),
			*ControlledPawn->GetName(), SkillComp->bIsAttacking ? TEXT("True") : TEXT("False"));
		return EBTNodeResult::Failed;
	}
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn())
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_Attack] Tick Failed: Pawn Lost."));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	USkillComponent* SkillComp = AIC->GetPawn()->FindComponentByClass<USkillComponent>();

	if (SkillComp)
	{
		// [로그 추가] 매 프레임 상태를 확인하고 있는지 (너무 자주 찍힐 수 있으므로 주의)
		// UE_LOG(LogTemp, Verbose, TEXT("[%s] TickTask: Waiting for bIsAttacking to be false..."), *AIC->GetPawn()->GetName());

		if (!SkillComp->bIsAttacking)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] BTTask_Attack: Skill finished. Finishing Latent Task with Success."), *AIC->GetPawn()->GetName());

			// 태스크 성공 종료 알림
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}