#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Global/ProjectRTSTypes.h"
#include "WeaponInterface.generated.h"

// 이 클래스는 엔진 내부 처리를 위한 클래스이므로 수정하지 마세요.
UINTERFACE(MinimalAPI, BlueprintType)
class UWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * RTS 프로젝트의 무기 액터 또는 무기를 사용하는 주체가 구현할 인터페이스입니다.
 */
class PROJECTRTS_API IWeaponInterface
{
	GENERATED_BODY()

public:
	/** 무기의 기본 데이터를 반환합니다. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RTS|Weapon")
	FST_Weapon GetWeaponData() const;

	/** 무기를 보이거나 숨깁니다 (장착/해제 시 활용). */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RTS|Weapon")
	void SetWeaponVisibility(bool bVisible);

	/** 현재 무기의 소유자를 반환합니다. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RTS|Weapon")
	class AActor* GetWeaponOwner() const;
};