// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/EquipComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Actors/Weapon.h"
#include "Components/StateComponent.h"
#include "Components/SkillComponent.h"
#include "Interface/WeaponInterface.h"
#include "Global/RtsGameSettings.h"
#include "Interface/EquipableInterface.h"	

// Sets default values for this component's properties
UEquipComponent::UEquipComponent() :WeaponClass(AWeapon::StaticClass())
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	m_EquipSlots.SetNum(static_cast<int32>(EUnitEquipSlot::Max));
}

FST_ItemSlot& UEquipComponent::GetEquipSlot(EUnitEquipSlot SlotType)
{
	const int32 Index = GetEquipSlotIndex(SlotType);
	if (!m_EquipSlots.IsValidIndex(Index))
	{
		m_EquipSlots.SetNum(static_cast<int32>(EUnitEquipSlot::Max));
	}

	return m_EquipSlots[Index];
}

const FST_ItemSlot& UEquipComponent::GetEquipSlot(EUnitEquipSlot SlotType) const
{
	static const FST_ItemSlot EmptySlot;
	const int32 Index = GetEquipSlotIndex(SlotType);
	return m_EquipSlots.IsValidIndex(Index) ? m_EquipSlots[Index] : EmptySlot;
}

bool UEquipComponent::TryGetEquippedItemInstance(const FGuid& InstanceId, FST_ItemInstance& OutInstance) const
{
	if (!InstanceId.IsValid()) return false;
	for (const FST_ItemInstance& Instance : m_EquippedItemInstances)
	{
		if (Instance.InstanceId == InstanceId)
		{
			OutInstance = Instance;
			return true;
		}
	}
	return false;
}

FName UEquipComponent::ResolveRowNameFromSlot(EUnitEquipSlot SlotType) const
{
	const FST_ItemSlot& Slot = GetEquipSlot(SlotType);
	if (!Slot.InstanceId.IsValid()) return NAME_None;
	for (const FST_ItemInstance& Instance : m_EquippedItemInstances)
	{
		if (Instance.InstanceId == Slot.InstanceId)
		{
			return Instance.ItemId;
		}
	}
	return NAME_None;
}

FName UEquipComponent::GetEquipSlotRowName(EUnitEquipSlot SlotType) const
{
	return ResolveRowNameFromSlot(SlotType);
}

EUnitEquipSlot UEquipComponent::GetSlotTypeFromWeaponSlot(EWeaponSlot Slot) const
{
	return (Slot == EWeaponSlot::RightHand) ? EUnitEquipSlot::HandR : EUnitEquipSlot::HandL;
}

EUnitEquipSlot UEquipComponent::GetSlotTypeFromEquipType(EEquipType Type) const
{
	switch (Type)
	{
	case EEquipType::Head: return EUnitEquipSlot::Head;
	case EEquipType::Body: return EUnitEquipSlot::Body;
	case EEquipType::Horse: return EUnitEquipSlot::Horse;
	default: return EUnitEquipSlot::Body;
	}
}

void UEquipComponent::ApplySlotItem(EUnitEquipSlot SlotType, const FST_ItemInstance& ItemInstance)
{
	FST_ItemSlot& Slot = GetEquipSlot(SlotType);
	Slot.InstanceId = ItemInstance.InstanceId;
	Slot.ItemAssetId = ItemInstance.ItemAssetId;
	Slot.Quantity = ItemInstance.Quantity;

	for (FST_ItemInstance& Existing : m_EquippedItemInstances)
	{
		if (Existing.InstanceId == ItemInstance.InstanceId)
		{
			Existing = ItemInstance;
			return;
		}
	}

	m_EquippedItemInstances.Add(ItemInstance);
}

