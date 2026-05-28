// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RtsGameMode.h"
#include "Global/RtsGameSettings.h" // 전역 세팅 클래스 인클루드
#include "Global/RtsGameInstance.h" // 게임 인스턴스 클래스 인클루드

void ARtsGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 1. const 키워드를 제거하여 상태를 변경(캐싱)할 수 있도록 합니다.
    URtsGameInstance* GameInstance = Cast<URtsGameInstance>(GetGameInstance());

    // 2. CDO(Class Default Object) 기반으로 전역 개발자 세팅 객체를 안전하게 가져옵니다.
    if (GameInstance)
    {
        // 이 함수 내부에서 LoadSynchronous()가 실행되며 하드디스크의 에셋이 RAM(메모리)에 올라가고 캐싱됩니다.
        if (GameInstance->GetUnitDataTable())
        {
            UE_LOG(LogTemp, Log, TEXT("ARtsGameMode: Unit Data Table successfully pre-loaded into memory."));
        }

        if (GameInstance->GetWeaponDataTable())
        {
            UE_LOG(LogTemp, Log, TEXT("ARtsGameMode: Weapon Data Table successfully pre-loaded into memory."));
        }

        if (GameInstance->GetJobDataTable())
        {
            UE_LOG(LogTemp, Log, TEXT("ARtsGameMode: Job Data Table successfully pre-loaded into memory."));
        }

        if (GameInstance->GetArmorDataTable())
        {
            UE_LOG(LogTemp, Log, TEXT("ARtsGameMode: Armor Data Table successfully pre-loaded into memory."));
        }
    }
    else
    {
        // 로그 메시지 수정: Settings가 아니라 Instance 로드 실패임
        UE_LOG(LogTemp, Warning, TEXT("ARtsGameMode: Failed to retrieve URtsGameInstance."));
    }
}