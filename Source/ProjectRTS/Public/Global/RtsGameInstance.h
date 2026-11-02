// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Global/RTSItemTypes.h"
#include "RtsGameInstance.generated.h"

class URTSItemDefinitionBase;
class URTSUnitDefinition;

/**
 * 
 */
UCLASS()
class PROJECTRTS_API URtsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDataTable* GetWeaponDataTable();
	UDataTable* GetArmorDataTable();
	UDataTable* GetJobDataTable();
	UDataTable* GetSkillDataTable();
	bool TryGetUnitDefinitionByAssetId(const FPrimaryAssetId& UnitDefinitionId, URTSUnitDefinition*& OutUnitDefinition);
	URTSUnitDefinition* GetUnitDefinitionByAssetId(const FPrimaryAssetId& UnitDefinitionId);
	bool TryGetUnitDefinitionById(FName UnitId, URTSUnitDefinition*& OutUnitDefinition);
	URTSUnitDefinition* GetUnitDefinitionById(FName UnitId);
	bool TryGetItemDefinitionById(FName ItemId, URTSItemDefinitionBase*& OutItemDefinition);
	URTSItemDefinitionBase* GetItemDefinitionById(FName ItemId);

private:
	// === 캐싱용 포인터만 남깁니다. Config나 EditAnywhere는 불필요합니다. ===
	UPROPERTY()
	UDataTable* CachedWeaponTable = nullptr;

	UPROPERTY()
	UDataTable* CachedArmorTable = nullptr;

	UPROPERTY()
	UDataTable* CachedJobTable = nullptr;

	UPROPERTY()
	UDataTable* CachedSkillTable = nullptr;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<URTSUnitDefinition>> CachedUnitDefinitionsById;

	UPROPERTY(Transient)
	TMap<FPrimaryAssetId, TObjectPtr<URTSUnitDefinition>> CachedUnitDefinitionsByAssetId;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<URTSItemDefinitionBase>> CachedItemDefinitionsById;

	bool bUnitDefinitionCacheBuilt = false;
	bool bItemDefinitionCacheBuilt = false;

	void BuildUnitDefinitionCacheIfNeeded();
	void BuildItemDefinitionCacheIfNeeded();
};
