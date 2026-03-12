// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/IconComponent.h"
#include "Interface/UnitInterface.h"

UIconComponent::UIconComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 월드 표기용 메시를 기본적으로 생성 (기본은 숨김 처리)
	WorldMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WorldMarkerMesh"));
	WorldMarkerMesh->SetupAttachment(this);
	WorldMarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UIconComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1. 진영 정보 확인 및 색상 반영
	if (GetOwner()->Implements<UUnitInterface>())
	{
		int32 Faction = IUnitInterface::Execute_GetFaction(GetOwner());
		UpdateIconColorByFaction(Faction);
	}

	// 2. 미니맵 매니저에 등록하는 로직 추가 지점
	// 예: GetMiniMapManager()->Register(this);
}

void UIconComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 3. 무기 제거 로직과 마찬가지로, 월드에서 사라질 때 미니맵 리스트에서도 제거
	// 예: GetMiniMapManager()->Unregister(this);

	Super::EndPlay(EndPlayReason);
}

void UIconComponent::UpdateIconColorByFaction(int32 Faction)
{
	// 진영 번호에 따라 MinimapIcon의 틴트나 WorldMarkerMesh의 머티리얼 변경
}
