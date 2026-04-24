// Fill out your copyright notice in the Description page of Project Settings.

#include "Save/RtsSaveGame.h"

URtsSaveGame::URtsSaveGame()
{
    // 기본 슬롯 이름과 인덱스를 설정합니다.
    SaveSlotName = TEXT("DefaultSaveSlot");
    SaveIndex = 0;

    // SaveData 내부의 변수들은 구조체 정의 시의 기본값으로 초기화됩니다.
}