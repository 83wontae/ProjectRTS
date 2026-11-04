// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Global/RtsGameInstance.h"
#include "Global/RTSItemTypes.h"
#include "RtsGameSettings.generated.h"

class URTSItemDefinitionBase;
class URTSUnitDefinition;
class URTSUnitJobDefinition;

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

	FORCEINLINE URTSUnitDefinition* GetUnitDefinition(const UObject* WorldContextObject, const FPrimaryAssetId& UnitDefinitionId)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetUnitDefinitionByAssetId(UnitDefinitionId);
		}
		return nullptr;
	}

	FORCEINLINE bool TryGetUnitDefinition(const UObject* WorldContextObject, const FPrimaryAssetId& UnitDefinitionId, URTSUnitDefinition*& OutUnitDefinition)
	{
		OutUnitDefinition = nullptr;
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->TryGetUnitDefinitionByAssetId(UnitDefinitionId, OutUnitDefinition);
		}
		UE_LOG(LogTemp, Warning, TEXT("TryGetUnitDefinition failed: RtsGameInstance is null. UnitDefinitionId='%s'"), *UnitDefinitionId.ToString());
		return false;
	}

	FORCEINLINE URTSUnitJobDefinition* GetJobDefinition(const UObject* WorldContextObject, FName JobId)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetJobDefinitionById(JobId);
		}
		return nullptr;
	}

	FORCEINLINE bool TryGetJobDefinition(const UObject* WorldContextObject, FName JobId, URTSUnitJobDefinition*& OutJobDefinition)
	{
		OutJobDefinition = nullptr;
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->TryGetJobDefinitionById(JobId, OutJobDefinition);
		}
		UE_LOG(LogTemp, Warning, TEXT("TryGetJobDefinition failed: RtsGameInstance is null. JobId='%s'"), *JobId.ToString());
		return false;
	}

	FORCEINLINE UDataTable* GetSkillTable(const UObject* WorldContextObject)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetSkillDataTable();
		}
		return nullptr;
	}

	FORCEINLINE URTSItemDefinitionBase* GetItemDefinition(const UObject* WorldContextObject, FName ItemId)
	{
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->GetItemDefinitionById(ItemId);
		}
		return nullptr;
	}

	FORCEINLINE bool TryGetItemDefinition(const UObject* WorldContextObject, FName ItemId, URTSItemDefinitionBase*& OutItemDefinition)
	{
		OutItemDefinition = nullptr;
		if (URtsGameInstance* Instance = GetRtsInstance(WorldContextObject))
		{
			return Instance->TryGetItemDefinitionById(ItemId, OutItemDefinition);
		}
		UE_LOG(LogTemp, Warning, TEXT("TryGetItemDefinition failed: RtsGameInstance is null. ItemId='%s'"), *ItemId.ToString());
		return false;
	}
}