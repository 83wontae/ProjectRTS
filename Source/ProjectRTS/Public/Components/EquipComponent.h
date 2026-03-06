// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/ProjectRTSTypes.h"
#include "EquipComponent.generated.h"

// 장비 변경 알림을 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateWeapon, const FST_Weapon&, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateArmor, EEquipType, Type, const FST_Armor&, NewArmor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTRTS_API UEquipComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** 무기 장착 함수 [cite: 1089-1090] */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipWeapon(const FST_Weapon& NewWeapon);

	/** 방어구 장착 함수 [cite: 1120-1121] */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipArmor(const FST_Armor& NewArmor);

	/** 유닛 데이터로부터 전체 장비 초기화 [cite: 1244-1245] */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipToUnitData(const FST_Unit& UnitData);

	// --- OnRep 함수 --- 
	UFUNCTION() void OnRep_Weapon();
	UFUNCTION() void OnRep_ArmorHead();
	UFUNCTION() void OnRep_ArmorBody();
	UFUNCTION() void OnRep_ArmorHorse();

public:
	// --- 장비 변수 (네트워크 복제) --- 
	UPROPERTY(ReplicatedUsing = OnRep_Weapon, BlueprintReadWrite, Category = "RTS|Equip")
	FST_Weapon m_Weapon;

	UPROPERTY(ReplicatedUsing = OnRep_ArmorHead, BlueprintReadWrite, Category = "RTS|Equip")
	FST_Armor m_Armor_Head;

	UPROPERTY(ReplicatedUsing = OnRep_ArmorBody, BlueprintReadWrite, Category = "RTS|Equip")
	FST_Armor m_Armor_Body;

	UPROPERTY(ReplicatedUsing = OnRep_ArmorHorse, BlueprintReadWrite, Category = "RTS|Equip")
	FST_Armor m_Armor_Horse;

	// --- 이벤트 ---
	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateWeapon OnUpdateWeapon;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateArmor OnUpdateArmor;

private:
	/** 장착된 장비에 맞춰 메시 업데이트 */
	void UpdateMesh();

	UPROPERTY()
	class ACharacter* OwnerChar;
};
