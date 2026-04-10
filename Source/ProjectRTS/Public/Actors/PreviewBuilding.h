// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreviewBuilding.generated.h"

UCLASS()
class PROJECTRTS_API APreviewBuilding : public AActor
{
	GENERATED_BODY()

public:
	APreviewBuilding();

	/** 외형 메시 설정 */
	void SetPreviewMesh(UStaticMesh* NewMesh);

	/** 설치 가능 여부에 따른 색상 변경 */
	void SetValidPlacement(bool bIsValid);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	UStaticMeshComponent* MeshComponent;

	/** 설치 가능/불가능 시 적용할 머티리얼 인스턴스 */
	UPROPERTY(EditAnywhere, Category = "Preview")
	UMaterialInterface* GhostMaterialBase;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	UPROPERTY(EditAnywhere, Category = "Preview")
	FLinearColor ValidColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f); // 초록색

	UPROPERTY(EditAnywhere, Category = "Preview")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f); // 빨간색
};