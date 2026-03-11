// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/ProjectRTSTypes.h"
#include "EquipComponent.generated.h"

/** 무기 장착 슬롯 구분 */
UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	LeftHand	UMETA(DisplayName = "Left Hand"),
	RightHand	UMETA(DisplayName = "Right Hand")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateWeapon, EWeaponSlot, Slot, const FST_Weapon&, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateArmor, EEquipType, Type, const FST_Armor&, NewArmor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateBattleAnimType, EBattleAnimType, NewAnimType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateRideState, bool, bIsRiding);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateUnitBody, const FST_Unit&, UnitData);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API UEquipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	void RefreshWeaponsInEditor();

public:
	/** RowName을 사용하여 무기를 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipWeaponByName(FName WeaponName, EWeaponSlot Slot);

	/** RowName을 사용하여 방어구를 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipArmorByName(FName ArmorName, EEquipType Type);

	/** 유닛 데이터(구조체)를 받아 초기 외형을 설정합니다. (내부적으로 Name 방식으로 전환 가능) */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipToUnitData(FName InUnitRowName);

	UFUNCTION(BlueprintPure, Category = "RTS|Equip")
	bool IsRideState() const { return !m_ArmorHorseName.IsNone(); }

	/** 데이터 테이블 조회가 필요한 경우를 위해 데이터 반환 함수 제공 */
	const FST_Unit* GetUnitData(FName InUnitRowName) const;

	/** 장착 로직 내부 처리 함수 */
	void HandleWeaponAttachment(FName WeaponName, EWeaponSlot RequestedSlot);

	// --- OnRep 함수 ---
	UFUNCTION() void OnRep_UnitRowName();
	UFUNCTION() void OnRep_RightWeaponName();
	UFUNCTION() void OnRep_LeftWeaponName();
	UFUNCTION() void OnRep_ArmorHeadName();
	UFUNCTION() void OnRep_ArmorBodyName();
	UFUNCTION() void OnRep_ArmorHorseName();
	UFUNCTION() void OnRep_BattleAnimType();

	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	void UpdateBattleAnimType();

public:
	UPROPERTY(ReplicatedUsing = OnRep_UnitRowName, BlueprintReadWrite, Category = "RTS|Equip")
	FName m_UnitRowName;

	// --- 장비 RowName (네트워크 복제 최적화) --- 
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_RightWeaponName, BlueprintReadWrite, Category = "RTS|Equip")
	FName m_RightWeaponName;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_LeftWeaponName, BlueprintReadWrite, Category = "RTS|Equip")
	FName m_LeftWeaponName;

	UPROPERTY(ReplicatedUsing = OnRep_ArmorHeadName, BlueprintReadWrite, Category = "RTS|Equip")
	FName m_ArmorHeadName;

	UPROPERTY(ReplicatedUsing = OnRep_ArmorBodyName, BlueprintReadWrite, Category = "RTS|Equip")
	FName m_ArmorBodyName;

	UPROPERTY(ReplicatedUsing = OnRep_ArmorHorseName, BlueprintReadWrite, Category = "RTS|Equip")
	FName m_ArmorHorseName;

	// --- 데이터 테이블 포인터 (에디터에서 할당) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
	class UDataTable* UnitTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
	class UDataTable* WeaponTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
	class UDataTable* ArmorTable;

	UPROPERTY(ReplicatedUsing = OnRep_BattleAnimType, BlueprintReadWrite, Category = "RTS|Combat")
	EBattleAnimType m_BattleAnimType;

	// --- 이벤트 ---
	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateWeapon OnUpdateWeapon;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateArmor OnUpdateArmor;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateRideState OnUpdateRideState;

	/** 애니메이션 타입 변경 시 발생하는 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateBattleAnimType OnUpdateBattleAnimType;

	/** 유닛 본체 업데이트 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateUnitBody OnUpdateUnitBody;

private:
	UPROPERTY()
	class ACharacter* OwnerChar;

protected:
	/** 스폰할 무기 베이스 클래스 (BP_Weapon 등을 할당) */
	UPROPERTY(EditAnywhere, Category = "RTS|Equip|Settings")
	TSubclassOf<class AWeapon> WeaponClass;

	/** 소켓 이름 결정 보조 함수 */
	FName GetTargetSocketName(const FST_Weapon& WeaponData, EWeaponSlot RequestedSlot) const;

	// 스폰된 무기 액터 관리 변수
	UPROPERTY() class AWeapon* WeaponActor_R;
	UPROPERTY() class AWeapon* WeaponActor_L;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Equip|Settings")
	FName RightHandSocketName = TEXT("Weapon_Socket_R");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Equip|Settings")
	FName LeftHandSocketName = TEXT("Weapon_Socket_L");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Equip|Settings")
	FName ShieldSocketName = TEXT("Shield_Socket");
};
