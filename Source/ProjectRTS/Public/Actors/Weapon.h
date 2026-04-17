// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/WeaponInterface.h"	// 인터페이스 포함
#include "Global/ProjectRTSTypes.h"		// 무기 구조체 포함
#include "Weapon.generated.h"

UCLASS()
class PROJECTRTS_API AWeapon : public AActor, public IWeaponInterface
{
	GENERATED_BODY()

public:
	AWeapon();

protected:
	virtual void BeginPlay() override;

	/** 에디터 상에서의 시각적 동기화를 위한 Construction Script */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** 무기 메시 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	class UStaticMeshComponent* WeaponMesh;

	/** 무기 데이터 테이블 (데이터 허브인 EquipComponent에서 전달받음) */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Data")
	class UDataTable* WeaponTable;

	/** 무기 식별 이름 */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Data")
	FName WeaponRowName;

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void InitializeWeapon(UDataTable* InTable, FName InRowName);

	// --- 무기 공격 Hit 판정 처리 ---
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
	void ProcessWeaponHit();

	UFUNCTION(BlueprintPure, Category = "Weapon|State")
	bool IsCharRiding() const;

	// --- IWeaponInterface 구현 ---
	virtual FST_Weapon GetWeaponData_Implementation() const override;

	virtual void SetWeaponVisibility_Implementation(bool bVisible) override;

	virtual AActor* GetWeaponOwner_Implementation() const override { return GetOwner(); }
};