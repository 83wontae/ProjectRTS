#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Global/RTSCoreTypes.h"
#include "Global/RTSUnitTypes.h"
#include "RTSUnitDefinition.generated.h"

UCLASS(BlueprintType)
class PROJECTRTS_API URTSUnitDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType AssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(AssetType, GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unit")
	FName UnitId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unit")
	FString Name = TEXT("New Unit");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unit")
	TSubclassOf<AActor> UnitClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unit")
	FName UnitJobRowName = TEXT("Novice");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unit|Parts")
	TObjectPtr<USkeletalMesh> HeadPart = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Unit|Parts")
	TObjectPtr<USkeletalMesh> BodyPart = nullptr;
};

FORCEINLINE bool ToUnitLegacyData(const URTSUnitDefinition* Definition, FST_Unit& OutUnit)
{
	if (!Definition)
	{
		return false;
	}

	OutUnit.Name = Definition->Name;
	OutUnit.UnitClass = Definition->UnitClass;
	OutUnit.UnitJobRowName = Definition->UnitJobRowName;
	OutUnit.HeadPart = Definition->HeadPart;
	OutUnit.BodyPart = Definition->BodyPart;
	return true;
}
