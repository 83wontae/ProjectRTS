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
};
