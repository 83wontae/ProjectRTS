// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Global/ProjectRTSTypes.h"
#include "SpawnAreaUnit.generated.h"

class ARtsUnitCharacter;
class AAIController;
class URTSWeaponDefinition;

UCLASS()
class PROJECTRTS_API ASpawnAreaUnit : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnAreaUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable, Category = "RTS|Spawner")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "RTS|Spawner")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "RTS|Spawner")
	void PauseSpawning();

	UFUNCTION(BlueprintCallable, Category = "RTS|Spawner")
	void ResumeSpawning();

	UFUNCTION(BlueprintPure, Category = "RTS|Spawner")
	bool IsPausedByLimit() const { return bPausedByLimit; }

public:
	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Scene")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner")
	TSubclassOf<ARtsUnitCharacter> UnitClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner")
	TSubclassOf<AAIController> AIControllerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner")
	int32 m_Faction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner|Weapon")
	TSoftObjectPtr<URTSWeaponDefinition> LeftWeaponDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner|Weapon")
	TSoftObjectPtr<URTSWeaponDefinition> RightWeaponDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Spawner|Stats", meta = (InlineEditConditionToggle))
	bool bOverrideBaseCombatStats = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Spawner|Stats")
	FST_CombatStats BaseCombatStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner|Loop", meta = (ClampMin = "0.1"))
	float SpawnIntervalSeconds = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner|Loop", meta = (ClampMin = "1"))
	int32 SpawnPerInterval = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner|Limit", meta = (ClampMin = "0"))
	int32 MaxTotalSpawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner|Limit", meta = (ClampMin = "0"))
	int32 MaxAliveSpawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Spawner|Loop")
	bool bStartLoopOnBeginPlay = true;

private:
	FTimerHandle SpawnTimerHandle;

	UPROPERTY(VisibleAnywhere, Category = "RTS|Spawner|Runtime")
	int32 TotalSpawnedCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "RTS|Spawner|Runtime")
	bool bPausedByLimit = false;

	UPROPERTY(VisibleAnywhere, Category = "RTS|Spawner|Runtime")
	TArray<TWeakObjectPtr<ARtsUnitCharacter>> AliveSpawnedUnits;

	void SpawnLoopTick();
	bool TrySpawnUnit();
	bool CanSpawn() const;
	void RefreshAliveUnits();
};
