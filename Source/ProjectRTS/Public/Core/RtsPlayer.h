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

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 컨트롤러에서 혹시 참조할 일이 있을 수 있으므로 public 혹은 Getter 제공
	class UInputAction* GetMoveAction() const { return MoveAction; }

protected:
	virtual void BeginPlay() override;

	virtual void PawnClientRestart() override;

	/** IMC_Global_Nav: 카메라 이동 등 공용 입력 */
	UPROPERTY(EditAnywhere, Category = "RTS|Input")
	class UInputMappingContext* GlobalNavContext;

	/** 2. 입력 액션 (에디터 할당) */
	UPROPERTY(EditAnywhere, Category = "RTS|Input")
	class UInputAction* MoveAction;

public:
	virtual void Tick(float DeltaTime) override;

	/** 이동 처리 함수 */
	void Move(const FInputActionValue& Value);

protected:

	/** 이동 속도 */
	UPROPERTY(EditAnywhere, Category = "RTS|Movement")
	float MovementSpeed = 100.0f;


};