bool UEquipComponent::ValidateItemInstanceForSlot(const FST_ItemInstance& ItemInstance, EUnitEquipSlot SlotType) const
{
	if (ItemInstance.ItemId.IsNone())
	{
		return false;
	}

	if (SlotType == EUnitEquipSlot::HandR || SlotType == EUnitEquipSlot::HandL)
	{
		UDataTable* WeaponTable = RtsSettings::GetWeaponTable(GetOwner());
		const FST_Weapon* WeaponData = WeaponTable ? WeaponTable->FindRow<FST_Weapon>(ItemInstance.ItemId, TEXT("ValidateItemInstanceForSlot")) : nullptr;
		if (!WeaponData)
		{
			UE_LOG(LogTemp, Warning, TEXT("EquipItemInstance: '%s' is not a valid weapon row for slot %d."), *ItemInstance.ItemId.ToString(), (int32)SlotType);
			return false;
		}

		if (SlotType == EUnitEquipSlot::HandR &&
			(WeaponData->HandConstraint == EWeaponHandConstraint::ForceLeft || WeaponData->HandConstraint == EWeaponHandConstraint::Shield))
		{
			UE_LOG(LogTemp, Warning, TEXT("EquipItemInstance: '%s' cannot be equipped to right hand."), *ItemInstance.ItemId.ToString());
			return false;
		}

		if (SlotType == EUnitEquipSlot::HandL && WeaponData->HandConstraint == EWeaponHandConstraint::ForceRight)
		{
			UE_LOG(LogTemp, Warning, TEXT("EquipItemInstance: '%s' cannot be equipped to left hand."), *ItemInstance.ItemId.ToString());
			return false;
		}

		return true;
	}

	if (SlotType == EUnitEquipSlot::Head || SlotType == EUnitEquipSlot::Body || SlotType == EUnitEquipSlot::Horse)
	{
		UDataTable* ArmorTable = RtsSettings::GetArmorTable(GetOwner());
		const FST_Armor* ArmorData = ArmorTable ? ArmorTable->FindRow<FST_Armor>(ItemInstance.ItemId, TEXT("ValidateItemInstanceForSlot")) : nullptr;
		if (!ArmorData)
		{
			UE_LOG(LogTemp, Warning, TEXT("EquipItemInstance: '%s' is not a valid armor row for slot %d."), *ItemInstance.ItemId.ToString(), (int32)SlotType);
			return false;
		}

		const EEquipType ExpectedType =
			(SlotType == EUnitEquipSlot::Head) ? EEquipType::Head :
			(SlotType == EUnitEquipSlot::Body) ? EEquipType::Body :
			EEquipType::Horse;

		if (ArmorData->EquipType != ExpectedType)
		{
			UE_LOG(LogTemp, Warning, TEXT("EquipItemInstance: '%s' equip type mismatch. slot=%d, itemType=%d"), *ItemInstance.ItemId.ToString(), (int32)SlotType, (int32)ArmorData->EquipType);
			return false;
		}

		return true;
	}

	return false;
}

void UEquipComponent::NotifySlotUpdated(EUnitEquipSlot SlotType)
{
	switch (SlotType)
	{
	case EUnitEquipSlot::HandR:
		OnRep_RightWeaponName();
		UpdateBattleAnimType();
		break;
	case EUnitEquipSlot::HandL:
		OnRep_LeftWeaponName();
		UpdateBattleAnimType();
		break;
	case EUnitEquipSlot::Head:
		OnRep_ArmorHeadName();
		break;
	case EUnitEquipSlot::Body:
		OnRep_ArmorBodyName();
		break;
	case EUnitEquipSlot::Horse:
		OnRep_ArmorHorseName();
		break;
	default:
		break;
	}
}

bool UEquipComponent::IsEquipSlotEmpty(EUnitEquipSlot SlotType) const
{
	const FST_ItemSlot& Slot = GetEquipSlot(SlotType);
	return !Slot.InstanceId.IsValid();
}

void UEquipComponent::ClearEquipSlot(EUnitEquipSlot SlotType)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	FST_ItemSlot& Slot = GetEquipSlot(SlotType);
	const FGuid PreviousInstanceId = Slot.InstanceId;
	Slot.InstanceId.Invalidate();
	Slot.ItemAssetId = FPrimaryAssetId();
	Slot.Quantity = 0;

	if (PreviousInstanceId.IsValid())
	{
		bool bIsStillReferenced = false;
		for (int32 Index = 0; Index < m_EquipSlots.Num(); ++Index)
		{
			if (Index == static_cast<int32>(SlotType)) continue;
			if (m_EquipSlots[Index].InstanceId == PreviousInstanceId)
			{
				bIsStillReferenced = true;
				break;
			}
		}

		if (!bIsStillReferenced)
		{
			m_EquippedItemInstances.RemoveAll([&PreviousInstanceId](const FST_ItemInstance& Instance)
			{
				return Instance.InstanceId == PreviousInstanceId;
			});
		}
	}

	switch (SlotType)
	{
	case EUnitEquipSlot::HandR: OnRep_RightWeaponName(); break;
	case EUnitEquipSlot::HandL: OnRep_LeftWeaponName(); break;
	case EUnitEquipSlot::Head: OnRep_ArmorHeadName(); break;
	case EUnitEquipSlot::Body: OnRep_ArmorBodyName(); break;
	case EUnitEquipSlot::Horse: OnRep_ArmorHorseName(); break;
	default: break;
	}

	UpdateBattleAnimType();
	UpdateTotalEquipStats();
}

