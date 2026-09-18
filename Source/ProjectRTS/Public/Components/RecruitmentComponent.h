// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/ProjectRTSTypes.h"
#include "Global/RTSSaveTypes.h" // 통합 구조체 사용
#include "RecruitmentComponent.generated.h"

// FOnUnitRecruited 델리게이트도 통합 구조체를 전달하도록 변경
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitRecruited, const FST_UnitSaveRecord&, RecruitedUnit);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API URecruitmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URecruitmentComponent();

    /** * 새로운 유닛을 고용 리스트에 추가 (초기 데이터 생성) */
    UFUNCTION(BlueprintCallable, Category = "RTS|Recruitment")
    void AddUnitToRoster(FName UnitRowName, FName HandR, FName HandL);

    /** 세이브 데이터에서 직접 유닛을 복원해 목록에 추가합니다. */
    void AddUnitToRosterFromSave(const FST_UnitSaveRecord& SaveData);

    /** * 특정 클래스에 해당하는 고용 유닛들을 스폰 */
    UFUNCTION(BlueprintCallable, Category = "RTS|Recruitment")
    void SpawnRecruitedUnits(FName StartTag, float SpawnRadius = 500.0f);

    UFUNCTION(BlueprintPure, Category = "RTS|Recruitment")
    const TArray<FST_UnitSaveRecord>& GetSavedUnitRoster() const { return SavedUnitRoster; }

    UFUNCTION(BlueprintCallable, Category = "RTS|Recruitment")
    void ClearRoster() { SavedUnitRoster.Empty(); }

    /** * 세이브 시스템으로부터 전체 리스트를 한 번에 주입받을 때 사용 */
    void SetSavedUnitRoster(const TArray<FST_UnitSaveRecord>& NewList) { SavedUnitRoster = NewList; }

public:
    UPROPERTY(BlueprintAssignable, Category = "RTS|Recruitment|Events")
    FOnUnitRecruited OnUnitRecruited;

    UFUNCTION(BlueprintPure, Category = "RTS|Recruitment")
    const TMap<FGuid, FST_RuntimeUnitInfo>& GetRuntimeUnitsById() const { return RuntimeUnitsById; }

    /** UniqueId로 RuntimeUnit 정보를 조회합니다. */
    UFUNCTION(BlueprintCallable, Category = "RTS|Recruitment")
    bool TryGetActiveUnit(const FGuid& Id, FST_RuntimeUnitInfo& OutInfo) const;

private:
    /** 고용된 인재들의 명단 (세이브용 권한 데이터) */
    UPROPERTY(VisibleAnywhere, Category = "RTS|Recruitment")
    TArray<FST_UnitSaveRecord> SavedUnitRoster;

    /** 런타임에서 관리할 RuntimeUnit 정보 맵 (UniqueId -> RuntimeInfo) */
    UPROPERTY(VisibleAnywhere, Category = "RTS|Recruitment")
    TMap<FGuid, FST_RuntimeUnitInfo> RuntimeUnitsById;

};