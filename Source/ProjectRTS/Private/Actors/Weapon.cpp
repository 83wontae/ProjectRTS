// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h" // BoxTrace용
#include "Kismet/GameplayStatics.h"    // ApplyDamage용
#include "Interface/UnitInterface.h"
#include "GameFramework/Character.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (WeaponTable && !WeaponRowName.IsNone())
	{
		FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(WeaponRowName, TEXT("InitWeapon"));
		if (Data && WeaponMesh)
		{
			WeaponMesh->SetStaticMesh(Data->StaticMesh);
		}
	}
}

void AWeapon::InitializeWeapon(UDataTable* InTable, FName InRowName)
{
	if (InTable && !InRowName.IsNone())
	{
		WeaponTable = InTable;
		WeaponRowName = InRowName;

		// 메시 즉시 업데이트 (에디터 뷰포트 반영용)
		FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(WeaponRowName, TEXT("Init"));
		if (Data && WeaponMesh)
		{
			WeaponMesh->SetStaticMesh(Data->StaticMesh);
		}
	}
}

void AWeapon::ProcessWeaponHit()
{
	if (!WeaponMesh) return;

	// 1. 트레이스 시작/끝 위치 및 회전 (소켓 기준)
	const FVector StartPos = WeaponMesh->GetSocketLocation(TEXT("Start"));
	const FVector EndPos = WeaponMesh->GetSocketLocation(TEXT("End"));
	const FRotator Orientation = WeaponMesh->GetSocketRotation(TEXT("Start"));

	// 2. 트레이스 설정 (이미지의 BoxTraceForObjects 설정값 반영)
	const FVector HalfSize(12.f, 12.f, 12.f);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // Pawn 타입 검사

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner()); // 무기 소유자(나 자신)는 제외

	TArray<FHitResult> OutHits;

	// 3. 박스 트레이스 실행
	bool bHit = UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		StartPos,
		EndPos,
		HalfSize,
		Orientation,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration, // 디버그 라인 표시 (개발용)
		OutHits,
		true
	);

	if (bHit)
	{
		// 무기 데이터에서 공격력 가져오기
		FST_Weapon Data = GetWeaponData_Implementation();
		float Damage = (float)Data.AttackPower;

		// 4. 히트된 모든 액터에게 대미지 전달
		for (const FHitResult& Hit : OutHits)
		{
			if (Hit.GetActor())
			{
				UGameplayStatics::ApplyDamage(
					Hit.GetActor(),
					Damage,
					GetInstigatorController(), // 공격을 시킨 컨트롤러
					this,                      // 대미지 전달자 (무기)
					UDamageType::StaticClass()
				);
			}
		}
	}
}

bool AWeapon::IsCharRiding() const
{
	AActor* WeaponOwner = GetOwner();

	// 소유자가 존재하고, UnitInterface를 구현하고 있는지 확인합니다.
	if (WeaponOwner && WeaponOwner->Implements<UUnitInterface>())
	{
		// 인터페이스를 통해 기마 상태를 가져옵니다.
		// C++ 인터페이스의 BlueprintNativeEvent는 Execute_ 함수명을 사용합니다.
		return IUnitInterface::Execute_IsRiding(WeaponOwner);
	}

	return false;
}

void AWeapon::FireProjectile()
{
	if (!ProjectileClass || !WeaponMesh) return;

	// 1. 발사 위치 및 회전 설정 (범용 소켓 이름 사용)
	const FVector SpawnLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
	const FRotator SpawnRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);

	// 2. 스폰 설정 (Owner와 Instigator를 설정해야 대미지 처리가 정확해집니다)
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner(); // 무기 주인 (Character)
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 3. 발사체 액터 생성
	// 여기서 스폰만 해주면, 발사체 액터 내부의 ProjectileMovementComponent가 
	// BeginPlay 시점에 SpawnRotation 방향으로 날아가기 시작합니다.
	AActor* SpawnedProjectile = GetWorld()->SpawnActor<AActor>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (SpawnedProjectile)
	{
		// [필요 시] 발사체에게 추가 정보 전달 (예: 공격력 수치)
		// IProjectileInterface* ProjectileInt = Cast<IProjectileInterface>(SpawnedProjectile);
		// if (ProjectileInt) { ProjectileInt->Setup(GetWeaponData().AttackPower); }
	}
}

FST_Weapon AWeapon::GetWeaponData_Implementation() const
{
	if (WeaponTable && !WeaponRowName.IsNone())
	{
		FST_Weapon* Data = WeaponTable->FindRow<FST_Weapon>(WeaponRowName, TEXT("GetData"));
		if (Data) return *Data;
	}

	return FST_Weapon(); // 데이터를 못 찾으면 빈 구조체 반환
}

void AWeapon::SetWeaponVisibility_Implementation(bool bVisible)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetVisibility(bVisible);
	}
}

void AWeapon::OnWeaponAttack_Implementation(bool bIsRiding)
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar == nullptr)
		return;

	if (WeaponTable == nullptr)
		return;

	if (WeaponRowName.IsNone())
		return;

	FST_Weapon Data = GetWeaponData_Implementation();
	if (Data.WeaponType == EWeaponType::None)
		return;

	UAnimMontage* SelectedMontage = IsCharRiding() ? Data.RideAttackAnimation : Data.AttackAnimation;

	// 4. 선택된 몽타주 재생 [이미지: Play Anim Montage]
	if (SelectedMontage)
	{
		OwnerChar->PlayAnimMontage(SelectedMontage);
	}
}

void AWeapon::ExecuteAttackNotify_Implementation()
{
	// 1. 현재 무기의 데이터를 가져옵니다.
	FST_Weapon Data = GetWeaponData_Implementation();
	if (Data.WeaponType == EWeaponType::None)
		return;

	// 2. [이미지 888f1e 로직] 무기 타입에 따른 분기 처리
	switch (Data.WeaponType)
	{
	case EWeaponType::Sword:
		// 검(Sword)일 경우 근접 히트 판정 실행
		ProcessWeaponHit();
		break;

	case EWeaponType::Bow:
		// 활(Bow)일 경우 발사체 발사 실행
		FireProjectile();
		break;

	default:
		// 방패(Shield) 등 다른 타입은 현재 판정 로직이 없으므로 통과
		break;
	}
}
