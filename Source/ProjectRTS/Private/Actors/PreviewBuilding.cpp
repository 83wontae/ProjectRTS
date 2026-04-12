// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/PreviewBuilding.h"
#include "Components/PlacementComponent.h"
#include "Components/DecalComponent.h" // 데칼 컴포넌트 포함

APreviewBuilding::APreviewBuilding()
{
	PrimaryActorTick.bCanEverTick = true; // Tick 활성화

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCastShadow(false);

	// 데칼 컴포넌트 생성 및 초기화
	GridDecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("GridDecal"));
	GridDecalComp->SetupAttachment(RootComponent);

	// 데칼을 바닥 방향으로 회전 (Pitch -90)
	GridDecalComp->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	// 데칼 크기 설정 (투영 깊이, Y반지름, Z반지름) - 1x1 타일(100) 기준
	GridDecalComp->DecalSize = FVector(200.0f, 100.0f, 100.0f);

	PlacementComp = CreateDefaultSubobject<UPlacementComponent>(TEXT("PlacementManager"));
}

void APreviewBuilding::BeginPlay()
{
	Super::BeginPlay();

	// 데칼용 다이내믹 머티리얼 생성 (머티리얼 노드의 GridColor 파라미터 제어용)
	if (GridDecalComp)
	{
		DecalDynamicMaterial = GridDecalComp->CreateDynamicMaterialInstance();
	}

	if (PlacementComp)
	{
		PlacementComp->StartPlacement(FName("DefaultBuilding"));
	}
}

void APreviewBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlacementComp)
	{
		// 1. 컴포넌트에서 설치 가능 여부를 가져옴
		bool bValid = PlacementComp->IsCanPlace();
		// 2. [수정] 가져온 결과에 따라 외형과 데칼 색상을 업데이트함
		SetValidPlacement(bValid);
	}
}

void APreviewBuilding::SetValidPlacement(bool bIsValid)
{
	FLinearColor TargetColor = bIsValid ? ValidColor : InvalidColor;

	// 고스트 메쉬 색상 변경
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("GhostColor"), TargetColor);
	}

	// [추가] 데칼 그리드 색상 변경 (머티리얼의 파라미터 이름 GridColor와 일치해야 함)
	if (DecalDynamicMaterial)
	{
		DecalDynamicMaterial->SetVectorParameterValue(TEXT("GridColor"), TargetColor);
	}
}

void APreviewBuilding::SetPreviewMesh(UStaticMesh* NewMesh)
{
	if (MeshComponent && NewMesh)
	{
		MeshComponent->SetStaticMesh(NewMesh);
		if (GhostMaterialBase)
		{
			DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0, GhostMaterialBase);
		}
	}
}