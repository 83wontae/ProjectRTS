// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/ProjectRTSTypes.h"
#include "RecruitmentComponent.generated.h"

/** * 고용된 유닛의 핵심 정보를 담는 구조체 */
USTRUCT(BlueprintType)
struct FST_RecruitUnit
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName UnitRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HandR; // 오른손 무기 RowName

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName HandL; // 왼손 무기 RowName
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API URecruitmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URecruitmentComponent();

protected:
    virtual void BeginPlay() override;

public:
    /** * 새로운 유닛을 고용 리스트에 추가합니다.
     * @param UnitRowName 유닛 종류 (DT_Unit)
     * @param HandR 오른손 무기 (DT_Weapon)
     * @param HandL 왼손 무기 (DT_Weapon)
     */
    UFUNCTION(BlueprintCallable, Category = "RTS|Recruitment")
    void AddUnitToRoster(FName UnitRowName, FName HandR, FName HandL);

    /** * 현재 고용된 모든 유닛 리스트를 반환합니다. */
    UFUNCTION(BlueprintPure, Category = "RTS|Recruitment")
    const TArray<FST_RecruitUnit>& GetRecruitedUnits() const { return RecruitedUnits; }

    /** * 리스트를 비웁니다. (배틀 시작 후 혹은 초기화 시 사용) */
    UFUNCTION(BlueprintCallable, Category = "RTS|Recruitment")
    void ClearRoster() { RecruitedUnits.Empty(); }

public:
    /** * 고용된 모든 유닛을 특정 태그를 가진 PlayerStart 주변에 스폰합니다.
     * @param StartTag 찾을 PlayerStart의 태그
     * @param SpawnRadius 스폰 범위 (원형 영역 반지름)
     */
    UFUNCTION(BlueprintCallable, Category = "RTS|Recruitment")
    void SpawnRecruitedUnits(FName StartTag, float SpawnRadius = 500.0f);

protected:
    /** 유닛 클래스 정보를 가져오기 위한 데이터 테이블 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
    class UDataTable* UnitDataTable;

private:
    /** 고용된 인재들의 명단 (배틀 레벨로 넘겨줄 데이터) */
    UPROPERTY(VisibleAnywhere, Category = "RTS|Recruitment")
    TArray<FST_RecruitUnit> RecruitedUnits;
};