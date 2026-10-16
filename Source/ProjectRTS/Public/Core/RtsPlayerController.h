// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Misc/Guid.h"
#include "Global/ProjectRTSTypes.h"
#include "Misc/Guid.h"

UENUM(BlueprintType)
enum class EFormationType : uint8
{
	Grid    UMETA(DisplayName = "Grid"),
	Circle  UMETA(DisplayName = "Circle"),
	Line    UMETA(DisplayName = "Line")
};
#include "RtsPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTRTS_API ARtsPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ARtsPlayerController();

	virtual void Tick(float DeltaSeconds) override;

	/** Dynamic reformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	bool bEnableDynamicReformation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	float ReformationInterval = 1.0f; // seconds

protected:
	bool bHasActiveMove = false;
	FVector CurrentMoveGoal = FVector::ZeroVector;
	float TimeSinceLastReform = 0.0f;

	/** 입력 모드 전환 함수 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Input")
	void SetRtsInputMode(ERtsInputMode NewMode);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** IMC_Unit_Tactical: 유닛 지휘 입력 */
	UPROPERTY(EditAnywhere, Category = "RTS|Input")
	class UInputMappingContext* TacticalContext;

	/** IMC_Build_Placement: 건물 배치 입력 */
	UPROPERTY(EditAnywhere, Category = "RTS|Input")
	class UInputMappingContext* PlacementContext;

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

	/** 배치 모드에서 클릭 시 호출될 함수 */
	UFUNCTION(BlueprintCallable, Category = "RTS|Placement")
	void ConfirmPlacement();

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

	ERtsInputMode CurrentInputMode;

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

	/** Set leader by UniqueId (will select leader actor if found) */
	UFUNCTION(BlueprintCallable, Category = "RTS|Formation")
	void SetLeaderByUniqueId(const FGuid& UniqueId);

	UFUNCTION(BlueprintCallable, Category = "RTS|Formation")
	void ClearLeader();

	UFUNCTION(BlueprintCallable, Category = "RTS|Formation")
	void SetLeaderToSelectedFirst();

	/** Formation settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	EFormationType FormationType = EFormationType::Grid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	float FormationSpacing = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	float FormationJitterRatio = 0.1f; // fraction of spacing

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	bool bUseLeaderFollower = false;

	/** If using leader-follower, leader UniqueId (optional). If invalid, first selected actor is leader. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	FGuid LeaderUniqueId;

	/** Per-actor current targets for dynamic reformation */
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<AActor>, FVector> CurrentUnitTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	float AcceptanceRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Formation")
	float StuckVelocityThreshold = 10.0f; // below this considered stuck
};
