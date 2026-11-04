// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Global/RTSItemTypes.h"
#include "RtsGameInstance.generated.h"

class URTSItemDefinitionBase;
class URTSUnitDefinition;
class URTSUnitJobDefinition;

/**
 * 
 */
UCLASS()
class PROJECTRTS_API URtsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDataTable* GetSkillDataTable();
	bool TryGetJobDefinitionById(FName JobId, URTSUnitJobDefinition*& OutJobDefinition);
	URTSUnitJobDefinition* GetJobDefinitionById(FName JobId);
	bool TryGetUnitDefinitionByAssetId(const FPrimaryAssetId& UnitDefinitionId, URTSUnitDefinition*& OutUnitDefinition);
	URTSUnitDefinition* GetUnitDefinitionByAssetId(const FPrimaryAssetId& UnitDefinitionId);
	bool TryGetUnitDefinitionById(FName UnitId, URTSUnitDefinition*& OutUnitDefinition);
	URTSUnitDefinition* GetUnitDefinitionById(FName UnitId);
	bool TryGetItemDefinitionById(FName ItemId, URTSItemDefinitionBase*& OutItemDefinition);
	URTSItemDefinitionBase* GetItemDefinitionById(FName ItemId);

private:
	// === 캐싱용 포인터만 남깁니다. Config나 EditAnywhere는 불필요합니다. ===
	UPROPERTY()
	UDataTable* CachedSkillTable = nullptr;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<URTSUnitDefinition>> CachedUnitDefinitionsById;

	UPROPERTY(Transient)
	TMap<FPrimaryAssetId, TObjectPtr<URTSUnitDefinition>> CachedUnitDefinitionsByAssetId;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<URTSItemDefinitionBase>> CachedItemDefinitionsById;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<URTSUnitJobDefinition>> CachedJobDefinitionsById;

	bool bUnitDefinitionCacheBuilt = false;
	bool bItemDefinitionCacheBuilt = false;
	bool bJobDefinitionCacheBuilt = false;

	void BuildUnitDefinitionCacheIfNeeded();
	void BuildItemDefinitionCacheIfNeeded();
	void BuildJobDefinitionCacheIfNeeded();
};
