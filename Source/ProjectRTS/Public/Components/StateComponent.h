#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/ProjectRTSTypes.h"
#include "StateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateHp, double, CurHp, double, MaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEventDeath);
/** 경험치 변경 알림 (현재 경험치, 필요 경험치) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpChanged, double, CurrentExp, double, RequiredExp);
/** 레벨업 알림 (새로운 레벨) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API UStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStateComponent();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    /** --- [경험치 및 성장 시스템] --- */

    /** 처치 시 지급될 기초 경험치 (기획 수치) */
    UPROPERTY(EditAnywhere, Category = "RTS|State")
    double m_BaseExpReward = 1.0f;

    /** 특정 레벨에서 다음 레벨로 가기 위한 요구 경험치 계산 */
    UFUNCTION(BlueprintPure, Category = "RTS|Level")
    double GetRequiredExpForLevel(int32 Level) const;

    /** 이 유닛이 처치되었을 때 가해자에게 줄 경험치 계산 */
    UFUNCTION(BlueprintPure, Category = "RTS|Level")
    double CalculateExpReward() const;

    UFUNCTION(BlueprintCallable, Category = "RTS|Level")
    void AddExp(double Amount);

    UFUNCTION(BlueprintCallable, Category = "RTS|Level")
    void ChangeJob(FName NewJobRowName);

    void RefreshFinalStats();
    void UpdateCombatStats();

    /** --- [기존 전투 및 타겟 시스템] --- */
    UFUNCTION(BlueprintCallable, Category = "RTS|State")
    void AddDamage(AController* EventInstigator, double Damage);

    UFUNCTION(BlueprintPure, Category = "RTS|State")
    bool IsDeath() const;

    UFUNCTION(BlueprintCallable, Category = "RTS|State")
    void SetAggroTarget(AActor* NewTarget) { m_AggroTarget = NewTarget; }

    UFUNCTION(BlueprintPure, Category = "RTS|State")
    AActor* GetAggroTarget() const { return m_AggroTarget.Get(); }

    UFUNCTION(BlueprintPure, Category = "RTS|State")
    int32 GetFaction() const { return m_Faction; }

    UFUNCTION(BlueprintPure, Category = "RTS|State")
    double GetTotalAttack() const { return m_TotalCombatStats.Attack; }

    UFUNCTION(BlueprintPure, Category = "RTS|State")
    double GetCurrentHp() const { return m_CurHp; }

    UFUNCTION(BlueprintPure, Category = "RTS|State")
    double GetMaxHp() const { return m_TotalCombatStats.MaxHp; }

    // 장비 컴포넌트에서 무기/방어구 수치가 바뀔 때 호출합니다.
    UFUNCTION(BlueprintCallable, Category = "RTS|State")
    void SetEquipCombatStats(const FST_CombatStats& NewEquipStats);

    /** 스탯 정보를 머리 위 위젯에 갱신하는 함수 */
    void UpdateDebugWidget();

private:
    // [근본] 모든 유닛이 동일하게 시작하는 기초 값 (1.0)
    const FST_Attributes m_OriginAttributes = FST_Attributes(1.0, 1.0, 1.0, 1.0);

    // [성장] 레벨업 시 현재 직업의 성장치가 누적되는 변수
    UPROPERTY(VisibleAnywhere, Category = "RTS|State")
    FST_Attributes m_AccumulatedAttributes;

    // [최종 속성] Origin + Accumulated (순수 체급)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|State", meta = (AllowPrivateAccess = "true"))
    FST_Attributes m_CurrentAttributes;

    UPROPERTY(VisibleAnywhere, Category = "RTS|State")
    int32 m_CurrentLevel = 1;

    UPROPERTY(VisibleAnywhere, Category = "RTS|State")
    double m_CurrentExp = 0.0;

	UPROPERTY(VisibleAnywhere, Category = "RTS|State")
    FName m_JobRowName;

    // Attributes(힘,민,지)에서 계산된 순수 본체 능력치
    UPROPERTY(BlueprintReadOnly, Category = "RTS|State", meta = (AllowPrivateAccess = "true"))
    FST_CombatStats m_BaseCombatStats;

    // 아이템으로 추가된 능력치 (SetEquipCombatStats로 업데이트됨)
    UPROPERTY(BlueprintReadOnly, Category = "RTS|State", meta = (AllowPrivateAccess = "true"))
    FST_CombatStats m_EquipCombatStats;

    UPROPERTY(BlueprintReadOnly, Category = "RTS|State", meta = (AllowPrivateAccess = "true"))
    FST_CombatStats m_TotalCombatStats;

    void RecalculateTotalStats();

public:
    UPROPERTY(BlueprintAssignable, Category = "RTS|State|Events")
    FOnUpdateHp EventDispatcher_UpdateHp;

    UPROPERTY(BlueprintAssignable, Category = "RTS|State|Events")
    FOnEventDeath EventDispatcher_EventDeath;

    /** 블루프린트에서 바인딩 가능한 이벤트 디스패처 */
    UPROPERTY(BlueprintAssignable, Category = "RTS|State|Events")
    FOnExpChanged EventDispatcher_ExpChanged;

    UPROPERTY(BlueprintAssignable, Category = "RTS|State|Events")
    FOnLevelUp EventDispatcher_LevelUp;

protected:

    UPROPERTY(ReplicatedUsing = OnRep_CurHp, BlueprintReadWrite, Category = "RTS|State")
    double m_CurHp;

    /** [기존 팀 시스템] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
    int32 m_Faction;

    /** [기존 어그로 타겟] */
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "RTS|State")
    TWeakObjectPtr<AActor> m_AggroTarget;

    UPROPERTY(EditAnywhere, Category = "RTS|Data")
    class UDataTable* JobDataTable;

private:
    UPROPERTY() class ACharacter* OwnerChar;
    FTimerHandle DestroyTimerHandle;
    void HandleDeath();
    void DestroyDelay();
    UFUNCTION() void OnRep_CurHp();

public:
    /** --- [저장 및 로드 시스템] --- */

    /** 유닛의 고용 정보와 성장 상태를 모두 포함한 통합 데이터를 반환합니다. */
    UFUNCTION(BlueprintCallable, Category = "RTS|Save")
    FST_UnitSaveData GetFullSaveData();

    /** 통합 데이터를 받아 유닛의 상태와 장비를 한 번에 복구합니다. */
    UFUNCTION(BlueprintCallable, Category = "RTS|Save")
    void LoadFromFullSaveData(const FST_UnitSaveData& InData);
};