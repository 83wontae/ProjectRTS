// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindEnemy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTRTS_API UBTTask_FindEnemy : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindEnemy();

	/** 태스크 실행 시 호출되는 핵심 함수 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** 블랙보드에서 타겟을 저장할 키 (에디터에서 선택 가능) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
