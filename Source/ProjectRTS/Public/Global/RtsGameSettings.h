// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Global/RtsGameInstance.h"
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

	// === 이곳에 Config 데이터가 있어야 합니다 ===
	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> UnitDataTable;

	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> WeaponDataTable;

	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> ArmorDataTable;

	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> JobDataTable;

	UPROPERTY(Config, EditAnywhere, Category = "Data Tables")
	TSoftObjectPtr<UDataTable> SkillDataTable;
};

namespace RtsSettings
{
	// 게임 인스턴스를 가져오는 헬퍼 함수
	FORCEINLINE URtsGameInstance* GetRtsInstance(const UObject* WorldContextObject)
	{
		return Cast<URtsGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
	}

	FORCEINLINE UDataTable* GetUnitTable(const UObject* WorldContextObject)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetUnitDataTable();
		}
		return nullptr;
	}

	FORCEINLINE UDataTable* GetWeaponTable(const UObject* WorldContextObject)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetWeaponDataTable();
		}
		return nullptr;
	}

	FORCEINLINE UDataTable* GetJobTable(const UObject* WorldContextObject)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetJobDataTable();
		}
		return nullptr;
	}

	FORCEINLINE UDataTable* GetArmorTable(const UObject* WorldContextObject)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetArmorDataTable();
		}
		return nullptr;
	}

	FORCEINLINE UDataTable* GetSkillTable(const UObject* WorldContextObject)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetSkillDataTable();
		}
		return nullptr;
	}
}