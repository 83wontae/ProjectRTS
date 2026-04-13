// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Global/ProjectRTSTypes.h" // FST_Building 구조체 포함 확인
#include "Building.generated.h"

UCLASS()
class PROJECTRTS_API ABuilding : public AActor
{
	GENERATED_BODY()

public:
	ABuilding();

protected:
	virtual void BeginPlay() override;

public:
	/** 데이터 테이블 정보를 바탕으로 건물을 초기화합니다. */
	void InitializeBuilding();

	// 스폰 시점에 RowName을 전달받을 수 있도록 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS", meta = (ExposeOnSpawn = true))
	FName BuildingRowName;

protected:
	/** 실제 건물의 외형을 담당하는 메시 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS")
	UStaticMeshComponent* MeshComponent;

	/** 건물 정보가 담긴 데이터 테이블 (에디터에서 할당) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS")
	class UDataTable* BuildingDataTable;
};