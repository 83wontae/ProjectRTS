#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Global/RTSItemTypes.h"
#include "RtsSettingsBlueprintLibrary.generated.h"

class URTSItemDefinitionBase;

UCLASS()
class PROJECTRTS_API URtsSettingsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "RTS|Settings", meta = (WorldContext = "WorldContextObject"))
	static URTSItemDefinitionBase* GetItemDefinition(const UObject* WorldContextObject, FName ItemId);

	UFUNCTION(BlueprintCallable, Category = "RTS|Settings", meta = (WorldContext = "WorldContextObject"))
	static bool TryGetItemDefinition(const UObject* WorldContextObject, FName ItemId, URTSItemDefinitionBase*& OutItemDefinition);
};