void UEquipComponent::ClearAllEquipSlots()
{
	if (GetOwnerRole() != ROLE_Authority) return;

	ClearEquipSlot(EUnitEquipSlot::HandR);
	ClearEquipSlot(EUnitEquipSlot::HandL);
	ClearEquipSlot(EUnitEquipSlot::Head);
	ClearEquipSlot(EUnitEquipSlot::Body);
	ClearEquipSlot(EUnitEquipSlot::Horse);
}

// Called when the game starts
void UEquipComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<ACharacter>(GetOwner());
	m_LastReplicatedEquipSlots = m_EquipSlots;

	RefreshWeaponsInEditor();
}

void UEquipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipComponent, m_EquipSlots);
	DOREPLIFETIME(UEquipComponent, m_BattleAnimType);
}

void UEquipComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 1. 오른손 무기가 존재한다면 파괴
	if (WeaponActor_R && IsValid(WeaponActor_R))
	{
		WeaponActor_R->Destroy();
		WeaponActor_R = nullptr;
	}

	// 2. 왼손(또는 방패) 무기가 존재한다면 파괴
	if (WeaponActor_L && IsValid(WeaponActor_L))
	{
		WeaponActor_L->Destroy();
		WeaponActor_L = nullptr;
	}

	// 부모 클래스의 EndPlay 호출을 잊지 마세요!
	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void UEquipComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 1. CDO(설계도)이거나 아키타입(템플릿)인 경우 에디터 로직 실행 방지
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		return;

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UEquipComponent, m_EquipSlots))
	{
		// 월드가 유효하고 게임 월드가 아닐 때(에디터 뷰포트)만 실행
		if (GetWorld() && !GetWorld()->IsGameWorld())
		{
			RefreshWeaponsInEditor();
		}
	}
}
#endif

void UEquipComponent::RefreshWeaponsInEditor()
{
	// 기존에 만든 장착 로직을 재사용합니다.
	// HandleWeaponAttachment 내부에서 이미 Destroy() 로직이 있으므로 안전합니다.
	HandleWeaponAttachment(GetEquipSlotRowName(EUnitEquipSlot::HandR), EWeaponSlot::RightHand);
	HandleWeaponAttachment(GetEquipSlotRowName(EUnitEquipSlot::HandL), EWeaponSlot::LeftHand);
	UpdateBattleAnimType();
}

void UEquipComponent::EquipItemInstance(const FST_ItemInstance& ItemInstance, EUnitEquipSlot SlotType)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	if (ItemInstance.ItemId.IsNone() || !ItemInstance.InstanceId.IsValid())
	{
		ClearEquipSlot(SlotType);
		return;
	}

	if (!ValidateItemInstanceForSlot(ItemInstance, SlotType))
	{
		return;
	}

	ApplySlotItem(SlotType, ItemInstance);
	NotifySlotUpdated(SlotType);

	UpdateTotalEquipStats();
}

void UEquipComponent::EquipToWeaponInstance(const FST_ItemInstance& ItemInstance)
{
	if (GetOwnerRole() != ROLE_Authority || ItemInstance.ItemId.IsNone()) return;

	UDataTable* WeaponTable = RtsSettings::GetWeaponTable(GetOwner());
	if (!WeaponTable) return;

	const FST_Weapon* WeaponData = WeaponTable->FindRow<FST_Weapon>(ItemInstance.ItemId, TEXT("EquipToWeaponInstance"));
	if (!WeaponData) return;

	EWeaponSlot TargetSlot = EWeaponSlot::RightHand;
	if (WeaponData->HandConstraint == EWeaponHandConstraint::ForceLeft ||
		WeaponData->HandConstraint == EWeaponHandConstraint::Shield)
	{
		TargetSlot = EWeaponSlot::LeftHand;
	}
	else if (WeaponData->HandConstraint == EWeaponHandConstraint::ForceRight)
	{
		TargetSlot = EWeaponSlot::RightHand;
	}

	EquipItemInstance(ItemInstance, GetSlotTypeFromWeaponSlot(TargetSlot));
}

void UEquipComponent::EquipWeaponByName(FName WeaponName, EWeaponSlot Slot)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	if (WeaponName.IsNone())
	{
		ClearEquipSlot(GetSlotTypeFromWeaponSlot(Slot));
		return;
	}

	FST_ItemInstance ItemInstance;
	ItemInstance.InstanceId = FGuid::NewGuid();
	ItemInstance.ItemId = WeaponName;
	EquipItemInstance(ItemInstance, GetSlotTypeFromWeaponSlot(Slot));
}

