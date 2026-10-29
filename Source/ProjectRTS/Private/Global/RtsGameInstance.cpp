// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/RtsGameInstance.h"
#include "Global/RtsGameSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Global/RTSItemDefinition.h"

UDataTable* URtsGameInstance::GetUnitDataTable()
{
	if (CachedUnitTable) return CachedUnitTable;

	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	// IsNull()로 경로가 설정되어 있는지 확인 후 안전하게 로드합니다.
	if (Settings && !Settings->UnitDataTable.IsNull())
	{
		CachedUnitTable = Settings->UnitDataTable.LoadSynchronous();
	}
	return CachedUnitTable;
}

UDataTable* URtsGameInstance::GetWeaponDataTable()
{
	if (CachedWeaponTable) return CachedWeaponTable;

	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	if (Settings && !Settings->WeaponDataTable.IsNull())
	{
		CachedWeaponTable = Settings->WeaponDataTable.LoadSynchronous();
	}
	return CachedWeaponTable;
}

UDataTable* URtsGameInstance::GetArmorDataTable()
{
	if (CachedArmorTable) return CachedArmorTable;

	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	if (Settings && !Settings->ArmorDataTable.IsNull())
	{
		CachedArmorTable = Settings->ArmorDataTable.LoadSynchronous();
	}
	return CachedArmorTable;
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
	if (ItemId.IsNone())
	{
		return nullptr;
	}

	BuildItemDefinitionCacheIfNeeded();

	if (TObjectPtr<URTSItemDefinitionBase>* Found = CachedItemDefinitionsById.Find(ItemId))
	{
		return Found->Get();
	}

	return nullptr;
}
