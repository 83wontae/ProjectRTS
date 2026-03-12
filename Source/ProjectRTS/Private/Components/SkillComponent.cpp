// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SkillComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interface/UnitInterface.h"
#include "Core/RtsGameState.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // 전투 루프를 BT에 맡기므로 틱을 끕니다.
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		if (OwnerChar->GetMesh() && OwnerChar->GetMesh()->GetAnimInstance())
		{
			OwnerChar->GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &USkillComponent::OnAttackMontageEnded);
		}
	}

	AddSkill(GetDefaultAttackSkillName());
}

AActor* USkillComponent::FindBestTargetInRange()
{
	if (!OwnerChar) return nullptr;

	ARtsGameState* GS = GetWorld()->GetGameState<ARtsGameState>();
	if (!GS) return nullptr;

	// 1. 인터페이스를 통해 DetectionRange 가져오기
	float ScanRange = IUnitInterface::Execute_GetDetectionRange(OwnerChar);

	// 2. 에러 로그 및 최소값 보정
	if (ScanRange <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] FindBestTarget: DetectionRange is 0.0f! 데이터 테이블 설정을 확인하세요."), *OwnerChar->GetName());
		ScanRange = 800.0f;
	}

	FVector CenterPos = OwnerChar->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> OutActors;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), CenterPos, ScanRange, ObjectTypes, nullptr, { OwnerChar }, OutActors);

	AActor* BestTarget = nullptr;
	float MinDist = ScanRange;

	for (AActor* Actor : OutActors)
	{
		if (Actor)
		{
			// GameState를 통한 진영 확인 및 인터페이스를 통한 사망 확인
			if (GS->IsEnemyByActor(OwnerChar, Actor) && !IUnitInterface::Execute_IsDeath(Actor))
			{
				float Dist = FVector::Dist(CenterPos, Actor->GetActorLocation());
				if (Dist < MinDist)
				{
					MinDist = Dist;
					BestTarget = Actor;
				}
			}
		}
	}
	return BestTarget;
}

bool USkillComponent::IsCanAttack(AActor* InTarget)
{
	if (!InTarget || !OwnerChar) return false;

	// 1. 대상의 생존 여부 확인 (Interface)
	if (InTarget->Implements<UUnitInterface>())
	{
		if (IUnitInterface::Execute_IsDeath(InTarget)) return false;
	}

	// 2. 소유자의 공격 사거리 확인 (Interface)
	float AttackRange = 0.0f;
	if (OwnerChar->Implements<UUnitInterface>())
	{
		AttackRange = IUnitInterface::Execute_GetAttackRange(OwnerChar);
	}

	// 3. 거리 계산 및 판정
	float Distance = FVector::Dist(OwnerChar->GetActorLocation(), InTarget->GetActorLocation());
	return Distance <= AttackRange;
}

