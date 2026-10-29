#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "RTSAssetManager.generated.h"

UCLASS()
class PROJECTRTS_API URTSAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static URTSAssetManager& Get();

	virtual void StartInitialLoading() override;
};
