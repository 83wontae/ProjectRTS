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
	CreateDynamicMaterial(); // 반투명 머티리얼 준비

	if (PlacementComp && !BuildingRowName.IsNone())
	{
		// 1. 컴포넌트에게 데이터 테이블 정보를 찾아오라고 시킴
		if (PlacementComp->InitializePlacementData(BuildingRowName))
		{
			// 2. [자연스러운 흐름] 데이터를 가져와서 액터가 직접 자기 메시를 바꿈
			const FST_Building& Data = PlacementComp->GetCurrentBuildingData();
			SetPreviewMesh(Data.BuildingMesh);

			// 3. 준비가 끝났으니 컴포넌트의 배치 모드(Tick 이동 로직) 활성화
			PlacementComp->ActivatePlacementMode();
		}
	}
}

void APreviewBuilding::SetupPreviewBuilding(FName InRowName)
{
	// 1. 전달받은 RowName 저장
	BuildingRowName = InRowName;

	// 2. 반투명 머티리얼 준비
	CreateDynamicMaterial();

	if (PlacementComp && !BuildingRowName.IsNone())
	{
		// 3. 컴포넌트에게 데이터 테이블 정보 로드 요청
		if (PlacementComp->InitializePlacementData(BuildingRowName))
		{
			// 4. 데이터를 가져와서 액터의 메시 설정
			const FST_Building& Data = PlacementComp->GetCurrentBuildingData();
			SetPreviewMesh(Data.BuildingMesh);

			// 5. 배치 모드 활성화 (Tick 이동 시작)
			PlacementComp->ActivatePlacementMode();
		}
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

void APreviewBuilding::SetPreviewMesh(UStaticMesh* NewMesh)
{
	if (MeshComponent && NewMesh)
	{
		// 1. 새로운 스태틱 메시 적용
		MeshComponent->SetStaticMesh(NewMesh);

		// 2. 메쉬가 변경되었으므로 고스트 머티리얼(홀로그램)을 다시 생성하여 입힘
		// 이 함수 내부에서 GhostMaterialBase를 사용하여 DynamicMaterial을 만들고 적용합니다.
		CreateDynamicMaterial();

		// 3. (옵션) 기존에 설정된 색상 상태(Valid/Invalid)가 있다면 유지하도록 처리
		// PlacementComp가 있다면 다음 Tick에서 자동으로 업데이트되겠지만, 
		// 즉각적인 반응을 위해 아래와 같이 호출할 수 있습니다.
		if (PlacementComp)
		{
			SetValidPlacement(PlacementComp->IsCanPlace());
		}
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