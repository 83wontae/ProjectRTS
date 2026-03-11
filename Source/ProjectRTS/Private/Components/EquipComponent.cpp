// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/EquipComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Actors/Weapon.h"

// Sets default values for this component's properties
UEquipComponent::UEquipComponent() :WeaponClass(AWeapon::StaticClass())
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

	HandleWeaponAttachment(m_RightWeaponName, EWeaponSlot::RightHand);
	HandleWeaponAttachment(m_LeftWeaponName, EWeaponSlot::LeftHand);
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

#if WITH_EDITOR
void UEquipComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 1. CDO(설계도)이거나 아키타입(템플릿)인 경우 에디터 로직 실행 방지
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		return;

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UEquipComponent, m_RightWeaponName) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(UEquipComponent, m_LeftWeaponName))
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
	HandleWeaponAttachment(m_RightWeaponName, EWeaponSlot::RightHand);
	HandleWeaponAttachment(m_LeftWeaponName, EWeaponSlot::LeftHand);
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

const FST_Unit* UEquipComponent::GetUnitData(FName InRowName) const
{
	return UnitTable ? UnitTable->FindRow<FST_Unit>(InRowName, TEXT("")) : nullptr;
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

FName UEquipComponent::GetTargetSocketName(const FST_Weapon& WeaponData, EWeaponSlot RequestedSlot) const
{
	// 1. 방패 제약 확인
	if (WeaponData.HandConstraint == EWeaponHandConstraint::Shield)
	{
		return ShieldSocketName;
	}

	// 2. 강제 손 제약 확인
	if (WeaponData.HandConstraint == EWeaponHandConstraint::ForceLeft) return LeftHandSocketName;
	if (WeaponData.HandConstraint == EWeaponHandConstraint::ForceRight) return RightHandSocketName;

	// 3. 제약 없음: 요청받은 슬롯에 따라 결정
	return (RequestedSlot == EWeaponSlot::RightHand) ? RightHandSocketName : LeftHandSocketName;
}

void UEquipComponent::HandleWeaponAttachment(FName WeaponName, EWeaponSlot RequestedSlot)
{
	// 2. 템플릿 객체인 경우 스폰 로직 원천 봉쇄
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject)) return;

	// 월드 포인터가 없으면 스폰 불가
	UWorld* World = GetWorld();
	if (!World) return;

	// 3. 소유자 캐릭터 안전하게 가져오기
	ACharacter* TargetChar = OwnerChar;
	if (!TargetChar) TargetChar = Cast<ACharacter>(GetOwner());

	// 타겟 캐릭터나 테이블이 없으면 즉시 종료
	if (!TargetChar || !WeaponTable || !WeaponClass || WeaponName.IsNone()) return;

	FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(WeaponName, TEXT("Attachment"));
	if (!Data) return;

	FName FinalSocket = GetTargetSocketName(*Data, RequestedSlot);
	AWeapon** TargetActorVar = (FinalSocket == RightHandSocketName) ? &WeaponActor_R : &WeaponActor_L;

	// 4. 기존 무기 제거 (IsValid 체크 강화)
	if (*TargetActorVar && IsValid(*TargetActorVar))
	{
		(*TargetActorVar)->Destroy();
		*TargetActorVar = nullptr;
	}

	// 5. 무기 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 월드를 통해 안전하게 스폰
	AWeapon* NewWeapon = World->SpawnActor<AWeapon>(WeaponClass, SpawnParams);

	if (NewWeapon)
	{
		NewWeapon->InitializeWeapon(WeaponTable, WeaponName);

		if (TargetChar->GetMesh())
		{
			FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
			NewWeapon->AttachToComponent(TargetChar->GetMesh(), AttachRules, FinalSocket);
			*TargetActorVar = NewWeapon;
		}
	}
}