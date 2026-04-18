// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/EquipComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Actors/Weapon.h"
#include "Components/StateComponent.h"
#include "Components/SkillComponent.h"
#include "Interface/WeaponInterface.h"

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

	RefreshWeaponsInEditor();
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
	UpdateBattleAnimType();
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
	UpdateTotalEquipStats();
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

	UpdateTotalEquipStats();
}

void UEquipComponent::EquipToUnitData(FName InUnitRowName)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	m_UnitRowName = InUnitRowName;
	OnRep_UnitRowName(); // 서버에서도 비주얼 업데이트 실행
}

void UEquipComponent::UpdateTotalEquipStats()
{
	UStateComponent* StateComp = GetOwner()->FindComponentByClass<UStateComponent>();
	if (!StateComp) return;

	// 1. [Safety] 유닛 기본 데이터(Base)부터 다시 로드해서 State에 던져줍니다.
	// m_UnitRowName을 기준으로 항상 최신 베이스 스탯을 보장합니다.
	if (UnitTable && !m_UnitRowName.IsNone())
	{
		if (const FST_Unit* UnitData = UnitTable->FindRow<FST_Unit>(m_UnitRowName, TEXT("")))
		{
			StateComp->SetBaseStats(*UnitData);
		}
	}

	// 2. 장비 추가 스탯(Equip) 계산 시작
	FST_UnitStats NewEquipStats;

	// 오른손 무기 공격력 합산
	if (WeaponTable && !m_RightWeaponName.IsNone())
	{
		if (FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(m_RightWeaponName, TEXT("")))
			NewEquipStats.Attack += Data->AttackPower;
	}

	// 왼손 무기 공격력 합산
	if (WeaponTable && !m_LeftWeaponName.IsNone())
	{
		if (FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(m_LeftWeaponName, TEXT("")))
			NewEquipStats.Attack += Data->AttackPower;
	}

	// 3. 최종적으로 계산된 장비 보너스를 State에 주입
	// StateComponent 내부에서 자동으로 RecalculateTotalStats()가 실행됩니다.
	StateComp->SetEquipStats(NewEquipStats);
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
		// 모든 스탯 계산은 이 함수가 책임집니다.
		UpdateTotalEquipStats();

		// 비주얼(메쉬) 업데이트만 수행
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
	// 1. [가드 클로저] 서버 권한 및 데이터 테이블 유효성 확인
	if (GetOwnerRole() != ROLE_Authority || !WeaponTable) return;

	EBattleAnimType NewType = EBattleAnimType::None;

	// 2. 무기 데이터 조회 (Null 체크 포함)
	const FST_Weapon* RightData = WeaponTable->FindRow<FST_Weapon>(m_RightWeaponName, TEXT(""));
	const FST_Weapon* LeftData = WeaponTable->FindRow<FST_Weapon>(m_LeftWeaponName, TEXT(""));

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
	if (!World || !WeaponTable || !WeaponClass) return;

	ACharacter* TargetChar = OwnerChar ? OwnerChar : Cast<ACharacter>(GetOwner());
	if (!TargetChar) return;

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
	NewWeapon->AttachToComponent(TargetChar->GetMesh(), AttachRules, FinalSocket);

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

float UEquipComponent::GetAttackRange() const
{
	// 1. 우선순위: 오른손 무기 데이터 확인
	if (WeaponTable && !m_RightWeaponName.IsNone())
	{
		FST_Weapon* WeaponData = WeaponTable->FindRow<FST_Weapon>(m_RightWeaponName, TEXT("GetRange"));
		if (WeaponData)
		{
			return (float)WeaponData->AttackRange;
		}
	}

	// 2. 차순위: 왼손 무기 데이터 확인 (활이나 양손 무기 대응)
	if (WeaponTable && !m_LeftWeaponName.IsNone())
	{
		FST_Weapon* WeaponData = WeaponTable->FindRow<FST_Weapon>(m_LeftWeaponName, TEXT("GetRange"));
		if (WeaponData)
		{
			// 무기 타입이 Shield가 아닌 경우에만 사거리로 인정하는 로직을 추가할 수도 있습니다.
			if (WeaponData->WeaponType != EWeaponType::Shield)
			{
				return (float)WeaponData->AttackRange;
			}
		}
	}

	// 3. 무기가 없을 때: 유닛 기본 데이터 테이블에서 사거리 반환
	if (UnitTable && !m_UnitRowName.IsNone())
	{
		const FST_Unit* UnitData = GetUnitData(m_UnitRowName);
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
	// 유닛 데이터 테이블에서 해당 유닛의 인지 범위를 가져옵니다.
	if (UnitTable && !m_UnitRowName.IsNone())
	{
		if (const FST_Unit* Data = GetUnitData(m_UnitRowName))
		{
			return Data->DetectionRange;
		}
	}

	return 800.0f; // 데이터가 없을 경우의 기본값
}
