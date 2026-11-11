// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Global/RTSCoreTypes.h" // EUnitType 등을 사용하기 위해 포함
#include "RTSUnitTypes.generated.h"

/** * 유닛의 순수 속성 (STR, DEX, INT)
 * 모든 성장 연산의 기초가 되며, 연산자 오버로딩을 통해 합산이 가능합니다.
 */
USTRUCT(BlueprintType)
struct FST_Attributes : public FTableRowBase
{
    GENERATED_BODY()

    FST_Attributes()
        : Strength(1.0), Dexterity(1.0), Intelligence(1.0) {}

    FST_Attributes(double InStr, double InDex, double InInt)
        : Strength(InStr), Dexterity(InDex), Intelligence(InInt) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Strength = 1.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Dexterity = 1.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Intelligence = 1.0;

    // 속성 합산을 위한 + 연산자 오버로딩
    FST_Attributes operator+(const FST_Attributes& Other) const
    {
        return FST_Attributes(Strength + Other.Strength, Dexterity + Other.Dexterity,
            Intelligence + Other.Intelligence);
    }
};

USTRUCT(BlueprintType)
struct FST_JobProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Proficiency")
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Proficiency")
    double CurrentExp = 0.0;
};

/** * 유닛의 최종 전투 능력치
 * 속성(Attributes)을 기반으로 계산된 실질적인 전투 수치입니다.
 */
USTRUCT(BlueprintType)
struct FST_CombatStats : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Attack = 1.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double MaxHp = 100.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Defend = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Speed = 0.0;

    FST_CombatStats operator+(const FST_CombatStats& Other) const
    {
        FST_CombatStats Result;
        Result.Attack = Attack + Other.Attack;
        Result.MaxHp = MaxHp + Other.MaxHp;
        Result.Defend = Defend + Other.Defend;
        Result.Speed = Speed + Other.Speed;
        return Result;
    }
};

/** * 직업 정보 데이터 (레거시 호환용)
 * 직업 숙련도 기준값과 시작 스킬 정보를 정의합니다.
 */
USTRUCT(BlueprintType)
struct FST_UnitJob : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Info")
    FText JobDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Info")
    class UTexture2D* JobIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Proficiency")
    double ProficiencyExpPerLevel = 100.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Proficiency")
    int32 MaxProficiencyLevel = 999;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    TArray<FName> StartingSkills;
};

/** * 유닛 기초 데이터 (DT_Unit용)
 * 유닛의 외형과 어떤 직업으로 시작할지를 정의합니다.
 */
USTRUCT(BlueprintType)
struct FST_Unit : public FTableRowBase
{
    GENERATED_BODY()

    FST_Unit() : Name(TEXT("New Unit")), UnitJobRowName("Novice") {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    TSubclassOf<AActor> UnitClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    FName UnitJobRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Parts")
    class USkeletalMesh* HeadPart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Parts")
    class USkeletalMesh* BodyPart;
};

/** * 유닛의 실시간 성장 데이터
 * 레벨, 경험치 및 현재 누적된 속성치를 관리합니다.
 */
USTRUCT(BlueprintType)
struct FST_UnitData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere) int32 Level = 1;
    UPROPERTY(VisibleAnywhere) double CurrentExp = 0.0;
    UPROPERTY(VisibleAnywhere) FString Name = "UnitName";
    UPROPERTY(EditAnywhere) FST_Attributes Attributes; // 누적된 총 속성치
    UPROPERTY(VisibleAnywhere) FST_CombatStats FinalStats; // 계산된 최종 전투력
};

/**
 * 게임 내에 스폰되어 활동 중인 유닛의 종합 런타임 데이터입니다.
 * UI 갱신, 데미지 계산, 세이브 데이터 변환 시 핵심 포인터로 사용됩니다.
 */
USTRUCT(BlueprintType)
struct FST_RuntimeUnitInfo
{
    GENERATED_BODY()

    // 1. 고유 식별 및 참조 (UnitDefinition PrimaryAssetId 기반)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Identity")
    FPrimaryAssetId UnitDefinitionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Identity")
    FName CurrentJobRowName;

    // 2. 실시간 전투 상태 (MaxHp는 FinalStats에 있지만, '현재 체력'은 이곳에서 관리합니다)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Status")
    double CurrentHp = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Status")
    double CurrentMana = 0.0;

    // 3. 실시간 스탯 및 성장 (기존에 만드신 구조체 활용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Growth")
    FST_UnitData GrowthData;

    // 4. 현재 장착 중인 장비 목록 (RTSItemTypes의 장비 RowName 참조)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Equipment")
    FName EquipHandR;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Equipment")
    FName EquipHandL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Equipment")
    FName EquipHead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Equipment")
    FName EquipBody;

    // 5. 현재 유닛에게 적용된 버프/디버프 스킬 이름 목록 (옵션)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Buffs")
    TArray<FName> ActiveBuffs;

    // 6. 현재 유닛이 사용 가능한 액티브/패시브 스킬 목록 (능동적인 보유 스킬)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Skills")
    TArray<FName> AvailableSkills;
};