void UEquipComponent::EquipArmorByName(FName ArmorName, EEquipType Type)
{
	if (GetOwnerRole() != ROLE_Authority) return; // 서버에서만 실행 [cite: 1120-1121]

	if (ArmorName.IsNone())
	{
		ClearEquipSlot(GetSlotTypeFromEquipType(Type));
		return;
	}

	FST_ItemInstance ItemInstance;
	ItemInstance.InstanceId = FGuid::NewGuid();
	ItemInstance.ItemId = ArmorName;
	EquipItemInstance(ItemInstance, GetSlotTypeFromEquipType(Type));
}

void UEquipComponent::EquipToUnitData(FName InUnitRowName)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	m_UnitRowName = InUnitRowName;
	OnRep_UnitRowName(); // 서버에서도 비주얼 업데이트 실행
}

bool UEquipComponent::IsRideState() const
{
	return !GetEquipSlotRowName(EUnitEquipSlot::Horse).IsNone();
}

void UEquipComponent::UpdateTotalEquipStats()
{
	UDataTable* WeaponTable = RtsSettings::GetWeaponTable(GetOwner());
	if (WeaponTable == nullptr) return; // 데이터 테이블이 없으면 기본값 반환

	UStateComponent* StateComp = GetOwner()->FindComponentByClass<UStateComponent>();
	if (!StateComp) return;

	// 2. 장비 추가 스탯(Equip) 계산 시작
	FST_CombatStats NewEquipCombat;

	// 오른손 무기 공격력 합산
	const FName RightRowName = GetEquipSlotRowName(EUnitEquipSlot::HandR);
	if (WeaponTable && !RightRowName.IsNone())
	{
		if (FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(RightRowName, TEXT("")))
			NewEquipCombat.Attack += Data->AttackPower;
	}

	// 왼손 무기 공격력 합산
	const FName LeftRowName = GetEquipSlotRowName(EUnitEquipSlot::HandL);
	if (WeaponTable && !LeftRowName.IsNone())
	{
		if (FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(LeftRowName, TEXT("")))
			NewEquipCombat.Attack += Data->AttackPower;
	}

	// 3. 최종적으로 계산된 장비 보너스를 State에 주입
	// StateComponent 내부에서 자동으로 RecalculateTotalStats()가 실행됩니다.
	StateComp->SetEquipCombatStats(NewEquipCombat);
}

void UEquipComponent::OnRep_EquipSlots()
{
	const int32 MaxSlotCount = static_cast<int32>(EUnitEquipSlot::Max);
	for (int32 SlotIndex = 0; SlotIndex < MaxSlotCount; ++SlotIndex)
	{
		const bool bHasCurrent = m_EquipSlots.IsValidIndex(SlotIndex);
		const bool bHasPrevious = m_LastReplicatedEquipSlots.IsValidIndex(SlotIndex);
		FST_ItemSlot CurrentSlot;
		FST_ItemSlot PreviousSlot;
		if (bHasCurrent) CurrentSlot = m_EquipSlots[SlotIndex];
		if (bHasPrevious) PreviousSlot = m_LastReplicatedEquipSlots[SlotIndex];

		const bool bChanged = (CurrentSlot.InstanceId != PreviousSlot.InstanceId);
		if (!bChanged)
		{
			continue;
		}

		NotifySlotUpdated(static_cast<EUnitEquipSlot>(SlotIndex));
	}

	m_LastReplicatedEquipSlots = m_EquipSlots;
}

// --- OnRep  --- 
void UEquipComponent::OnRep_UnitRowName()
{
	UDataTable* UnitTable = RtsSettings::GetUnitTable(GetOwner());

	if (!UnitTable || m_UnitRowName.IsNone()) return;

	const FST_Unit* Data = UnitTable->FindRow<FST_Unit>(m_UnitRowName, TEXT(""));
	if (Data)
	{
		// 모든 스탯 계산은 이 함수가 책임집니다.
		UpdateTotalEquipStats();

		// 비주얼(메쉬) 업데이트만 수행
		OnUpdateUnitBody.Broadcast(*Data);
	}
}

