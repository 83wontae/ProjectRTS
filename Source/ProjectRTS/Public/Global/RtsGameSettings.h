// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "RtsGameSettings.generated.h"

/**
 * */
UCLASS(Config = Game, defaultconfig, Meta = (DisplayName = "RTS Game Settings"))
class PROJECTRTS_API URtsGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URtsGameSettings();

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
	virtual FName GetSectionName() const override { return TEXT("RTSGameSettings"); }

public:
	/** 유닛 데이터 테이블 경로 (DT_Unit) */
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	FSoftObjectPath UnitDataTablePath;

	/** 무기 데이터 테이블 경로 (DT_Weapon) */
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	FSoftObjectPath WeaponDataTablePath;

	/** 방어구 데이터 테이블 경로 (DT_Armor) */
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	FSoftObjectPath ArmorDataTablePath;

	/** 직업 데이터 테이블 경로 (DT_UnitJob) */
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	FSoftObjectPath JobDataTablePath;

public:
	/** 캐싱된 유닛 데이터 테이블을 가져옵니다. 없으면 TryLoad를 실행합니다. */
	UDataTable* GetUnitDataTable() const;

	/** 캐싱된 무기 데이터 테이블을 가져옵니다. 없으면 TryLoad를 실행합니다. */
	UDataTable* GetWeaponDataTable() const;

	/** 캐싱된 방어구 데이터 테이블을 가져옵니다. 없으면 TryLoad를 실행합니다. */
	UDataTable* GetArmorDataTable() const;

	/** 캐싱된 직업 데이터 테이블을 가져옵니다. 없으면 TryLoad를 실행합니다. */
	UDataTable* GetJobDataTable() const;

private:
	UPROPERTY()
	mutable UDataTable* CachedUnitTable = nullptr;

	/** 무기 테이블 캐싱용 포인터 */
	UPROPERTY()
	mutable UDataTable* CachedWeaponTable = nullptr;

	/** 방어구 테이블 캐싱용 포인터 */
	UPROPERTY()
	mutable UDataTable* CachedArmorTable = nullptr;

	/** 직업 테이블 캐싱용 포인터 */
	UPROPERTY()
	mutable UDataTable* CachedJobTable = nullptr;
};

namespace RtsSettings
{
	FORCEINLINE const URtsGameSettings* GetRtsSettings()
	{
		return GetDefault<URtsGameSettings>();
	}

	FORCEINLINE UDataTable* GetUnitTable()
	{
		return GetDefault<URtsGameSettings>()->GetUnitDataTable();
	}

	FORCEINLINE UDataTable* GetWeaponTable()
	{
		return GetDefault<URtsGameSettings>()->GetWeaponDataTable();
	}

	FORCEINLINE UDataTable* GetJobTable()
	{
		return GetDefault<URtsGameSettings>()->GetJobDataTable();
	}

	FORCEINLINE UDataTable* GetArmorTable()
	{
		return GetDefault<URtsGameSettings>()->GetArmorDataTable();
	}
}