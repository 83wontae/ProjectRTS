// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h" // 필수: 입력 값을 받기 위해 필요
#include "RtsPlayer.generated.h"

UCLASS()
class PROJECTRTS_API ARtsPlayer : public APawn
{
	GENERATED_BODY()

public:
	ARtsPlayer();

protected:
	virtual void BeginPlay() override;

	// 블루프린트의 'ReceiveControllerChanged'와 유사한 역할을 하는 함수
	virtual void PawnClientRestart() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** 입력 설정 (에디터에서 할당) */
	UPROPERTY(EditAnywhere, Category = "RTS|Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "RTS|Input")
	class UInputAction* MoveAction;

	/** 이동 속도 */
	UPROPERTY(EditAnywhere, Category = "RTS|Movement")
	float MovementSpeed = 100.0f;

	/** 이동 처리 함수 */
	void Move(const FInputActionValue& Value);

};