#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Global/RTSItemTypes.h"
#include "RTSItemDefinition.generated.h"

UCLASS(Abstract, BlueprintType)
class PROJECTRTS_API URTSItemDefinitionBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType AssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(AssetType, GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemType ItemType = EItemType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UTexture2D> IconSprite;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 MaxStackCount = 1;
};

UCLASS(BlueprintType)
class PROJECTRTS_API URTSWeaponDefinition : public URTSItemDefinitionBase
{
	GENERATED_BODY()

public:
	URTSWeaponDefinition()
	{
		ItemType = EItemType::Weapon;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName SkillName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName SkillNameRide;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponHandConstraint HandConstraint = EWeaponHandConstraint::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName = TEXT("MuzzleSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	double AttackPower = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	double AttackRange = 0.0;
};

UCLASS(BlueprintType)
class PROJECTRTS_API URTSArmorDefinition : public URTSItemDefinitionBase
{
	GENERATED_BODY()

public:
	URTSArmorDefinition()
	{
		ItemType = EItemType::Armor;
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
	EEquipType EquipType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
	EUnitType UnitType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
};
