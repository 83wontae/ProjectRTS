#include "Global/RtsSettingsBlueprintLibrary.h"
#include "Global/RtsGameSettings.h"

URTSItemDefinitionBase* URtsSettingsBlueprintLibrary::GetItemDefinition(const UObject* WorldContextObject, FName ItemId)
{
	return RtsSettings::GetItemDefinition(WorldContextObject, ItemId);
}

bool URtsSettingsBlueprintLibrary::TryGetItemDefinition(const UObject* WorldContextObject, FName ItemId, URTSItemDefinitionBase*& OutItemDefinition)
{
	return RtsSettings::TryGetItemDefinition(WorldContextObject, ItemId, OutItemDefinition);
}
