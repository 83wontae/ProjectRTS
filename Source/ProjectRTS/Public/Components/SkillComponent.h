// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/ProjectRTSTypes.h"
#include "SkillComponent.generated.h"

// 스킬 관련 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillStarted, FName, SkillName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownUpdated, FName, SkillName, float, RemainingTime);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTRTS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkillComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** 스킬 시전 시도 */
    UFUNCTION(BlueprintCallable, Category = "RTS|Skill")
    bool UseSkill(FName SkillName);

    /** 쿨타임 확인 */
    UFUNCTION(BlueprintPure, Category = "RTS|Skill")
    bool IsInCooldown(FName SkillName) const;

    /** 스킬 초기화 (DataTable 연동) */
    UFUNCTION(BlueprintCallable, Category = "RTS|Skill")
    void AddSkill(FName SkillName, const FST_Skill& SkillData);

public:
    // --- 이벤트 ---
    UPROPERTY(BlueprintAssignable, Category = "RTS|Skill|Events")
    FOnSkillStarted OnSkillStarted;

    UPROPERTY(BlueprintAssignable, Category = "RTS|Skill|Events")
    FOnCooldownUpdated OnCooldownUpdated;

private:
    // 스킬 데이터 저장 (이름 : 데이터)
    UPROPERTY()
    TMap<FName, FST_Skill> m_SkillMap;

    // 현재 진행 중인 쿨타임 (이름 : 남은 시간)
    UPROPERTY()
    TMap<FName, float> m_CooldownMap;

    UPROPERTY()
    class ACharacter* OwnerChar;
};
