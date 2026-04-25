// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Global/RTSUnitTypes.h" // FST_Attributes를 사용하기 위해 포함
#include "RTSSaveTypes.generated.h"

/** * 개별 인재(유닛)의 저장 데이터
 * 유닛의 고유 식별자와 무한 성장으로 쌓아온 결과물을 기록합니다.
 */
USTRUCT(BlueprintType)
struct FST_UnitSaveData
{
    GENERATED_BODY()

    // 1. 기초 고용 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName UnitRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HandR; // 오른손 무기

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HandL; // 왼손 무기

    // 2. 실시간 성장 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double CurrentExp = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentJob = TEXT("Novice");

    /** 무한 성장의 핵심: 누적된 순수 속성치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FST_Attributes AccumulatedAttributes;
};

/** * 전체 게임 세이브 데이터 구조
 * 보유한 인재 리스트와 자원, 진행도 등을 담습니다.
 */
USTRUCT(BlueprintType)
struct FST_SaveGameData
{
    GENERATED_BODY()

    /** 플레이어가 보유한 유닛(인재) 목록 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FST_UnitSaveData> RecruitedUnits;

    /** 보유 자원 (금광, 식량 등) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Gold = 0;

    /** 현재 진행 중인 스테이지 또는 시나리오 정보 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentStageRowName;
};