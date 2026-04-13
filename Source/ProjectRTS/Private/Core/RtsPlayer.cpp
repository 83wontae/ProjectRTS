// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RtsPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h" // Character 캐스팅을 위해 필요

ARtsPlayer::ARtsPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARtsPlayer::BeginPlay()
{
	Super::BeginPlay();
}

// 1. Mapping Context 추가 (이미지의 'Add Input Mapping' 부분)
void ARtsPlayer::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				// 이미지와 동일하게 Priority 0으로 설정
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

// 2. 입력 바인딩
void ARtsPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			// Triggered 상태일 때 Move 함수 호출
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARtsPlayer::Move);
		}
	}
}

// 3. 이동 로직 (이미지의 'Movement Input' 부분)
void ARtsPlayer::Move(const FInputActionValue& Value)
{
	// 벡터2D 값 추출 (X: Left/Right, Y: Forward/Backward)
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Forward/Backward 이동
		if (MovementVector.Y != 0.0f)
		{
			FVector Direction = GetActorForwardVector();
			AddActorWorldOffset(Direction * MovementVector.Y * MovementSpeed * GetWorld()->GetDeltaSeconds());
		}

		// Left/Right 이동
		if (MovementVector.X != 0.0f)
		{
			FVector Direction = GetActorRightVector();
			AddActorWorldOffset(Direction * MovementVector.X * MovementSpeed * GetWorld()->GetDeltaSeconds());
		}
	}
}

void ARtsPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
