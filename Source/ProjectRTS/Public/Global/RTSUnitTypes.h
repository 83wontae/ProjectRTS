// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Global/RTSCoreTypes.h" // EUnitType 등을 사용하기 위해 포함
#include "RTSUnitTypes.generated.h"

/** * 유닛의 순수 속성 (힘, 민, 지, 체)
 * 모든 성장 연산의 기초가 되며, 연산자 오버로딩을 통해 합산이 가능합니다.
 */
USTRUCT(BlueprintType)
struct FST_Attributes : public FTableRowBase
{
    GENERATED_BODY()

    FST_Attributes()
        : Strength(0.0), Agility(0.0), Intelligence(0.0), Stamina(0.0) {}

    FST_Attributes(double InStr, double InAgi, double InInt, double InSta)
        : Strength(InStr), Agility(InAgi), Intelligence(InInt), Stamina(InSta) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Strength = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Agility = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Intelligence = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Stamina = 0.0;

    // 속성 합산을 위한 + 연산자 오버로딩
    FST_Attributes operator+(const FST_Attributes& Other) const
    {
        return FST_Attributes(Strength + Other.Strength, Agility + Other.Agility,
            Intelligence + Other.Intelligence, Stamina + Other.Stamina);
    }
};

/** * 유닛의 최종 전투 능력치
 * 속성(Attributes)을 기반으로 계산된 실질적인 전투 수치입니다.
 */
USTRUCT(BlueprintType)
struct FST_CombatStats : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Attack = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double MaxHp = 0.0;
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

/** * 직업 정보 데이터 (DT_Job용)
 * 초기 스탯과 레벨업 시 상승하는 속성 증가치를 결정합니다.
 */
USTRUCT(BlueprintType)
struct FST_UnitJob : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Info")
    FText JobDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Job Info")
    class UTexture2D* JobIcon;

    /** 레벨 1일 때 부여받는 기본 능력치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Stats")
    FST_Attributes BaseAttributes;

    /** 레벨업 시마다 누적되어 더해질 속성 증가치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth Stats")
    FST_Attributes GrowthAttributes;

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

    FST_Unit() : Name(TEXT("New Unit")), UnitJobRowName("Novice"), AttackRange(200.0), DetectionRange(1500.0f) {}

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Combat")
    double AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Combat")
    float DetectionRange;
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
    UPROPERTY(EditAnywhere) FST_Attributes Attributes; // 누적된 총 속성치
    UPROPERTY(VisibleAnywhere) FST_CombatStats FinalStats; // 계산된 최종 전투력
};