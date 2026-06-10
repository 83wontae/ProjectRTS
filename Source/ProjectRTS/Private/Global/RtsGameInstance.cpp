// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/RtsGameInstance.h"
#include "Global/RtsGameSettings.h"

UDataTable* URtsGameInstance::GetUnitDataTable()
{
	if (CachedUnitTable) return CachedUnitTable;

	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	// IsNull()로 경로가 설정되어 있는지 확인 후 안전하게 로드합니다.
	if (Settings && !Settings->UnitDataTable.IsNull())
	{
		CachedUnitTable = Settings->UnitDataTable.LoadSynchronous();
	}
	return CachedUnitTable;
}

UDataTable* URtsGameInstance::GetWeaponDataTable()
{
	if (CachedWeaponTable) return CachedWeaponTable;

	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	if (Settings && !Settings->WeaponDataTable.IsNull())
	{
		CachedWeaponTable = Settings->WeaponDataTable.LoadSynchronous();
	}
	return CachedWeaponTable;
}

UDataTable* URtsGameInstance::GetArmorDataTable()
{
	if (CachedArmorTable) return CachedArmorTable;

	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	if (Settings && !Settings->ArmorDataTable.IsNull())
	{
		CachedArmorTable = Settings->ArmorDataTable.LoadSynchronous();
	}
	return CachedArmorTable;
}

UDataTable* URtsGameInstance::GetJobDataTable()
{
	if (CachedJobTable) return CachedJobTable;

	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	if (Settings && !Settings->JobDataTable.IsNull())
	{
		CachedJobTable = Settings->JobDataTable.LoadSynchronous();
	}
	return CachedJobTable;
}

UDataTable* URtsGameInstance::GetSkillDataTable()
{
	// 1. 이미 캐싱되어 있다면 바로 반환
	if (CachedSkillTable != nullptr)
	{
		return CachedSkillTable;
	}

	// 2. 캐싱되어 있지 않다면 Settings에서 경로를 읽어와 로드
	const URtsGameSettings* Settings = GetDefault<URtsGameSettings>();
	if (Settings && Settings->SkillDataTable.ToSoftObjectPath().IsValid())
	{
		CachedSkillTable = Settings->SkillDataTable.LoadSynchronous();
	}

	return CachedSkillTable;
}