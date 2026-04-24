// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSCoreTypes.generated.h"

/** * RTS 입력 상태 정의
 * 유닛을 조종하는 모드와 건물을 짓는 모드를 구분합니다.
 */
UENUM(BlueprintType)
enum class ERtsInputMode : uint8
{
    Tactical    UMETA(DisplayName = "Tactical Mode"),  // 유닛 지휘 모드
    Placement   UMETA(DisplayName = "Placement Mode")  // 건물 배치 모드
};

/** * 유닛의 대략적인 분류
 * 유닛의 상성이나 이동 로직을 결정할 때 사용합니다.
 */
UENUM(BlueprintType)
enum class EUnitType : uint8
{
    None        UMETA(DisplayName = "None"),
    Infantry    UMETA(DisplayName = "Infantry"),
    Cavalry     UMETA(DisplayName = "Cavalry")
};

/** * 전투 애니메이션 스타일 유형
 * 어떤 무기 세트를 사용하는지에 따라 애니메이션 블루프린트에서 참조합니다.
 */
UENUM(BlueprintType)
enum class EBattleAnimType : uint8
{
    None            UMETA(DisplayName = "None"),
    OneHanded       UMETA(DisplayName = "OneHanded"),
    OneHandShield   UMETA(DisplayName = "OneHandShield"),
    Bow             UMETA(DisplayName = "Bow"),
    Spear           UMETA(DisplayName = "Spear"),
    TwoHanded       UMETA(DisplayName = "TwoHanded")
};

/** * 장비 장착 부위 유형
 * 캐릭터의 메시 교체 및 인벤토리 슬롯 구분용으로 쓰입니다.
 */
UENUM(BlueprintType)
enum class EEquipType : uint8
{
    None    UMETA(DisplayName = "None"),
    Head    UMETA(DisplayName = "Head"),
    Body    UMETA(DisplayName = "Body"),
    Horse   UMETA(DisplayName = "Horse")
};