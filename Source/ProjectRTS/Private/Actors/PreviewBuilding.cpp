// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/PreviewBuilding.h"
#include "Components/PlacementComponent.h"
#include "Components/DecalComponent.h" // 데칼 컴포넌트 포함

APreviewBuilding::APreviewBuilding()
{
	PrimaryActorTick.bCanEverTick = true; // Tick 활성화

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCastShadow(false);
	MeshComponent->SetGenerateOverlapEvents(true);

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
	if (!MeshComponent || !NewMesh) return;

	// 1. 메쉬 설정 및 충돌 설정 (중요: GenerateOverlapEvents를 켜야 감지가 됩니다)
	MeshComponent->SetStaticMesh(NewMesh);
	MeshComponent->SetCollisionProfileName(TEXT("OverlapAll"));

	// 2. 데칼 크기 조정 (Radii가 아닌 전체 크기 기준)
	if (GridDecalComp)
	{
		const FST_Building& Data = PlacementComp->GetCurrentBuildingData();

		// 타일당 100유닛일 때, GridSize가 1이면 전체 크기는 100.0f가 되어야 합니다.
		// 0.5f를 곱하면 타일의 절반만 덮게 되어 그리드 선이 안 보일 수 있습니다.
		float DecalDefaultSize = 1000.0f;
		float NewSizeX = (DecalDefaultSize + Data.GridSizeX * 100.0f);
		float NewSizeY = (DecalDefaultSize + Data.GridSizeY * 100.0f);

		// X=투영깊이(200), Y=가로(NewSizeX), Z=세로(NewSizeY)
		GridDecalComp->DecalSize = FVector(200.0f, NewSizeX, NewSizeY);
	}

	// 3. 머티리얼 재생성 및 초기화
	CreateDynamicMaterial();
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
	if (GridDecalComp && GridDecalComp->GetDecalMaterial())
	{
		// 부모 머티리얼로부터 다이내믹 인스턴스를 생성하여 캐싱합니다.
		DecalDynamicMaterial = GridDecalComp->CreateDynamicMaterialInstance();
	}
}