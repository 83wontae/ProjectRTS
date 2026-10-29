#include "Global/RTSAssetManager.h"
#include "Engine/Engine.h"

URTSAssetManager& URTSAssetManager::Get()
{
	return *CastChecked<URTSAssetManager>(GEngine->AssetManager);
}

void URTSAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
}
