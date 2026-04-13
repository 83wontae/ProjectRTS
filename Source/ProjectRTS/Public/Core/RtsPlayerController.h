// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RtsPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTRTS_API ARtsPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "RtsPlayerController")
	bool IsInSideDragRect(FVector LocationToTest, FVector2D Start, FVector2D End);

	UFUNCTION(BlueprintPure, Category = "RtsPlayerController")
	void GetActorsInsideDragRect(TSubclassOf<UInterface> TargetInterface, FVector2D Start, FVector2D End, TArray<AActor*>& OutActors);

public:
    // 현재 생성된 고스트 건물을 가져오거나 외부에서 강제로 지울 때 사용
    UFUNCTION(BlueprintPure, Category = "RTS|Placement")
    class APreviewBuilding* GetCurrentPreviewBuilding() const { return CurrentPreviewActor; }

    UFUNCTION(BlueprintCallable, Category = "RTS|Placement")
    void StartPlacementMode(FName BuildingRowName);

    // 다른 이벤트(UI 클릭, ESC 등) 발생 시 호출하여 고스트 제거
    UFUNCTION(BlueprintCallable, Category = "RTS|Placement")
    void ClearCurrentPreview();

protected:
    // 배치할 고스트 건물의 베이스 클래스 (BP에서 할당)
    UPROPERTY(EditAnywhere, Category = "RTS|Settings")
    TSubclassOf<APreviewBuilding> PreviewBuildingClass;

    // 현재 월드에 존재하는 고스트 객체 포인터
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Placement")
    class APreviewBuilding* CurrentPreviewActor;
	
	/** 현재 선택된 유닛 액터들의 배열 */
	UPROPERTY(BlueprintReadWrite, Category = "RTS|Selection")
	TArray<AActor*> SelectedActors;

public:
	/** 모든 유닛의 선택을 해제하고 배열을 비우는 함수 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void UnSelectedAllUnits();

	/** 새로운 액터들을 선택 목록에 담는 함수 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void DoSelectUnits(const TArray<AActor*>& InActors);

	/** 마우스 클릭 지점의 유닛 하나를 선택 목록에 추가하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Selection")
	void DoSelectUnitSingle();

	/** 선택된 유닛들을 목표 지점으로 이동시킴 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Movement")
	void MoveUnits(FVector Goal);
};