void UEquipComponent::OnRep_RightWeaponName()
{
	// 세팅 가져오기
	UDataTable* WeaponTable = RtsSettings::GetWeaponTable(GetOwner());

	if (!WeaponTable) return;

	// 런타임에 실제 무기 액터를 스폰하고 부착합니다.
	const FName WeaponRowName = GetEquipSlotRowName(EUnitEquipSlot::HandR);
	HandleWeaponAttachment(WeaponRowName, EWeaponSlot::RightHand);

	FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(WeaponRowName, TEXT(""));
	if (Data)
	{
		// Ensure both the new (with id) and legacy delegates are invoked for right weapon updates.
		// Broadcasting on an unbound delegate is safe (no-op), so call both directly.
		OnUpdateWeaponWithId.Broadcast(m_OwnerUniqueId, EWeaponSlot::RightHand, *Data);
		OnUpdateWeapon.Broadcast(EWeaponSlot::RightHand, *Data);
	}
}

void UEquipComponent::OnRep_LeftWeaponName()
{
	// 세팅 가져오기
	UDataTable* WeaponTable = RtsSettings::GetWeaponTable(GetOwner());

	if (!WeaponTable) return;

	// 런타임에 실제 무기 액터를 스폰하고 부착합니다.
	const FName WeaponRowName = GetEquipSlotRowName(EUnitEquipSlot::HandL);
	HandleWeaponAttachment(WeaponRowName, EWeaponSlot::LeftHand);

	FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(WeaponRowName, TEXT(""));
	if (Data)
	{
		// Ensure both the new (with id) and legacy delegates are invoked for left weapon updates.
		// Broadcasting on an unbound delegate is safe (no-op), so call both directly.
		OnUpdateWeaponWithId.Broadcast(m_OwnerUniqueId, EWeaponSlot::LeftHand, *Data);
		OnUpdateWeapon.Broadcast(EWeaponSlot::LeftHand, *Data);
	}
}

void UEquipComponent::OnRep_ArmorHeadName()
{
	// 세팅 가져오기
	UDataTable* ArmorTable = RtsSettings::GetArmorTable(GetOwner());

	if (!ArmorTable) return;

	FName ArmorRowName = GetEquipSlotRowName(EUnitEquipSlot::Head);
	FST_Armor* Data = ArmorTable->FindRow<FST_Armor>(ArmorRowName, TEXT(""));
	if (Data)
	{
		// Ensure both the new (with id) and legacy delegates are invoked for head armor updates.
		// Broadcasting on an unbound delegate is safe (no-op), so call both directly.
		OnUpdateArmorWithId.Broadcast(m_OwnerUniqueId, EEquipType::Head, *Data);
		OnUpdateArmor.Broadcast(EEquipType::Head, *Data);
		// 캐릭터 기마 상태 업데이트 로직 호출
	}
}

void UEquipComponent::OnRep_ArmorBodyName()
{
	// 세팅 가져오기
	UDataTable* ArmorTable = RtsSettings::GetArmorTable(GetOwner());

	if (!ArmorTable) return;

	FName ArmorRowName = GetEquipSlotRowName(EUnitEquipSlot::Body);
	FST_Armor* Data = ArmorTable->FindRow<FST_Armor>(ArmorRowName, TEXT(""));
	if (Data)
	{
		// Ensure both the new (with id) and legacy delegates are invoked for body armor updates.
		// Broadcasting on an unbound delegate is safe (no-op), so call both directly.
		OnUpdateArmorWithId.Broadcast(m_OwnerUniqueId, EEquipType::Body, *Data);
		OnUpdateArmor.Broadcast(EEquipType::Body, *Data);
		// 캐릭터 기마 상태 업데이트 로직 호출
	}
}

void UEquipComponent::OnRep_ArmorHorseName()
{
	// 세팅 가져오기
	UDataTable* ArmorTable = RtsSettings::GetArmorTable(GetOwner());

	if (!ArmorTable) return;

	FName ArmorRowName = GetEquipSlotRowName(EUnitEquipSlot::Horse);
	FST_Armor* Data = ArmorTable->FindRow<FST_Armor>(ArmorRowName, TEXT(""));
	if (Data)
	{
		// Ensure both the new (with id) and legacy delegates are invoked for horse armor updates.
		// Broadcasting on an unbound delegate is safe (no-op), so call both directly.
		OnUpdateArmorWithId.Broadcast(m_OwnerUniqueId, EEquipType::Horse, *Data);
		OnUpdateArmor.Broadcast(EEquipType::Horse, *Data);
		// 캐릭터 기마 상태 업데이트 로직 호출
	}

	// 이름이 None이면(해제 시) 기마 상태 해제 알림
	OnUpdateRideState.Broadcast(IsRideState());
	UpdateBattleAnimType();
}

void UEquipComponent::OnRep_BattleAnimType()
{
	OnUpdateBattleAnimType.Broadcast(m_BattleAnimType);
}

