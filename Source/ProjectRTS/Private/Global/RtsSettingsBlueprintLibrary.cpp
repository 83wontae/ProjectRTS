#include "Global/RtsSettingsBlueprintLibrary.h"
#include "Global/RtsGameSettings.h"
#include "Global/RTSUnitDefinition.h"

URTSUnitDefinition* URtsSettingsBlueprintLibrary::GetUnitDefinition(const UObject* WorldContextObject, FName UnitAssetName)
{
	const FPrimaryAssetId UnitDefinitionId(URTSUnitDefinition::AssetType, UnitAssetName);
	return RtsSettings::GetUnitDefinition(WorldContextObject, UnitDefinitionId);
}

bool URtsSettingsBlueprintLibrary::TryGetUnitDefinition(const UObject* WorldContextObject, FName UnitAssetName, URTSUnitDefinition*& OutUnitDefinition)
{
	const FPrimaryAssetId UnitDefinitionId(URTSUnitDefinition::AssetType, UnitAssetName);
	return RtsSettings::TryGetUnitDefinition(WorldContextObject, UnitDefinitionId, OutUnitDefinition);
}

URTSItemDefinitionBase* URtsSettingsBlueprintLibrary::GetItemDefinition(const UObject* WorldContextObject, FName ItemId)
{
	return RtsSettings::GetItemDefinition(WorldContextObject, ItemId);
}

bool URtsSettingsBlueprintLibrary::TryGetItemDefinition(const UObject* WorldContextObject, FName ItemId, URTSItemDefinitionBase*& OutItemDefinition)
{
	return RtsSettings::TryGetItemDefinition(WorldContextObject, ItemId, OutItemDefinition);
}
