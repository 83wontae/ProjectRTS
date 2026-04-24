// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Global/ProjectRTSTypes.h" // 데이터 구조체 참조를 위해 포함
#include "RtsSaveGame.generated.h"

/**
 * 하드디스크에 바이너리 형태로 저장될 실제 세이브 파일 객체입니다.
 */
UCLASS()
class PROJECTRTS_API URtsSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    URtsSaveGame();

    /** * 실제 저장될 데이터 덩어리입니다.
     * 여기에 인재 리스트, 자원, 진행도 등이 모두 담깁니다.
     */
    UPROPERTY(VisibleAnywhere, Category = "SaveData")
    FST_SaveGameData SaveData;

    /** 세이브 파일의 식별을 위한 슬롯 이름 및 인덱스 (기본값 설정용) */
    UPROPERTY(VisibleAnywhere, Category = "SaveInfo")
    FString SaveSlotName;

    UPROPERTY(VisibleAnywhere, Category = "SaveInfo")
    int32 SaveIndex;
};