// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RtsGameMode.generated.h"

/**
 * 게임 월드가 시작될 때 총괄하는 RTS 게임 모드 클래스입니다.
 */
UCLASS()
class PROJECTRTS_API ARtsGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	// 게임 모드가 시작될 때 최초 1회 호출되는 함수
	virtual void BeginPlay() override;
};