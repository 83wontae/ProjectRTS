// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DebugWidgetComponent.generated.h"


UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API UDebugWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

protected:
    // 생성자에서 Tick을 켜야 합니다.
    UDebugWidgetComponent();

public:
    // 매 프레임 위젯 상태를 감시할 Tick 함수
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // 전송 대기 중인 최신 로그 보관함
    TArray<FString> PendingLogs;
    bool bHasPendingLogs = false;

public:
	/** 리스트 업데이트 (위젯 내부의 함수를 찾아 호출) */
	UFUNCTION(BlueprintCallable, Category = "RTS|Debug")
	void UpdateLogList(const TArray<FString>& NewLogs);
};