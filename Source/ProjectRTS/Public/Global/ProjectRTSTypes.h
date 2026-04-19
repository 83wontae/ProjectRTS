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
    OneHanded       UMETA(DisplayName = "OneHanded"),
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

    /** 이 무기를 장착했을 때 추가될 기본 스킬 이름(미탑승 상태일 경우) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName SkillName;

    /** 이 무기를 장착했을 때 추가될 기본 스킬 이름(탑승 상태일 경우) */
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

/** 유닛 기초 데이터 ＊*/
USTRUCT(BlueprintType)
struct FST_Unit : public FTableRowBase
{
    GENERATED_BODY()

    // 생성자를 통한 기본값 설정
    FST_Unit()
        : Name(TEXT("New Unit"))
        , UnitClass(nullptr)
        , UnitType(EUnitType::None) // 프로젝트에 정의된 기본값 사용
        , HeadPart(nullptr)
        , BodyPart(nullptr)
        , AttackRange(250.0f)      // 근접 기본 사거리
        , Attack(10.0)
        , Defend(0.0)
        , MaxHp(100.0)
        , Speed(500.0)            // 기본 이동 속도
        , DetectionRange(2000.0f)  // 기본 인지 범위
    {
    }

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
    float DetectionRange;
};

/** 유닛의 종합 스탯 구조체 */
USTRUCT(BlueprintType)
struct FST_UnitStats
{
    GENERATED_BODY()

    // --- [레벨 시스템] ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
    int32 Level = 1;

    // --- [기본 주요 스탯 (Primary Attributes)] ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primary")
    double Strength = 0.0;      // 힘: 물리 공격력, 최대 체력에 영향

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primary")
    double Agility = 0.0;       // 민첩: 공격 속도, 이동 속도, 방어력에 영향

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primary")
    double Intelligence = 0.0;  // 지능: 스킬 데미지, 마나량에 영향

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primary")
    double Stamina = 0.0;       // 스테미너: 최대 체력, 행동력에 영향

    // --- [파생 전투 스탯 (Derived Stats)] ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Derived")
    double Attack = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Derived")
    double MaxHp = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Derived")
    double Defend = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Derived")
    double Speed = 0.0;

    // 더하기 연산자 오버로딩 (아이템 장착 시나 레벨업 보너스 합산용)
    FST_UnitStats operator+(const FST_UnitStats& Other) const
    {
        FST_UnitStats Result;
        Result.Level = Level + Other.Level;
        Result.Strength = Strength + Other.Strength;
        Result.Agility = Agility + Other.Agility;
        Result.Intelligence = Intelligence + Other.Intelligence;
        Result.Stamina = Stamina + Other.Stamina;

        Result.Attack = Attack + Other.Attack;
        Result.MaxHp = MaxHp + Other.MaxHp;
        Result.Defend = Defend + Other.Defend;
        Result.Speed = Speed + Other.Speed;
        return Result;
    }
};

/** 유닛의 직업 종류 */
UENUM(BlueprintType)
enum class EUnitJob : uint8
{
    None		UMETA(DisplayName = "None"),        // 시스템 에러 체크용
	Novice		UMETA(DisplayName = "Novice"),      // 견습생 (초기 직업)
    Warrior		UMETA(DisplayName = "Warrior"),     // 전사
	Mage		UMETA(DisplayName = "Mage"),        // 마법사
	Rogue		UMETA(DisplayName = "Rogue"),       // 도적
	Priest		UMETA(DisplayName = "Priest"),      // 성직자
    MAX
};

/** 직업별 레벨업 성장 수치 데이터 */
USTRUCT(BlueprintType)
struct FST_ClassGrowth : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double PlusStr = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double PlusAgi = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double PlusInt = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double PlusSta = 0.0;
};

/** 스킬의 구체적인 유형 */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
    Melee,      // 근접 타격
    Projectile, // 발사체 발사
    Buff,       // 자기 자신 또는 아군 버프
    Area        // 범위 공격
};

/** 스킬 데이터 구조체 (DataTable용) */
USTRUCT(BlueprintType)
struct FST_Skill : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    ESkillType SkillType;

    /** 발사체 스킬일 경우 사용할 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    TSubclassOf<AActor> ProjectileClass;

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

USTRUCT(BlueprintType)
struct FST_Building : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TSubclassOf<class ABuilding> BuildingClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    class UStaticMesh* BuildingMesh; // 건설 전 보여줄 고스트 메쉬

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 GridSizeX = 1; // 1x1, 2x2 등

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 GridSizeY = 1;
};

// RTS 입력 상태 정의
UENUM(BlueprintType)
enum class ERtsInputMode : uint8
{
    Tactical,    // 유닛 지휘 모드
    Placement    // 건물 배치 모드
};