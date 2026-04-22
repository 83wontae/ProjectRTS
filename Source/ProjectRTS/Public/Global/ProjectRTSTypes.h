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
/** 유닛 기초 데이터 (DT_Unit용) */
USTRUCT(BlueprintType)
struct FST_Unit : public FTableRowBase
{
    GENERATED_BODY()

    FST_Unit()
        : Name(TEXT("New Unit")), UnitClass(nullptr), UnitJobRowName("Novice")
        , AttackRange(200.0f), DetectionRange(1500.0f)
    {}

    // --- [메타 데이터] ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    TSubclassOf<AActor> UnitClass;

    // --- [직업 설정] ---
    // 이제 수치를 직접 적지 않고, 이 Job에 설정된 BaseStats을 가져다 씁니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    FName UnitJobRowName;

    // --- [외형 데이터] ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Parts")
    class USkeletalMesh* HeadPart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Parts")
    class USkeletalMesh* BodyPart;

    // --- [AI/전투 설정] ---
    // 사거리는 '능력치'라기보다 유닛의 '특성(궁수, 전사)'에 가깝기 때문에 여기 둡니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Combat")
    double AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Combat")
    float DetectionRange;
};

USTRUCT(BlueprintType)
struct FST_Attributes : public FTableRowBase
{
    GENERATED_BODY()

    // 1. 기본 생성자 (언리얼 시스템 및 초기화를 위해 필수)
    FST_Attributes()
        : Strength(0.0), Agility(0.0), Intelligence(0.0), Stamina(0.0) {}

    // 2. 4개의 인자를 받는 생성자 추가 (에러 해결 핵심)
    FST_Attributes(double InStr, double InAgi, double InInt, double InSta)
        : Strength(InStr), Agility(InAgi), Intelligence(InInt), Stamina(InSta) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Strength = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Agility = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Intelligence = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Stamina = 0.0;

    // + 연산자 오버로딩 수정
    FST_Attributes operator+(const FST_Attributes& Other) const
    {
        FST_Attributes Result;
        Result.Strength = Strength + Other.Strength;
        Result.Agility = Agility + Other.Agility;
        Result.Intelligence = Intelligence + Other.Intelligence;
        Result.Stamina = Stamina + Other.Stamina;
        return Result;
    }
};

USTRUCT(BlueprintType)
struct FST_CombatStats : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Attack = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double MaxHp = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Defend = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Speed = 0.0;

    // --- [추가] 더하기 연산자 오버로딩 ---
    FST_CombatStats operator+(const FST_CombatStats& Other) const
    {
        FST_CombatStats Result;
        Result.Attack = Attack + Other.Attack;
        Result.Defend = Defend + Other.Defend;
        Result.MaxHp = MaxHp + Other.MaxHp;
        Result.Speed = Speed + Other.Speed;
        return Result;
    }
};

USTRUCT(BlueprintType)
struct FST_UnitData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere) int32 Level = 1;
    UPROPERTY(VisibleAnywhere) double CurrentExp = 0.0;

    // 순수 속성 (힘, 민, 지...)
    UPROPERTY(EditAnywhere) FST_Attributes Attributes;

    // 최종 전투력 (계산 결과)
    UPROPERTY(VisibleAnywhere) FST_CombatStats FinalStats;
};

/** 직업의 모든 정보를 담는 구조체 */
USTRUCT(BlueprintType)
struct FST_UnitJob : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Info")
    FText JobDisplayName; // 화면에 표시될 이름 (예: "전사", "Warrior")

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Info")
    class UTexture2D* JobIcon; // UI용 아이콘

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Info")
    FString JobDescription; // 직업 설명

    // --- [초기 스탯] ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Stats")
    FST_Attributes BaseAttributes; // 레벨 1일 때의 기본 능력치

    // --- [성장 수치] ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stats")
    FST_Attributes GrowthAttributes; // 레벨업 시 추가되는 능력치

    // --- [시작 스킬] ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    TArray<FName> StartingSkills; // 태어날 때부터 가지고 있는 스킬 목록
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