void UEquipComponent::UpdateBattleAnimType()
{
	// 세팅 가져오기
	UDataTable* WeaponTable = RtsSettings::GetWeaponTable(GetOwner());

	// 1. [가드 클로저] 서버 권한 및 데이터 테이블 유효성 확인
	if (GetOwnerRole() != ROLE_Authority || !WeaponTable) return;

	EBattleAnimType NewType = EBattleAnimType::None;

	// 2. 무기 데이터 조회 (Null 체크 포함)
	const FST_Weapon* RightData = WeaponTable->FindRow<FST_Weapon>(GetEquipSlotRowName(EUnitEquipSlot::HandR), TEXT(""));
	const FST_Weapon* LeftData = WeaponTable->FindRow<FST_Weapon>(GetEquipSlotRowName(EUnitEquipSlot::HandL), TEXT(""));

	// 3. 애니메이션 타입 결정 (우선순위 로직)
	if (LeftData && LeftData->WeaponType == EWeaponType::Bow)
	{
		// 왼손에 활이 있으면 우선적으로 Bow 타입 지정
		NewType = EBattleAnimType::Bow;
	}
	else if (RightData && RightData->WeaponType == EWeaponType::Sword)
	{
		// 오른손에 검이 있으면 OneHandShield 타입 지정
		NewType = EBattleAnimType::OneHandShield;
	}
	else if (RightData || LeftData)
	{
		// 무기가 하나라도 있지만 위 조건에 해당하지 않을 경우의 기본값
		NewType = EBattleAnimType::OneHandShield;
	}

	// 4. 상태 변경 시에만 업데이트 및 로그 출력
	if (m_BattleAnimType != NewType)
	{
		// --- [정리된 화면 로그] ---
		if (GEngine)
		{
			uint64 Key = (uint64)GetOwner()->GetUniqueID() + 20;
			const UEnum* EnumPtr = StaticEnum<EBattleAnimType>();
			FString OldName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)m_BattleAnimType) : TEXT("None");
			FString NewName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)NewType) : TEXT("None");

			FString FinalMsg = FString::Printf(TEXT("[%s] Anim Style: %s -> %s"),
				*GetOwner()->GetName(), *OldName, *NewName);

			GEngine->AddOnScreenDebugMessage(Key, 5.0f, FColor::Cyan, FinalMsg);
		}

		m_BattleAnimType = NewType;
		OnRep_BattleAnimType(); // 클라이언트 알림
	}
}

