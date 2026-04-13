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

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	/** 초기화 함수 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Preview")
	void SetupPreviewBuilding(FName InRowName);

	UFUNCTION(BlueprintCallable, Category = "RTS|Preview")
	void SetPreviewMesh(class UStaticMesh* NewMesh);

	void CreateDynamicMaterial();
	void SetValidPlacement(bool bIsValid);

public:
	// meta = (ExposeOnSpawn = true)를 추가하여 스폰 노드에서 바로 보이게 합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS", meta = (ExposeOnSpawn = true))
	FName BuildingRowName;

protected:
	UPROPERTY(VisibleAnywhere, Category = "RTS")
	class UPlacementComponent* PlacementComp;

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	UStaticMeshComponent* MeshComponent;

	// 바닥 그리드를 표시할 데칼 컴포넌트 추가
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	class UDecalComponent* GridDecalComp;

	UPROPERTY(EditAnywhere, Category = "Preview")
	class UMaterialInterface* GhostMaterialBase;

	// 고스트 메쉬용 다이내믹 머티리얼
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;

	// 데칼 그리드용 다이내믹 머티리얼
	UPROPERTY()
	class UMaterialInstanceDynamic* DecalDynamicMaterial;

	UPROPERTY(EditAnywhere, Category = "Preview")
	FLinearColor ValidColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);

	UPROPERTY(EditAnywhere, Category = "Preview")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);
};