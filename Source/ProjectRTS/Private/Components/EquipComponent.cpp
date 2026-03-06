// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/EquipComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StateComponent.h"

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

	DOREPLIFETIME(UEquipComponent, m_Weapon);
	DOREPLIFETIME(UEquipComponent, m_Armor_Head);
	DOREPLIFETIME(UEquipComponent, m_Armor_Body);
	DOREPLIFETIME(UEquipComponent, m_Armor_Horse);
}

void UEquipComponent::EquipWeapon(const FST_Weapon& NewWeapon)
{
	if (GetOwnerRole() != ROLE_Authority) return; // 서버에서만 실행 [cite: 1089-1090]

	m_Weapon = NewWeapon;
	OnRep_Weapon();

	// StateComponent가 있다면 공격력 반영 (선택 사항)
	if (UStateComponent* StateComp = GetOwner()->FindComponentByClass<UStateComponent>())
	{
		// 무기 공격력을 기본 공격력에 합산하는 로직 추가 가능
	}
}

void UEquipComponent::EquipArmor(const FST_Armor& NewArmor)
{
	if (GetOwnerRole() != ROLE_Authority) return; // 서버에서만 실행 [cite: 1120-1121]

	switch (NewArmor.EquipType) // 
	{
	case EEquipType::Head: m_Armor_Head = NewArmor; OnRep_ArmorHead(); break;
	case EEquipType::Body: m_Armor_Body = NewArmor; OnRep_ArmorBody(); break;
	case EEquipType::Horse: m_Armor_Horse = NewArmor; OnRep_ArmorHorse(); break;
	default: break;
	}
}

void UEquipComponent::EquipToUnitData(const FST_Unit& UnitData)
{
	// 유닛 기본 파츠 메시 설정 로직 (필요시 구현) [cite: 1244-1245]
}

// --- OnRep 및 메시 업데이트 로직 --- 

void UEquipComponent::OnRep_Weapon()
{
	OnUpdateWeapon.Broadcast(m_Weapon);
	// 여기에 무기 StaticMesh를 소켓에 부착하는 로직 추가
}

void UEquipComponent::OnRep_ArmorHead()
{
	OnUpdateArmor.Broadcast(EEquipType::Head, m_Armor_Head);
	if (OwnerChar && m_Armor_Head.SkeletalMesh)
	{
		// 머리 메시 교체 로직 (예: 캐릭터의 머리 파츠 SkeletalMeshComponent 업데이트)
	}
}

void UEquipComponent::OnRep_ArmorBody()
{
	OnUpdateArmor.Broadcast(EEquipType::Body, m_Armor_Body);
	// 몸통 메시 교체 로직
}

void UEquipComponent::OnRep_ArmorHorse()
{
	OnUpdateArmor.Broadcast(EEquipType::Horse, m_Armor_Horse);
	// 탈것 메시 교체 로직
}