FVector UEquipComponent::GetMainMuzzleLocation() const
{
	FString ActorName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
	uint64 Key = (uint64)GetOwner()->GetUniqueID() + 40;

	// 1. 메인 무기 액터 유효성 검사
	AWeapon* MainWeapon = IsValid(WeaponActor_R) ? WeaponActor_R : WeaponActor_L;
	if (!IsValid(MainWeapon))
	{
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	// 2. [수정된 인터페이스 호출] 캐스팅 후 직접 호출이 아닌 Execute_ 형식을 사용합니다.
	// BlueprintNativeEvent는 인터페이스 포인터가 아닌 액터 포인터를 넘겨야 합니다.
	if (!MainWeapon->Implements<UWeaponInterface>())
	{
		return MainWeapon->GetActorLocation();
	}

	// IWeaponInterface::Execute_함수이름(대상액터) 형식으로 호출
	FST_Weapon WeaponData = IWeaponInterface::Execute_GetWeaponData(MainWeapon);

	// 3. 데이터 및 머즐 소켓 유효성 검사
	if (WeaponData.MuzzleSocketName.IsNone())
	{
		return MainWeapon->GetActorLocation();
	}

	// 4. 무기 액터 '내부'의 메쉬 컴포넌트에서 소켓 찾기
	UMeshComponent* WeaponMesh = MainWeapon->FindComponentByClass<UMeshComponent>();
	if (!WeaponMesh || !WeaponMesh->DoesSocketExist(WeaponData.MuzzleSocketName))
	{
		return MainWeapon->GetActorLocation();
	}

	// 5. [성공] 최종 위치 반환
	if (GEngine) GEngine->AddOnScreenDebugMessage(Key, 1.0f, FColor::Green,
		FString::Printf(TEXT("[%s] Muzzle Found: %s"), *ActorName, *WeaponData.MuzzleSocketName.ToString()));

	return WeaponMesh->GetSocketLocation(WeaponData.MuzzleSocketName);
}

FName UEquipComponent::GetTargetSocketName(const FST_Weapon& WeaponData, EWeaponSlot RequestedSlot) const
{
	// 1. 우선적으로 무기 유형이 Shield인지 확인합니다.
	// 또는 HandConstraint가 Shield인 경우에도 ShieldSocketName을 반환합니다.
	if (WeaponData.WeaponType == EWeaponType::Shield ||
		WeaponData.HandConstraint == EWeaponHandConstraint::Shield)
	{
		return ShieldSocketName; // "Shield_Socket"
	}

	// 2. 그 외 강제 손 제약 확인
	if (WeaponData.HandConstraint == EWeaponHandConstraint::ForceLeft) return LeftHandSocketName;
	if (WeaponData.HandConstraint == EWeaponHandConstraint::ForceRight) return RightHandSocketName;

	// 3. 제약 없음: 요청받은 슬롯에 따라 결정
	return (RequestedSlot == EWeaponSlot::RightHand) ? RightHandSocketName : LeftHandSocketName;
}

void UEquipComponent::HandleWeaponAttachment(FName WeaponName, EWeaponSlot RequestedSlot)
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject)) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const URtsGameSettings* GameSettings = GetDefault<URtsGameSettings>();
	UDataTable* WeaponTable = RtsSettings::GetWeaponTable(GetOwner());

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !WeaponTable || !WeaponClass) return;

	USkeletalMeshComponent* AttachMesh = nullptr;

	// 인터페이스를 통한 메시 획득
	if (OwnerActor->Implements<UEquipableInterface>())
	{
		AttachMesh = IEquipableInterface::Execute_GetTargetAttachMesh(OwnerActor);
	}
	else
	{
		// 인터페이스를 안 붙인 액터가 있을 때 에러 로그 출력
		UE_LOG(LogTemp, Error, TEXT("[%s] IEquipableInterface를 구현하지 않아 무기를 부착할 수 없습니다!"), *OwnerActor->GetName());
		return;
	}

	if (!AttachMesh) return;

	// --- [기존 무기 해제 로직] ---
	// 1. 요청된 슬롯에 따라 현재 관리 중인 변수를 선택합니다.
	AWeapon** CurrentSlotVar = (RequestedSlot == EWeaponSlot::RightHand) ? &WeaponActor_R : &WeaponActor_L;

	// 2. [핵심] 새로운 값이 None이라면, 해당 슬롯의 무기를 지우고 종료합니다.
	if (WeaponName.IsNone())
	{
		if (*CurrentSlotVar && IsValid(*CurrentSlotVar))
		{
			(*CurrentSlotVar)->Destroy();
			*CurrentSlotVar = nullptr;
		}
		return;
	}

	// --- [새로운 무기 장착 로직] ---
	// 3. 데이터 조회
	FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(WeaponName, TEXT("Attachment"));
	if (!Data) return;

	// 4. 제공해주신 데이터(WeaponType 등)를 기반으로 소켓 결정
	FName FinalSocket = GetTargetSocketName(*Data, RequestedSlot);

	// 5. 방패나 왼손 무기는 무조건 WeaponActor_L 변수에서 관리하도록 타겟 변수를 재설정합니다.
	AWeapon** TargetActorVar = (FinalSocket == RightHandSocketName) ? &WeaponActor_R : &WeaponActor_L;

	// 6. 새로 장착될 위치에 이미 무기가 있다면 제거합니다.
	if (*TargetActorVar && IsValid(*TargetActorVar))
	{
		(*TargetActorVar)->Destroy();
		*TargetActorVar = nullptr;
	}

	// 7. 스폰 실행
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* NewWeapon = World->SpawnActor<AWeapon>(WeaponClass, SpawnParams);

	// [가드] 스폰 실패 처리
	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SpawnWeapon FAILED: %s"), *GetOwner()->GetName(), *WeaponName.ToString());
		return;
	}

	// 초기화 및 부착
	NewWeapon->InitializeWeapon(WeaponTable, WeaponName);

	// --- [부착 로직 및 로그] ---
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	NewWeapon->AttachToComponent(AttachMesh, AttachRules, FinalSocket);

	if (GEngine)
	{
		uint64 Key = (uint64)GetOwner()->GetUniqueID() + 60;
		FString AttachMsg = FString::Printf(TEXT("<Equip> %s attached to [%s]"), *WeaponName.ToString(), *FinalSocket.ToString());
		GEngine->AddOnScreenDebugMessage(Key, 3.0f, FColor::Cyan, AttachMsg);
	}

	// 변수 할당
	*TargetActorVar = NewWeapon;

	// --- [스킬 추가 로직 및 로그] ---
	USkillComponent* SkillComp = GetOwner()->FindComponentByClass<USkillComponent>();
	if (!SkillComp) return; // 스킬 컴포넌트 없으면 종료 (계단 방지)

	// 인터페이스를 통한 데이터 확보 (포인터가 아닌 값 반환 대응)
	FST_Weapon WeaponData = NewWeapon->GetWeaponData_Implementation();

	// 스킬 등록 및 상세 로그 출력
	if (!WeaponData.SkillName.IsNone())
	{
		SkillComp->AddSkill(WeaponData.SkillName);
		UE_LOG(LogTemp, Log, TEXT("[%s] Added Skill: %s"), *GetOwner()->GetName(), *WeaponData.SkillName.ToString());
	}

	if (!WeaponData.SkillNameRide.IsNone())
	{
		SkillComp->AddSkill(WeaponData.SkillNameRide);
		UE_LOG(LogTemp, Log, TEXT("[%s] Added Ride Skill: %s"), *GetOwner()->GetName(), *WeaponData.SkillNameRide.ToString());
	}

	// 화면에 최종 등록 결과 표시
	if (GEngine)
	{
		uint64 Key = (uint64)GetOwner()->GetUniqueID() + 61;
		FString SkillMsg = FString::Printf(TEXT("Skills: [%s] / [%s]"),
			*WeaponData.SkillName.ToString(), *WeaponData.SkillNameRide.ToString());
		GEngine->AddOnScreenDebugMessage(Key, 3.0f, FColor::Green, SkillMsg);
	}
}

