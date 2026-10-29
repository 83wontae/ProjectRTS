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
// Updated delegates include the owner's UniqueId so listeners can identify the source
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUpdateWeaponWithId, const FGuid&, UniqueId, EWeaponSlot, Slot, const FST_Weapon&, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUpdateArmorWithId, const FGuid&, UniqueId, EEquipType, Type, const FST_Armor&, NewArmor);
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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	void RefreshWeaponsInEditor();

public:
	/** RowName을 사용하여 무기를 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipWeaponByName(FName WeaponName, EWeaponSlot Slot);

	/** 인스턴스 정보를 사용하여 무기를 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipItemInstance(const FST_ItemInstance& ItemInstance, EUnitEquipSlot SlotType);

	/** RowName을 사용하여 방어구를 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipArmorByName(FName ArmorName, EEquipType Type);

	/** 유닛 데이터(구조체)를 받아 초기 외형을 설정합니다. (내부적으로 Name 방식으로 전환 가능) */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipToUnitData(FName InUnitRowName);

	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void UpdateTotalEquipStats();

	UFUNCTION(BlueprintPure, Category = "RTS|Equip")
	bool IsRideState() const;

	/** 장착 로직 내부 처리 함수 */
	void HandleWeaponAttachment(FName WeaponName, EWeaponSlot RequestedSlot);

	/** * 무기 이름만으로 적절한 슬롯을 찾아 장착합니다.
	 * 제약 조건(ForceLeft, ForceRight 등)에 따라 자동으로 손을 결정합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipToWeapon(FName WeaponName);

	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void EquipToWeaponInstance(const FST_ItemInstance& ItemInstance);

	UFUNCTION(BlueprintPure, Category = "RTS|Equip")
	bool IsEquipSlotEmpty(EUnitEquipSlot SlotType) const;

	UFUNCTION(BlueprintPure, Category = "RTS|Equip")
	FName GetEquipSlotRowName(EUnitEquipSlot SlotType) const;

	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void ClearEquipSlot(EUnitEquipSlot SlotType);

	UFUNCTION(BlueprintCallable, Category = "RTS|Equip")
	void ClearAllEquipSlots();

	UFUNCTION(BlueprintPure, Category = "RTS|Equip")
	float GetAttackRange() const;

	UFUNCTION(BlueprintPure, Category = "RTS|Equip")
	float GetDetectionRange() const;

	// --- OnRep 함수 ---
	UFUNCTION() void OnRep_UnitRowName();
	UFUNCTION() void OnRep_EquipSlots();
	UFUNCTION() void OnRep_RightWeaponName();
	UFUNCTION() void OnRep_LeftWeaponName();
	UFUNCTION() void OnRep_ArmorHeadName();
	UFUNCTION() void OnRep_ArmorBodyName();
	UFUNCTION() void OnRep_ArmorHorseName();
	UFUNCTION() void OnRep_BattleAnimType();

	UFUNCTION(BlueprintCallable, Category = "RTS|Combat")
	void UpdateBattleAnimType();

public:
	/** 특정 슬롯에 장착된 무기의 머즐 소켓 이름을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "RTS|Equip")
	FVector GetMainMuzzleLocation() const;

	UPROPERTY(ReplicatedUsing = OnRep_UnitRowName, BlueprintReadWrite, Category = "RTS|Equip")
	FName m_UnitRowName;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_EquipSlots, BlueprintReadWrite, Category = "RTS|Equip")
	TArray<FST_ItemSlot> m_EquipSlots;

	UPROPERTY()
	TArray<FST_ItemInstance> m_EquippedItemInstances;

	UPROPERTY(ReplicatedUsing = OnRep_BattleAnimType, BlueprintReadWrite, Category = "RTS|Combat")
	EBattleAnimType m_BattleAnimType;

	// --- 이벤트 ---
	// Legacy two-parameter delegates (still broadcast for compatibility)
	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateWeapon OnUpdateWeapon;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateArmor OnUpdateArmor;

	// New UniqueId-aware delegates
	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateWeaponWithId OnUpdateWeaponWithId;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateArmorWithId OnUpdateArmorWithId;

	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateRideState OnUpdateRideState;

	/** 애니메이션 타입 변경 시 발생하는 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateBattleAnimType OnUpdateBattleAnimType;

	/** 유닛 본체 업데이트 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "RTS|Equip|Events")
	FOnUpdateUnitBody OnUpdateUnitBody;

private:
	EUnitEquipSlot GetSlotTypeFromWeaponSlot(EWeaponSlot Slot) const;
	EUnitEquipSlot GetSlotTypeFromEquipType(EEquipType Type) const;
	void ApplySlotItem(EUnitEquipSlot SlotType, const FST_ItemInstance& ItemInstance);
	FName ResolveRowNameFromSlot(EUnitEquipSlot SlotType) const;
	bool ValidateItemInstanceForSlot(const FST_ItemInstance& ItemInstance, EUnitEquipSlot SlotType) const;
	void NotifySlotUpdated(EUnitEquipSlot SlotType);

	UPROPERTY()
	class ACharacter* OwnerChar;

	UPROPERTY(Transient)
	TArray<FST_ItemSlot> m_LastReplicatedEquipSlots;

	// UniqueId of the owning unit (set by RecruitmentComponent when actor registered)
	UPROPERTY()
	FGuid m_OwnerUniqueId;

public:
	// Set the owner UniqueId so EquipComponent can include it in broadcasts
	void SetOwnerUniqueId(const FGuid& InId) { m_OwnerUniqueId = InId; }

	int32 GetEquipSlotIndex(EUnitEquipSlot SlotType) const { return static_cast<int32>(SlotType); }
	FST_ItemSlot& GetEquipSlot(EUnitEquipSlot SlotType);
	const FST_ItemSlot& GetEquipSlot(EUnitEquipSlot SlotType) const;
	bool TryGetEquippedItemInstance(const FGuid& InstanceId, FST_ItemInstance& OutInstance) const;

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
