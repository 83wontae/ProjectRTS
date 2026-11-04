// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/RtsGameInstance.h"
#include "Global/RtsGameSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Global/RTSItemDefinition.h"
#include "Global/RTSSkillDefinition.h"
#include "Global/RTSUnitJobDefinition.h"
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

void URtsGameInstance::BuildSkillDefinitionCacheIfNeeded()
{
	if (bSkillDefinitionCacheBuilt)
	{
		return;
	}

	bSkillDefinitionCacheBuilt = true;
	CachedSkillDefinitionsById.Reset();

	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(URTSSkillDefinition::AssetType, AssetIds);

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
		UObject* LoadedObject = AssetPath.IsValid() ? AssetPath.TryLoad() : nullptr;
		URTSSkillDefinition* Definition = Cast<URTSSkillDefinition>(LoadedObject);
		if (!Definition)
		{
			continue;
		}

		if (!Definition->SkillId.IsNone())
		{
			CachedSkillDefinitionsById.Add(Definition->SkillId, Definition);
		}

		if (!AssetId.PrimaryAssetName.IsNone())
		{
			CachedSkillDefinitionsById.Add(AssetId.PrimaryAssetName, Definition);
		}
	}
}

URTSSkillDefinition* URtsGameInstance::GetSkillDefinitionById(FName SkillId)
{
	URTSSkillDefinition* SkillDefinition = nullptr;
	if (TryGetSkillDefinitionById(SkillId, SkillDefinition))
	{
		return SkillDefinition;
	}
	return nullptr;
}

bool URtsGameInstance::TryGetSkillDefinitionById(FName SkillId, URTSSkillDefinition*& OutSkillDefinition)
{
	OutSkillDefinition = nullptr;

	if (SkillId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TryGetSkillDefinitionById failed: SkillId is None."));
		return false;
	}

	BuildSkillDefinitionCacheIfNeeded();

	if (TObjectPtr<URTSSkillDefinition>* Found = CachedSkillDefinitionsById.Find(SkillId))
	{
		OutSkillDefinition = Found->Get();
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("TryGetSkillDefinitionById failed: SkillId '%s' was not found."), *SkillId.ToString());
	return false;
}

void URtsGameInstance::BuildJobDefinitionCacheIfNeeded()
{
	if (bJobDefinitionCacheBuilt)
	{
		return;
	}

	bJobDefinitionCacheBuilt = true;
	CachedJobDefinitionsById.Reset();

	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(URTSUnitJobDefinition::AssetType, AssetIds);

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
		UObject* LoadedObject = AssetPath.IsValid() ? AssetPath.TryLoad() : nullptr;
		URTSUnitJobDefinition* Definition = Cast<URTSUnitJobDefinition>(LoadedObject);
		if (!Definition)
		{
			continue;
		}

		if (!Definition->JobId.IsNone())
		{
			CachedJobDefinitionsById.Add(Definition->JobId, Definition);
		}

		if (!AssetId.PrimaryAssetName.IsNone())
		{
			CachedJobDefinitionsById.Add(AssetId.PrimaryAssetName, Definition);
		}
	}
}

URTSUnitJobDefinition* URtsGameInstance::GetJobDefinitionById(FName JobId)
{
	URTSUnitJobDefinition* JobDefinition = nullptr;
	if (TryGetJobDefinitionById(JobId, JobDefinition))
	{
		return JobDefinition;
	}
	return nullptr;
}

bool URtsGameInstance::TryGetJobDefinitionById(FName JobId, URTSUnitJobDefinition*& OutJobDefinition)
{
	OutJobDefinition = nullptr;

	if (JobId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TryGetJobDefinitionById failed: JobId is None."));
		return false;
	}

	BuildJobDefinitionCacheIfNeeded();

	if (TObjectPtr<URTSUnitJobDefinition>* Found = CachedJobDefinitionsById.Find(JobId))
	{
		OutJobDefinition = Found->Get();
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("TryGetJobDefinitionById failed: JobId '%s' was not found."), *JobId.ToString());
	return false;
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
