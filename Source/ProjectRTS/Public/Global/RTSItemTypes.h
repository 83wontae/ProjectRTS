// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Global/RTSCoreTypes.h" // 기초 타입 활용을 위해 포함
#include "RTSItemTypes.generated.h"

/** * 무기 유형
 * 어떤 종류의 무기인지 구분하며, 애니메이션 결정 시 참조될 수 있습니다.
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None    UMETA(DisplayName = "None"),
    Sword   UMETA(DisplayName = "Sword"),
    Bow     UMETA(DisplayName = "Bow"),
    Shield  UMETA(DisplayName = "Shield")
};

/** * 무기 장착 손 제약
 * 무기가 왼손용인지 오른손용인지, 혹은 양손 모두 가능한지를 정의합니다.
 */
UENUM(BlueprintType)
enum class EWeaponHandConstraint : uint8
{
    None,           // 어느 손이든 상관없음 (검, 단검 등)
    ForceLeft,      // 무조건 왼손 (방패, 활)
    ForceRight,     // 무조건 오른손
    Shield          // 방패 전용 슬롯
};

/** * 무기 데이터 (DataTable용)
 * 스태틱 메시와 공격력, 사거리 등 실질적인 전투 수치를 포함합니다.
 */
USTRUCT(BlueprintType)
struct FST_Weapon : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EWeaponType WeaponType;

    /** 이 무기를 장착했을 때 추가될 기본 스킬 이름 (미탑승/탑승 상태 구분) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName SkillNameRide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EWeaponHandConstraint HandConstraint;

    /** 발사체가 생성될 무기의 소켓 이름 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName MuzzleSocketName = TEXT("MuzzleSocket");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    class UStaticMesh* StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    double AttackPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    double AttackRange;
};

/** * 방어구 데이터 (DataTable용)
 * 스켈레탈 메시 부품(머리, 몸통 등)과 아이콘 정보를 관리합니다.
 */
USTRUCT(BlueprintType)
struct FST_Armor : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    EEquipType EquipType; // RTSCoreTypes.h에 정의됨

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    EUnitType UnitType;   // RTSCoreTypes.h에 정의됨

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    class USkeletalMesh* SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    class UTexture2D* Icon;
};