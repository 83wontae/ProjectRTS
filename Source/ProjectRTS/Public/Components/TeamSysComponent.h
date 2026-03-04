// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeamSysComponent.generated.h"

// 팀 변경 및 리더 변경을 알리기 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamChanged, int32, NewTeamIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeaderChanged, AActor*, NewLeader);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API UTeamSysComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTeamSysComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 네트워크 복제 속성 정의를 위한 필수 오버라이드
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// OnRep 함수 (변수가 복제될 때 클라이언트에서 실행)
	UFUNCTION()
	void OnRep_TeamIndex();

	UFUNCTION()
	void OnRep_Leader();

	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(int32 NewTeamIndex);

	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetLeader(AActor* NewLeader);

	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsAlly(AActor* TargetActor) const;

	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsEnemy(AActor* TargetActor) const;

	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsAllyByTeamIndex(int32 TeamIndex) const;

	UFUNCTION(BlueprintPure, Category = "Team")
	bool IsEnemyByTeamIndex(int32 TeamIndex) const;

public:
	// --- 팀 시스템 변수 및 함수 ---
	UPROPERTY(ReplicatedUsing = OnRep_TeamIndex, BlueprintReadWrite, Category = "Team")
	int32 m_TeamIndex;

	UPROPERTY(ReplicatedUsing = OnRep_Leader, BlueprintReadWrite, Category = "Team")
	AActor* m_Leader;

	UPROPERTY(BlueprintAssignable, Category = "Team|Events")
	FOnTeamChanged OnTeamChanged;

	UPROPERTY(BlueprintAssignable, Category = "Team|Events")
	FOnLeaderChanged OnLeaderChanged;
};