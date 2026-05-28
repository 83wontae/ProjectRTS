// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RtsGameMode.h"
#include "Global/RtsGameSettings.h" // 전역 세팅 클래스 인클루드

void ARtsGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 1. CDO(Class Default Object) 기반으로 전역 개발자 세팅 객체를 안전하게 가져옵니다.
    if (const URtsGameSettings* GameSettings = GetDefault<URtsGameSettings>())
    {
        // 2. 세팅 내부에 구현된 Get 함수들을 여기서 '최초 1회 호출'해 줍니다.
        // 이 함수 내부에서 SoftObjectPath.TryLoad()가 실행되며 하드디스크의 에셋이 RAM(메모리)에 올라가고 캐싱됩니다.

        if (GameSettings->GetUnitDataTable())
        {
            UE_LOG(LogTemp, Log, TEXT("ARtsGameMode: Unit Data Table successfully pre-loaded into memory."));
        }

        // 만약 다른 테이블(무기, 직업 등)도 URtsGameSettings에 추가되어 있다면 아래처럼 한 줄씩 호출해 주면 됩니다.
        // GameSettings->GetWeaponDataTable();
        // GameSettings->GetJobDataTable();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ARtsGameMode: Failed to retrieve URtsGameSettings."));
    }
}