bool USkillComponent::UseSkill(FName SkillName, AActor* InTarget)
{
	// 1. 초기 상태 체크 로그
	if (!OwnerChar)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] UseSkill Failed: OwnerChar is Null!"), *GetOwner()->GetName());
		return false;
	}

	if (bIsAttacking)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] UseSkill Skipped: Already Attacking."), *OwnerChar->GetName());
		return false;
	}

	// 2. 스킬 이름 결정 로그
	if (SkillName.IsNone())
	{
		SkillName = GetDefaultAttackSkillName();
		UE_LOG(LogTemp, Log, TEXT("[%s] UseSkill: SkillName was None. Defaulted to [%s]"), *OwnerChar->GetName(), *SkillName.ToString());
	}

	// 3. 맵 데이터 검색 로그
	const FST_Skill* SkillData = m_SkillMap.Find(SkillName);

	// Fallback 로직 진입 시 로그
	if (!SkillData && m_SkillMap.Num() > 0)
	{
		TArray<FName> Keys;
		m_SkillMap.GetKeys(Keys);
		FName FallbackName = Keys[0];
		UE_LOG(LogTemp, Warning, TEXT("[%s] UseSkill: Requested skill [%s] not found. Falling back to first available: [%s]"),
			*OwnerChar->GetName(), *SkillName.ToString(), *FallbackName.ToString());

		SkillName = FallbackName;
		SkillData = m_SkillMap.Find(SkillName);
	}

	// 4. 최종 실행 여부 판단 로그
	if (SkillData)
	{
		if (SkillData->SkillAnim)
		{
			// 타겟 방향 회전
			if (InTarget)
			{
				FVector Dir = InTarget->GetActorLocation() - OwnerChar->GetActorLocation();
				Dir.Z = 0.f;
				OwnerChar->SetActorRotation(FRotationMatrix::MakeFromX(Dir).Rotator());
				UE_LOG(LogTemp, Verbose, TEXT("[%s] UseSkill: Rotating towards Target [%s]"), *OwnerChar->GetName(), *InTarget->GetName());
			}

			// 애니메이션 실행
			bIsAttacking = true;
			float Duration = OwnerChar->PlayAnimMontage(SkillData->SkillAnim);

			UE_LOG(LogTemp, Warning, TEXT("[%s] UseSkill SUCCESS: Playing Montage [%s]. Duration: %.2f"),
				*OwnerChar->GetName(), *SkillName.ToString(), Duration);

			OnSkillStarted.Broadcast(SkillName);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] UseSkill FAILED: SkillData found for [%s], but SkillAnim is NULL!"),
				*OwnerChar->GetName(), *SkillName.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] UseSkill FAILED: Skill [%s] not found in m_SkillMap and no fallback possible."),
			*OwnerChar->GetName(), *SkillName.ToString());
	}

	return false;
}

FName USkillComponent::GetDefaultAttackSkillName()
{
	if (!OwnerChar) return NAME_None;

	// 1. 인터페이스를 통해 상태 확인
	bool bIsRiding = IUnitInterface::Execute_IsRiding(OwnerChar);

	// 사거리가 250 이상이면 원거리(Archer)로 판정하는 임시 로직
	float AttackRange = IUnitInterface::Execute_GetAttackRange(OwnerChar);
	bool bIsRanged = (AttackRange > 250.0f);

	// 2. 조합에 따른 스킬 이름 결정
	if (bIsRiding)
	{
		return bIsRanged ? FName("CavArcherAttack") : FName("CavAttack");
	}
	else
	{
		return bIsRanged ? FName("ArcherAttack") : FName("InfantryAttack");
	}
}

void USkillComponent::AddSkill(FName SkillName)
{
	// 1. 유효성 체크 (테이블이 없거나 이름이 비어있으면 종료)
	if (!SkillTable || SkillName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] AddSkill: SkillTable is null or SkillName is None."), *GetOwner()->GetName());
		return;
	}

	// 2. 데이터 테이블에서 스킬 데이터 찾기
	// SkillData.csv 형식이 FST_Skill 구조체와 매핑되어 있어야 합니다.
	FST_Skill* FoundData = SkillTable->FindRow<FST_Skill>(SkillName, TEXT(""));

	if (FoundData)
	{
		// 3. 맵에 추가 (이미 존재하면 업데이트됨)
		m_SkillMap.Add(SkillName, *FoundData);

		UE_LOG(LogTemp, Log, TEXT("[%s] Successfully added skill: %s"), *GetOwner()->GetName(), *SkillName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] AddSkill: Failed to find skill [%s] in SkillTable!"),
			*GetOwner()->GetName(), *SkillName.ToString());
	}
}

void USkillComponent::ClearSkills()
{
	m_SkillMap.Empty();
	m_CooldownMap.Empty();
}

void USkillComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [로그 추가] 몽타주 종료 시점 확인
	UE_LOG(LogTemp, Warning, TEXT("[%s] OnAttackMontageEnded: Montage [%s] Finished. Interrupted: %s"),
		*OwnerChar->GetName(), *Montage->GetName(), bInterrupted ? TEXT("True") : TEXT("False"));

	bIsAttacking = false;

	// [로그 추가] 상태 해제 확인
	UE_LOG(LogTemp, Log, TEXT("[%s] bIsAttacking set to FALSE."), *OwnerChar->GetName());
}
