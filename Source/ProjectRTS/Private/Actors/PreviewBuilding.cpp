// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/PreviewBuilding.h"

// Sets default values
APreviewBuilding::APreviewBuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 중요: 고스트 건물은 마우스 클릭이나 유닛 이동에 방해되지 않도록 콜리전을 끕니다.
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCastShadow(false);
}

void APreviewBuilding::SetPreviewMesh(UStaticMesh* NewMesh)
{
	if (MeshComponent && NewMesh)
	{
		MeshComponent->SetStaticMesh(NewMesh);

		// 다이내믹 머티리얼 생성 (색상 조절용)
		if (GhostMaterialBase)
		{
			DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0, GhostMaterialBase);
		}
	}
}

void APreviewBuilding::SetValidPlacement(bool bIsValid)
{
	if (DynamicMaterial)
	{
		FLinearColor TargetColor = bIsValid ? ValidColor : InvalidColor;
		DynamicMaterial->SetVectorParameterValue(TEXT("GhostColor"), TargetColor);
	}
}