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

	CreateDynamicMaterial();

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

void APreviewBuilding::CreateDynamicMaterial()
{
	// 1. 메쉬 컴포넌트 유효성 확인
	if (!MeshComponent) return;

	// 2. 고스트 메쉬용 다이내믹 머티리얼 생성
	// 메쉬의 원래 머티리얼 대신, 미리 설정한 GhostMaterialBase(홀로그램 머티리얼)를 사용하도록 권장
	if (GhostMaterialBase)
	{
		// 이 함수는 다이내믹 인스턴스를 반환함과 동시에 Mesh의 0번 슬롯에 머티리얼을 즉시 적용합니다.
		DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0, GhostMaterialBase);
	}

	// 3. 데칼용 다이내믹 머티리얼 생성
	if (GridDecalComp)
	{
		// 데칼에 이미 머티리얼이 할당되어 있는지 확인 후 생성
		if (GridDecalComp->GetDecalMaterial())
		{
			DecalDynamicMaterial = GridDecalComp->CreateDynamicMaterialInstance();
		}
	}
}