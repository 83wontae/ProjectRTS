// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/RtsGameInstance.h"
#include "Global/RtsGameSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Global/RTSItemDefinition.h"
#include "Global/RTSUnitDefinition.h"

void URtsGameInstance::BuildUnitDefinitionCacheIfNeeded()
{
	if (bUnitDefinitionCacheBuilt)
	{
		return;
	}

	bUnitDefinitionCacheBuilt = true;
	CachedUnitDefinitionsById.Reset();
	CachedUnitDefinitionsByAssetId.Reset();

	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(URTSUnitDefinition::AssetType, AssetIds);

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
		UObject* LoadedObject = AssetPath.IsValid() ? AssetPath.TryLoad() : nullptr;
		URTSUnitDefinition* Definition = Cast<URTSUnitDefinition>(LoadedObject);
		if (!Definition)
		{
			continue;
		}

		CachedUnitDefinitionsByAssetId.Add(AssetId, Definition);

		if (!Definition->UnitId.IsNone())
		{
			CachedUnitDefinitionsById.Add(Definition->UnitId, Definition);
		}

		if (!AssetId.PrimaryAssetName.IsNone())
		{
			CachedUnitDefinitionsById.Add(AssetId.PrimaryAssetName, Definition);
		}
	}

}

URTSUnitDefinition* URtsGameInstance::GetUnitDefinitionByAssetId(const FPrimaryAssetId& UnitDefinitionId)
{
	URTSUnitDefinition* UnitDefinition = nullptr;
	if (TryGetUnitDefinitionByAssetId(UnitDefinitionId, UnitDefinition))
	{
		return UnitDefinition;
	}
	return nullptr;
}

bool URtsGameInstance::TryGetUnitDefinitionByAssetId(const FPrimaryAssetId& UnitDefinitionId, URTSUnitDefinition*& OutUnitDefinition)
{
	OutUnitDefinition = nullptr;

	if (!UnitDefinitionId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TryGetUnitDefinitionByAssetId failed: UnitDefinitionId is invalid."));
		return false;
	}

	BuildUnitDefinitionCacheIfNeeded();

	if (TObjectPtr<URTSUnitDefinition>* Found = CachedUnitDefinitionsByAssetId.Find(UnitDefinitionId))
	{
		OutUnitDefinition = Found->Get();
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("TryGetUnitDefinitionByAssetId failed: UnitDefinitionId '%s' was not found."), *UnitDefinitionId.ToString());
	return false;
}

URTSUnitDefinition* URtsGameInstance::GetUnitDefinitionById(FName UnitId)
{
	URTSUnitDefinition* UnitDefinition = nullptr;
	if (TryGetUnitDefinitionById(UnitId, UnitDefinition))
	{
		return UnitDefinition;
	}
	return nullptr;
}

bool URtsGameInstance::TryGetUnitDefinitionById(FName UnitId, URTSUnitDefinition*& OutUnitDefinition)
{
	OutUnitDefinition = nullptr;

	if (UnitId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TryGetUnitDefinitionById failed: UnitId is None."));
		return false;
	}

	BuildUnitDefinitionCacheIfNeeded();

	if (TObjectPtr<URTSUnitDefinition>* Found = CachedUnitDefinitionsById.Find(UnitId))
	{
		OutUnitDefinition = Found->Get();
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("TryGetUnitDefinitionById failed: UnitId '%s' was not found."), *UnitId.ToString());
	return false;
}

UDataTable* URtsGameInstance::GetJobDataTable()
{
	if (CachedJobTable) return CachedJobTable;

	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	if (Settings && !Settings->JobDataTable.IsNull())
	{
		CachedJobTable = Settings->JobDataTable.LoadSynchronous();
	}
	return CachedJobTable;
}

UDataTable* URtsGameInstance::GetSkillDataTable()
{
	// 1. 이미 캐싱되어 있다면 바로 반환
	if (CachedSkillTable != nullptr)
	{
		return CachedSkillTable;
	}

	// 2. 캐싱되어 있지 않다면 Settings에서 경로를 읽어와 로드
	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	if (Settings && Settings->SkillDataTable.ToSoftObjectPath().IsValid())
	{
		CachedSkillTable = Settings->SkillDataTable.LoadSynchronous();
	}

	return CachedSkillTable;
}

void URtsGameInstance::BuildItemDefinitionCacheIfNeeded()
{
	if (bItemDefinitionCacheBuilt)
	{
		return;
	}

	bItemDefinitionCacheBuilt = true;
	CachedItemDefinitionsById.Reset();

	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(URTSItemDefinitionBase::AssetType, AssetIds);

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
		UObject* LoadedObject = AssetPath.IsValid() ? AssetPath.TryLoad() : nullptr;
		URTSItemDefinitionBase* Definition = Cast<URTSItemDefinitionBase>(LoadedObject);
		if (!Definition)
		{
			continue;
		}

		if (!Definition->ItemId.IsNone())
		{
			CachedItemDefinitionsById.Add(Definition->ItemId, Definition);
		}

		if (!AssetId.PrimaryAssetName.IsNone())
		{
			CachedItemDefinitionsById.Add(AssetId.PrimaryAssetName, Definition);
		}
	}
}

URTSItemDefinitionBase* URtsGameInstance::GetItemDefinitionById(FName ItemId)
{
	URTSItemDefinitionBase* ItemDefinition = nullptr;
	if (TryGetItemDefinitionById(ItemId, ItemDefinition))
	{
		return ItemDefinition;
	}
	return nullptr;
}

bool URtsGameInstance::TryGetItemDefinitionById(FName ItemId, URTSItemDefinitionBase*& OutItemDefinition)
{
	OutItemDefinition = nullptr;

	if (ItemId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TryGetItemDefinitionById failed: ItemId is None."));
		return false;
	}

	BuildItemDefinitionCacheIfNeeded();

	if (TObjectPtr<URTSItemDefinitionBase>* Found = CachedItemDefinitionsById.Find(ItemId))
	{
		OutItemDefinition = Found->Get();
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("TryGetItemDefinitionById failed: ItemId '%s' was not found."), *ItemId.ToString());
	return false;
}
