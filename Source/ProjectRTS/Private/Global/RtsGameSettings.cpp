// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/RtsGameSettings.h"

URtsGameSettings::URtsGameSettings()
{}

UDataTable* URtsGameSettings::GetUnitDataTable() const
{
	if (CachedUnitTable) return CachedUnitTable;

	if (UnitDataTablePath.IsValid())
	{
		CachedUnitTable = Cast<UDataTable>(UnitDataTablePath.TryLoad());
	}
	return CachedUnitTable;
}

UDataTable* URtsGameSettings::GetWeaponDataTable() const
{
	// 이미 로드된 캐시가 있다면 즉시 포인터 반환
	if (CachedWeaponTable) return CachedWeaponTable;

	if (WeaponDataTablePath.IsValid())
	{
		// 처음 호출될 때만 안전하게 디스크에서 메모리로 로드
		CachedWeaponTable = Cast<UDataTable>(WeaponDataTablePath.TryLoad());
	}
	return CachedWeaponTable;
}

UDataTable* URtsGameSettings::GetArmorDataTable() const
{
	// 이미 로드된 캐시가 있다면 즉시 포인터 반환
	if (CachedArmorTable) return CachedArmorTable;

	if (ArmorDataTablePath.IsValid())
	{
		// 처음 호출될 때만 안전하게 디스크에서 메모리로 로드
		CachedArmorTable = Cast<UDataTable>(ArmorDataTablePath.TryLoad());
	}
	return CachedArmorTable;
}

UDataTable* URtsGameSettings::GetJobDataTable() const
{
	// 이미 로드된 캐시가 있다면 즉시 포인터 반환
	if (CachedJobTable) return CachedJobTable;

	if (JobDataTablePath.IsValid())
	{
		// 처음 호출될 때만 안전하게 디스크에서 메모리로 로드
		CachedJobTable = Cast<UDataTable>(JobDataTablePath.TryLoad());
	}
	return CachedJobTable;
}