void UEquipComponent::EquipToWeapon(FName WeaponName)
{
	if (GetOwnerRole() != ROLE_Authority || WeaponName.IsNone()) return;

	FST_ItemInstance ItemInstance;
	ItemInstance.InstanceId = FGuid::NewGuid();
	ItemInstance.ItemId = WeaponName;
	EquipToWeaponInstance(ItemInstance);
}

float UEquipComponent::GetAttackRange() const
{
	UDataTable* WeaponTable = RtsSettings::GetWeaponTable(GetOwner());
	if (WeaponTable == nullptr) return 0.0f; // 데이터 테이블이 없으면 기본값 반환

	UDataTable* UnitTable = RtsSettings::GetUnitTable(GetOwner());
	if (UnitTable == nullptr) return 0.0f; // 데이터 테이블이 없으면 기본값 반환

	// 1. 우선순위: 오른손 무기 데이터 확인
	const FName RightRowName = GetEquipSlotRowName(EUnitEquipSlot::HandR);
	if (!RightRowName.IsNone())
	{
		FST_Weapon* WeaponData = WeaponTable->FindRow<FST_Weapon>(RightRowName, TEXT("GetRange"));
		if (WeaponData) return (float)WeaponData->AttackRange;
	}

	// 2. 차순위: 왼손 무기 데이터 확인 (활이나 양손 무기 대응)
	const FName LeftRowName = GetEquipSlotRowName(EUnitEquipSlot::HandL);
	if (!LeftRowName.IsNone())
	{
		FST_Weapon* WeaponData = WeaponTable->FindRow<FST_Weapon>(LeftRowName, TEXT("GetRange"));
		if (WeaponData)
		{
			if (WeaponData->WeaponType != EWeaponType::Shield)
			{
				return (float)WeaponData->AttackRange;
			}
		}
	}

	// 3. 무기가 없을 때: 유닛 기본 데이터 테이블에서 사거리 반환
	if (!m_UnitRowName.IsNone())
	{
		const FST_Unit* UnitData = UnitTable->FindRow<FST_Unit>(m_UnitRowName, TEXT(""));
		if (UnitData)
		{
			// FST_Unit 구조체에도 AttackRange 변수가 정의되어 있어야 합니다.
			return (float)UnitData->AttackRange;
		}
	}

	// 모든 데이터가 없을 경우 기본값 반환
	return 0.0f;
}

float UEquipComponent::GetDetectionRange() const
{
	UDataTable* UnitTable = RtsSettings::GetUnitTable(GetOwner());
	if (UnitTable == nullptr) return 0.0f; // 데이터 테이블이 없으면 기본값 반환

	// 유닛 데이터 테이블에서 해당 유닛의 인지 범위를 가져옵니다.
	if (!m_UnitRowName.IsNone())
	{
		const FST_Unit* Data = UnitTable->FindRow<FST_Unit>(m_UnitRowName, TEXT(""));
		if (Data)
		{
			return Data->DetectionRange;
		}
	}

	return 800.0f; // 데이터가 없을 경우의 기본값
}
