// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RTSSkillTypes.generated.h"

/** * 스킬의 구체적인 유형
 * 스킬이 어떤 방식으로 작동하는지 구분합니다.
 */
UENUM(BlueprintType)
enum class ESkillType : uint8
{
    Melee       UMETA(DisplayName = "Melee"),      // 근접 타격
    Projectile  UMETA(DisplayName = "Projectile"), // 발사체 발사
    Buff        UMETA(DisplayName = "Buff"),       // 자기 자신 또는 아군 버프
    Area        UMETA(DisplayName = "Area")        // 범위 공격
};

/** * 유닛 활성 애니메이션 데이터
 * 공격, 피격, 사망 등 기본적인 액션에 필요한 몽타주 세트입니다.
 */
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

/** * 스킬 상세 데이터 (DataTable용)
 * 스킬의 성능, 소모 값, 연출용 에셋 정보를 모두 포함합니다.
 */
USTRUCT(BlueprintType)
struct FST_Skill : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    ESkillType SkillType;

    /** 발사체 스킬일 경우 소환할 액터 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    TSubclassOf<AActor> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    class UTexture2D* Icon;

    /** 스킬 사용 시 재생할 전용 애니메이션 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    class UAnimMontage* SkillAnim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float ManaCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float Range;
};