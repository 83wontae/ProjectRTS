// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SpawnAreaUnit.h"
#include "AIController.h"
#include "Components/EquipComponent.h"
#include "Components/StateComponent.h"
#include "Core/RtsUnitCharacter.h"
#include "Global/RTSItemDefinition.h"

// Sets default values
ASpawnAreaUnit::ASpawnAreaUnit()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(SceneRoot);
	SphereComponent->SetSphereRadius(500.0f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetGenerateOverlapEvents(false);
	SphereComponent->bHiddenInGame = true;
}

void ASpawnAreaUnit::StartSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	if (SpawnIntervalSeconds <= 0.0f)
	{
		SpawnIntervalSeconds = 10.0f;
	}

	bPausedByLimit = false;
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ASpawnAreaUnit::SpawnLoopTick, SpawnIntervalSeconds, true, SpawnIntervalSeconds);
}

void ASpawnAreaUnit::StopSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	bPausedByLimit = false;
}

void ASpawnAreaUnit::PauseSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void ASpawnAreaUnit::ResumeSpawning()
{
	if (!HasAuthority())
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(SpawnTimerHandle))
	{
		return;
	}

	bPausedByLimit = false;
	StartSpawning();
}

void ASpawnAreaUnit::RefreshAliveUnits()
{
	AliveSpawnedUnits.RemoveAll([](const TWeakObjectPtr<ARtsUnitCharacter>& Unit)
	{
		if (!Unit.IsValid())
		{
			return true;
		}

		const UStateComponent* StateComp = Unit->FindComponentByClass<UStateComponent>();
		return StateComp && StateComp->IsDeath();
	});
}

bool ASpawnAreaUnit::CanSpawn() const
{
	if (MaxTotalSpawnCount > 0 && TotalSpawnedCount >= MaxTotalSpawnCount)
	{
		return false;
	}

	if (MaxAliveSpawnCount > 0 && AliveSpawnedUnits.Num() >= MaxAliveSpawnCount)
	{
		return false;
	}

	return true;
}

void ASpawnAreaUnit::SpawnLoopTick()
{
	if (!HasAuthority())
	{
		return;
	}

	RefreshAliveUnits();

	if (!CanSpawn())
	{
		bPausedByLimit = true;
		PauseSpawning();
		return;
	}

	const int32 SpawnCount = FMath::Max(1, SpawnPerInterval);
	for (int32 i = 0; i < SpawnCount; ++i)
	{
		if (!CanSpawn())
		{
			bPausedByLimit = true;
			PauseSpawning();
			break;
		}

		if (!TrySpawnUnit())
		{
			break;
		}
	}
}

bool ASpawnAreaUnit::TrySpawnUnit()
{
	if (!UnitClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnAreaUnit: UnitClass is not set."));
		return false;
	}

	const FVector Center = SphereComponent ? SphereComponent->GetComponentLocation() : GetActorLocation();
	const float Radius = SphereComponent ? SphereComponent->GetScaledSphereRadius() : 500.0f;
	const FVector2D RandomOffset2D = FMath::RandPointInCircle(Radius);
	FVector SpawnLocation = Center + FVector(RandomOffset2D.X, RandomOffset2D.Y, 0.0f);

	FHitResult Hit;
	const FVector TraceStart = SpawnLocation + FVector(0.0f, 0.0f, 2000.0f);
	const FVector TraceEnd = SpawnLocation - FVector(0.0f, 0.0f, 4000.0f);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpawnAreaUnitGroundTrace), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		SpawnLocation = Hit.Location;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ARtsUnitCharacter* SpawnedUnit = GetWorld()->SpawnActor<ARtsUnitCharacter>(
		UnitClass,
		SpawnLocation,
		GetActorRotation(),
		SpawnParams);

	if (!SpawnedUnit)
	{
		return false;
	}

	if (AIControllerClass)
	{
		SpawnedUnit->AIControllerClass = AIControllerClass;
	}
	SpawnedUnit->SpawnDefaultController();

	if (SpawnedUnit->StateComp)
	{
		SpawnedUnit->StateComp->SetFaction(m_Faction);
	}

	if (SpawnedUnit->EquipComp)
	{
		if (!RightWeaponDefinition.IsNull())
		{
			URTSWeaponDefinition* RightDefinition = RightWeaponDefinition.LoadSynchronous();
			if (RightDefinition && !RightDefinition->ItemId.IsNone())
			{
				SpawnedUnit->EquipComp->EquipWeaponByName(RightDefinition->ItemId, EWeaponSlot::RightHand);
			}
		}

		if (!LeftWeaponDefinition.IsNull())
		{
			URTSWeaponDefinition* LeftDefinition = LeftWeaponDefinition.LoadSynchronous();
			if (LeftDefinition && !LeftDefinition->ItemId.IsNone())
			{
				SpawnedUnit->EquipComp->EquipWeaponByName(LeftDefinition->ItemId, EWeaponSlot::LeftHand);
			}
		}
	}

	if (bOverrideBaseCombatStats && SpawnedUnit->StateComp)
	{
		SpawnedUnit->StateComp->SetBaseCombatStatsOverride(BaseCombatStats);
	}

	AliveSpawnedUnits.Add(SpawnedUnit);
	TotalSpawnedCount++;
	return true;
}

// Called when the game starts or when spawned
void ASpawnAreaUnit::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bStartLoopOnBeginPlay)
	{
		StartSpawning();
	}
}

