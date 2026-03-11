// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ProjectRTSTypes.generated.h"

// --- [열거형 정의] ---

/** 전투 애니메이션 스타일 유형 **/
UENUM(BlueprintType)
enum class EBattleAnimType : uint8
{
    None            UMETA(DisplayName = "None"),
    OneHandShield   UMETA(DisplayName = "OneHandShield"),
    Bow             UMETA(DisplayName = "Bow"),
    Spear           UMETA(DisplayName = "Spear"),
    TwoHanded       UMETA(DisplayName = "TwoHanded")
};

/** 장비 장착 부위 유형 **/
UENUM(BlueprintType)
enum class EEquipType : uint8
{
    None    UMETA(DisplayName = "None"),
    Head    UMETA(DisplayName = "Head"),
    Body    UMETA(DisplayName = "Body"),
    Horse   UMETA(DisplayName = "Horse")
};

/** 유닛 유형 **/
UENUM(BlueprintType)
enum class EUnitType : uint8
{
    None        UMETA(DisplayName = "None"),
    Infantry    UMETA(DisplayName = "Infantry"),
    Cavalry     UMETA(DisplayName = "Cavalry")
};

/** 무기 유형 **/
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None    UMETA(DisplayName = "None"),
    Sword   UMETA(DisplayName = "Sword"),
    Bow     UMETA(DisplayName = "Bow"),
    Shield  UMETA(DisplayName = "Shield")
};

UENUM(BlueprintType)
enum class EWeaponHandConstraint : uint8
{
    None,           // 어느 손이든 상관없음 (검, 단검 등)
    ForceLeft,      // 무조건 왼손 (방패, 활)
    ForceRight,     // 무조건 오른손
    Shield          // 방패용
};

// --- [구조체 정의] ---

/** 유닛 애니메이션 데이터 ＊*/
USTRUCT(BlueprintType)
struct FST_ActiveAnim : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* Attack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* Hit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* Death;
};

/** 방어구 데이터 ＊*/
USTRUCT(BlueprintType)
struct FST_Armor : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    EEquipType EquipType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    EUnitType UnitType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    class USkeletalMesh* SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    class UTexture2D* Icon;
};

/** 무기 데이터 ＊*/
USTRUCT(BlueprintType)
struct FST_Weapon : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EWeaponType WeaponType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EWeaponHandConstraint HandConstraint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    class UStaticMesh* StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    class UAnimMontage* AttackAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    class UAnimMontage* RideAttackAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    double AttackPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    double AttackRange;
};

/** 유닛 기초 데이터 ＊*/
USTRUCT(BlueprintType)
struct FST_Unit : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    TSubclassOf<AActor> UnitClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    EUnitType UnitType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Parts")
    class USkeletalMesh* HeadPart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Parts")
    class USkeletalMesh* BodyPart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    double AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    double Attack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    double Defend;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    double MaxHp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    double Speed;
};

/** 스킬 데이터 구조체 (DataTable용) */
USTRUCT(BlueprintType)
struct FST_Skill : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    class UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    class UAnimMontage* SkillAnim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float ManaCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float Range;
};