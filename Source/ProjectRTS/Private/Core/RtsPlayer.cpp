// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RtsPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h" // Character 캐스팅을 위해 필요

ARtsPlayer::ARtsPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARtsPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			// 자기 자신의 Move 함수에 직접 바인딩
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARtsPlayer::Move);
		}
	}
}

void ARtsPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void ARtsPlayer::PawnClientRestart()
{
	Super::PawnClientRestart();

	// 1. 컨트롤러가 없으면 더 볼 것도 없이 리턴
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// 2. 서브시스템이나 컨텍스트가 없으면 리턴
	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem || !GlobalNavContext) return;

	// 3. 모든 가드를 통과했을 때만 로직 실행 (들여쓰기 깊이 0)
	Subsystem->AddMappingContext(GlobalNavContext, 0);
}

// 3. 이동 로직 (이미지의 'Movement Input' 부분)
void ARtsPlayer::Move(const FInputActionValue& Value)
{
	// 벡터2D 값 추출 (X: Left/Right, Y: Forward/Backward)
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (MovementVector.Y != 0.0f)
	{
		AddActorWorldOffset(GetActorForwardVector() * MovementVector.Y * MovementSpeed * GetWorld()->GetDeltaSeconds());
	}
	if (MovementVector.X != 0.0f)
	{
		AddActorWorldOffset(GetActorRightVector() * MovementVector.X * MovementSpeed * GetWorld()->GetDeltaSeconds());
	}
}

void ARtsPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
