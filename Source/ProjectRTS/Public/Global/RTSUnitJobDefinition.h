#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Global/RTSUnitTypes.h"
#include "RTSUnitJobDefinition.generated.h"

UCLASS(BlueprintType)
class PROJECTRTS_API URTSUnitJobDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType AssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(AssetType, GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job")
	FName JobId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job Info")
	FText JobDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Job Info")
	TObjectPtr<UTexture2D> JobIcon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Stats")
	FST_Attributes BaseAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Growth Stats")
	FST_Attributes GrowthAttributes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
	TArray<FName> StartingSkills;
};
