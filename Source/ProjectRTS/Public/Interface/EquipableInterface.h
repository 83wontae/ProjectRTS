#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EquipableInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UEquipableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTRTS_API IEquipableInterface
{
	GENERATED_BODY()

public:
	/** 무기나 장비가 부착될 기준 SkeletalMeshComponent 반환 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equip")
	USkeletalMeshComponent* GetTargetAttachMesh() const;
};