// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Global/ProjectRTSTypes.h"
#include "ProjectileInterface.generated.h"

// 이 클래스는 엔진 내부 처리를 위한 클래스이므로 수정하지 마세요.
UINTERFACE(MinimalAPI, BlueprintType)
class UProjectileInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTRTS_API IProjectileInterface
{
	GENERATED_BODY()

public:
	/** 발사체의 초기 정보를 설정하는 공통 함수 */
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void SetupProjectile(AActor* InOwner, AActor* InTarget, double InDamage);
};