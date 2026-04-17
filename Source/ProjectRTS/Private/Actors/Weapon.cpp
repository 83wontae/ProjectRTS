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