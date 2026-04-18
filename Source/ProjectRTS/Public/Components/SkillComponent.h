// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/ProjectRTSTypes.h"
#include "SkillComponent.generated.h"

// 스킬 관련 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillStarted, FName, SkillName);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 1. [기능] 주변의 가장 가까운 적을 찾아 반환합니다. (BTTask에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	AActor* FindBestTargetInRange();

	/** 2. [기능] 특정 대상이 현재 무기 사거리 안에 있는지 확인합니다. (BTDecorator에서 호출) */
	UFUNCTION(BlueprintPure, Category = "RTS|Combat")
	bool IsCanAttack(AActor* InTarget);

	/** 3. [액션] 스킬(공격)을 실행합니다. (BTTask에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	bool UseSkill(FName SkillName, AActor* InTarget);

	/** BTTask 등에서 호출하는 범용 공격 함수 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	bool ExecuteBestAttack(AActor* InTarget);

	/** 현재 공격(몽타주 재생) 중인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "RTS|Combat")
	bool bIsAttacking;

	UFUNCTION(BlueprintPure, Category = "RTS|Combat")
	FName GetDefaultAttackSkillName();

	/** 1. 단일 스킬을 맵에 직접 추가합니다. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Skill")
	void AddSkill(FName SkillName);

	/** 2. 현재 등록된 모든 스킬을 제거합니다. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Skill")
	void ClearSkills();

	/** 블루프린트 AnimNotify_SkillHit 등에서 호출할 함수 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Skill")
	void ExecuteSkillEffect();

	void ProcessMeleeHit(const FST_Skill* SkillData, AActor* Target);

	void SpawnProjectile(const FST_Skill* SkillData, AActor* Target, FVector MuzzleLoc);

	void UpdateDebugWidget();

protected:
	/** 공격 종료 시 상태 초기화를 위한 콜백 */
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	UPROPERTY() class ACharacter* OwnerChar;

	// 스킬 데이터 및 쿨타임 관리 (기존 유지)
	UPROPERTY(EditAnywhere, Category = "RTS|Data")
	TMap<FName, FST_Skill> m_SkillMap;

	TMap<FName, float> m_CooldownMap;

	/** 현재 실행 중인 스킬의 이름을 저장 (Notify 시 데이터 참조용) */
	FName CurrentActiveSkillName;

protected:
	/** 스킬 정보를 참조할 데이터 테이블 */
	UPROPERTY(EditAnywhere, Category = "RTS|Data")
	class UDataTable* SkillTable;

public:
	UPROPERTY(BlueprintAssignable, Category = "RTS|Events")
	FOnSkillStarted OnSkillStarted;
};
