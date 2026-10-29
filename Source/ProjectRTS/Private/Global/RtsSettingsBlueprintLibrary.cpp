#include "Global/RtsSettingsBlueprintLibrary.h"
#include "Global/RtsGameSettings.h"

URTSItemDefinitionBase* URtsSettingsBlueprintLibrary::GetItemDefinition(const UObject* WorldContextObject, FName ItemId)
{
	return RtsSettings::GetItemDefinition(WorldContextObject, ItemId);
}
