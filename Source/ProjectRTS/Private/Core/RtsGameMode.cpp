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
        if (GameInstance->GetJobDefinitionById(TEXT("Novice")))
        {
            UE_LOG(LogTemp, Log, TEXT("ARtsGameMode: Job definition assets are available."));
        }
    }
    else
    {
        // 로그 메시지 수정: Settings가 아니라 Instance 로드 실패임
        UE_LOG(LogTemp, Warning, TEXT("ARtsGameMode: Failed to retrieve URtsGameInstance."));
    }
}