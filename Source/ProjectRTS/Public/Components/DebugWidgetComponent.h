// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DebugWidgetComponent.generated.h"


UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API UDebugWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UDebugWidgetComponent();

	/** 리스트 업데이트 (위젯 내부의 함수를 찾아 호출) */
	UFUNCTION(BlueprintCallable, Category = "RTS|Debug")
	void UpdateLogList(const TArray<FString>& NewLogs);
};