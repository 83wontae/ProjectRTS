// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/ProjectRTSTypes.h" // 이전에 만든 구조체 헤더 포함
#include "StateComponent.generated.h"

// 이벤트 디스패처 선언 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateHp, double, CurHp, double, MaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEventDeath);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTRTS_API UStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStateComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    /** 데미지를 입히는 함수 */
    UFUNCTION(BlueprintCallable, Category = "RTS|State")
    void AddDamage(double Damage);

    /** 사망 여부 확인 */
    UFUNCTION(BlueprintPure, Category = "RTS|State")
    bool IsDeath() const;

    /** 데이터 테이블 정보를 기반으로 스탯 업데이트  */
    UFUNCTION(BlueprintCallable, Category = "RTS|State")
    void UpdateUnitData(const FST_Unit& NewData);

    /** 현재 HP 복제 시 실행될 함수 [cite: 1819, 2098-2099] */
    UFUNCTION()
    void OnRep_CurHp();

    /** 타겟 설정 함수 */
    UFUNCTION(BlueprintCallable, Category = "RTS|State")
    void SetAggroTarget(AActor* NewTarget) { m_AggroTarget = NewTarget; }

    /** 타겟 가져오기 함수 */
    UFUNCTION(BlueprintPure, Category = "RTS|State")
    AActor* GetAggroTarget() const { return m_AggroTarget.Get(); }

protected:
    /** 사망 시 처리 로직 **/
    void HandleDeath();

    /** 일정 시간 후 액터 파괴 **/
    void DestroyDelay();

public:
    UPROPERTY(ReplicatedUsing = OnRep_CurHp, BlueprintReadWrite, Category = "RTS|State")
    double m_CurHp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|State")
    double m_MaxHp = 100.0;

    UPROPERTY(BlueprintAssignable, Category = "RTS|State|Events")
    FOnUpdateHp EventDispatcher_UpdateHp;

    UPROPERTY(BlueprintAssignable, Category = "RTS|State|Events")
    FOnEventDeath EventDispatcher_EventDeath;

protected:
    /** 현재 추적 및 공격 중인 대상 (어그로 타겟) */
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "RTS|State")
    TWeakObjectPtr<AActor> m_AggroTarget;

private:
    UPROPERTY()
    class ACharacter* OwnerChar;

    FTimerHandle DestroyTimerHandle;
};
