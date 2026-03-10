// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/EquipComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UEquipComponent::UEquipComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UEquipComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<ACharacter>(GetOwner());
	
}

void UEquipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipComponent, m_RightWeaponName);
	DOREPLIFETIME(UEquipComponent, m_LeftWeaponName);
	DOREPLIFETIME(UEquipComponent, m_ArmorHeadName);
	DOREPLIFETIME(UEquipComponent, m_ArmorBodyName);
	DOREPLIFETIME(UEquipComponent, m_ArmorHorseName);
	DOREPLIFETIME(UEquipComponent, m_BattleAnimType);
}

void UEquipComponent::EquipWeaponByName(FName WeaponName, EWeaponSlot Slot)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	if (Slot == EWeaponSlot::RightHand)
	{
		m_RightWeaponName = WeaponName;
		OnRep_RightWeaponName();
	}
	else
	{
		m_LeftWeaponName = WeaponName;
		OnRep_LeftWeaponName();
	}
	UpdateBattleAnimType();
}

void UEquipComponent::EquipArmorByName(FName ArmorName, EEquipType Type)
{
	if (GetOwnerRole() != ROLE_Authority) return; // 서버에서만 실행 [cite: 1120-1121]

	switch (Type) // 
	{
	case EEquipType::Head: m_ArmorHeadName = ArmorName; break;
	case EEquipType::Body: m_ArmorBodyName = ArmorName; break;
	case EEquipType::Horse: m_ArmorHorseName = ArmorName; break;
	default: break;
	}
}

void UEquipComponent::EquipToUnitData(FName InUnitRowName)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	m_UnitRowName = InUnitRowName;
	OnRep_UnitRowName(); // 서버에서도 비주얼 업데이트 실행
}

const FST_Unit* UEquipComponent::GetUnitData(FName InUnitRowName) const
{
	if (!UnitTable || m_UnitRowName.IsNone()) nullptr;

	const FST_Unit* Data = UnitTable->FindRow<FST_Unit>(m_UnitRowName, TEXT(""));
	
	return Data;
}

// --- OnRep  --- 
void UEquipComponent::OnRep_UnitRowName()
{
	if (!UnitTable || m_UnitRowName.IsNone()) return;

	const FST_Unit* Data = UnitTable->FindRow<FST_Unit>(m_UnitRowName, TEXT(""));
	if (Data)
	{
		// 핵심: 직접 메쉬를 바꾸지 않고 데이터만 던집니다.
		OnUpdateUnitBody.Broadcast(*Data);
	}
}

void UEquipComponent::OnRep_RightWeaponName()
{
	if (!WeaponTable) return;

	FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(m_RightWeaponName, TEXT(""));
	if (Data)
	{
		// 구조체 데이터를 이벤트로 전달 (기존 UI 로직 호환)
		OnUpdateWeapon.Broadcast(EWeaponSlot::RightHand, *Data);
	}
}

void UEquipComponent::OnRep_LeftWeaponName()
{
	if (!WeaponTable) return;

	FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(m_LeftWeaponName, TEXT(""));
	if (Data)
	{
		// 구조체 데이터를 이벤트로 전달 (기존 UI 로직 호환)
		OnUpdateWeapon.Broadcast(EWeaponSlot::LeftHand, *Data);
	}
}

void UEquipComponent::OnRep_ArmorHeadName()
{
	if (!ArmorTable) return;

	FST_Armor* Data = ArmorTable->FindRow<FST_Armor>(m_ArmorHeadName, TEXT(""));
	if (Data)
	{
		OnUpdateArmor.Broadcast(EEquipType::Head, *Data);
		// 캐릭터 기마 상태 업데이트 로직 호출
	}
}

void UEquipComponent::OnRep_ArmorBodyName()
{
	if (!ArmorTable) return;

	FST_Armor* Data = ArmorTable->FindRow<FST_Armor>(m_ArmorBodyName, TEXT(""));
	if (Data)
	{
		OnUpdateArmor.Broadcast(EEquipType::Body, *Data);
		// 캐릭터 기마 상태 업데이트 로직 호출
	}
}

void UEquipComponent::OnRep_ArmorHorseName()
{
	if (!ArmorTable) return;

	FST_Armor* Data = ArmorTable->FindRow<FST_Armor>(m_ArmorHorseName, TEXT(""));
	if (Data)
	{
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
	if (GetOwnerRole() != ROLE_Authority || !WeaponTable) return;

	EBattleAnimType NewType = EBattleAnimType::None;

	// RowName을 이용해 데이터 조회 후 로직 판정
	FST_Weapon* RightData = WeaponTable->FindRow<FST_Weapon>(m_RightWeaponName, TEXT(""));
	if (RightData)
	{
		if (RightData->WeaponType == EWeaponType::Bow) NewType = EBattleAnimType::Bow;
		else if (RightData->WeaponType == EWeaponType::Sword) NewType = EBattleAnimType::OneHandShield;
	}

	if (m_BattleAnimType != NewType)
	{
		m_BattleAnimType = NewType;
		OnRep_BattleAnimType();
	}
}