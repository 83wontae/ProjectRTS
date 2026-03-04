// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/TeamSysComponent.h"
#include "Net/UnrealNetwork.h" // DOREPLIFETIME 사용을 위해 반드시 필요
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interface/UnitInterface.h"

// Sets default values for this component's properties
UTeamSysComponent::UTeamSysComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTeamSysComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTeamSysComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// 네트워크를 통해 어떤 변수를 복제할지 정의
void UTeamSysComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTeamSysComponent, m_TeamIndex); // TeamIndex 복제 등록
	DOREPLIFETIME(UTeamSysComponent, m_Leader);    // Leader 복제 등록
}

void UTeamSysComponent::OnRep_TeamIndex()
{
	OnTeamChanged.Broadcast(m_TeamIndex);
}

void UTeamSysComponent::OnRep_Leader()
{
	OnLeaderChanged.Broadcast(m_Leader);
}

void UTeamSysComponent::SetTeam(int32 NewTeamIndex)
{
	if (GetOwnerRole() == ROLE_Authority) // 서버에서만 값 변경
	{
		m_TeamIndex = NewTeamIndex;
		OnRep_TeamIndex(); // 서버에서도 수동 호출하여 로직 실행
	}
}

void UTeamSysComponent::SetLeader(AActor* NewLeader)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		UTeamSysComponent* LeaderTeamComp = NewLeader->FindComponentByClass<UTeamSysComponent>();

		if (LeaderTeamComp)
		{
			m_Leader = NewLeader;
			OnRep_Leader();

			SetTeam(LeaderTeamComp->m_TeamIndex);
		}
		else
		{
			const FString FailMsg = TEXT("Set Leader Fail( TeamSysComponent is not Added)");
			UE_LOG(LogTemp, Error, TEXT("%s"), *FailMsg);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, FailMsg);
			}
		}
	}
}

bool UTeamSysComponent::IsAlly(AActor* TargetActor) const
{
	if (!IsValid(TargetActor)) return false;

	if (TargetActor->Implements<UUnitInterface>())
	{
		return IsAllyByTeamIndex(IUnitInterface::Execute_GetTeamIndex(TargetActor));
	}
	return true;
}

bool UTeamSysComponent::IsEnemy(AActor* TargetActor) const
{
	return !IsAlly(TargetActor);
}

bool UTeamSysComponent::IsAllyByTeamIndex(int32 TeamIndex) const
{
	return (TeamIndex== m_TeamIndex);
}

bool UTeamSysComponent::IsEnemyByTeamIndex(int32 TeamIndex) const
{
	return !IsAllyByTeamIndex(TeamIndex);
}
