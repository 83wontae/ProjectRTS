// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RtsGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTRTS_API URtsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDataTable* GetUnitDataTable();
	UDataTable* GetWeaponDataTable();
	UDataTable* GetArmorDataTable();
	UDataTable* GetJobDataTable();
	UDataTable* GetSkillDataTable();

private:
	// === 캐싱용 포인터만 남깁니다. Config나 EditAnywhere는 불필요합니다. ===
	UPROPERTY()
	UDataTable* CachedUnitTable = nullptr;

	UPROPERTY()
	UDataTable* CachedWeaponTable = nullptr;

	UPROPERTY()
	UDataTable* CachedArmorTable = nullptr;

	UPROPERTY()
	UDataTable* CachedJobTable = nullptr;

	UPROPERTY()
	UDataTable* CachedSkillTable = nullptr;
};
