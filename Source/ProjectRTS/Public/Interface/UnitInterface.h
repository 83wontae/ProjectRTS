// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UnitInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UUnitInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * BPI_Unit 인터페이스의 C++ 구현체
 */
class PROJECTRTS_API IUnitInterface
{
    GENERATED_BODY()

public:
    /** 유닛 초기화 (행 이름 파라미터 포함)  */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit")
    bool InitUnit(FName UnitRowName, FName L_WeaponRowName, FName R_WeaponRowName);

    /** * 유닛 클릭 이벤트
     * 파라미터: Mouse 위치 정보 등을 받을 수 있도록 설계됨
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit")
    bool EventUnitClicked(FVector MouseLocation);

    /** 유닛 선택 상태 변경 */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit")
    bool EventUnitSelected(bool IsSelected);

    /** 팀 인덱스를 즉시 반환하는 함수 (성능 최적화용) */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit")
    int32 GetFaction() const;
};