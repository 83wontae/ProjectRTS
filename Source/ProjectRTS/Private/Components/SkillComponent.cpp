// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SkillComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interface/UnitInterface.h"
#include "Core/RtsGameState.h"
#include "Components/StateComponent.h"
#include "Components/EquipComponent.h"
#include "Interface/ProjectileInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DebugWidgetComponent.h"

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
		UE_LOG(LogTemp, Error, TEXT("[%s] FindBestTarget: DetectionRange is 0.0f!"), *OwnerChar->GetName());
		ScanRange = 800.0f;
	}

	FVector CenterPos = OwnerChar->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	// 만약 건물 채널이 필요하다면 여기에 추가하세요.

	TArray<AActor*> OutActors;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), CenterPos, ScanRange, ObjectTypes, nullptr, { OwnerChar }, OutActors);

	// --- [로그 추가: 주변 감지 결과] ---
	if (GEngine)
	{
		uint64 Key = (uint64)OwnerChar->GetUniqueID() + 30; // 스탯/스킬 로그와 겹치지 않게 설정
		FString DebugMsg = FString::Printf(TEXT("[%s] Scanning... Range: %.1f | Found Actors: %d"),
			*OwnerChar->GetName(), ScanRange, OutActors.Num());
		GEngine->AddOnScreenDebugMessage(Key, 1.5f, FColor::Silver, DebugMsg);
	}

	AActor* BestTarget = nullptr;
	float MinDist = ScanRange;

	for (AActor* Actor : OutActors)
	{
		if (Actor)
		{
			bool bIsEnemy = GS->IsEnemyByActor(OwnerChar, Actor);
			bool bIsDead = IUnitInterface::Execute_IsDeath(Actor);

			// --- [상세 로그: 각 액터 판정 이유] ---
			UE_LOG(LogTemp, Verbose, TEXT("Checking Actor: %s | IsEnemy: %s | IsDead: %s"),
				*Actor->GetName(), bIsEnemy ? TEXT("True") : TEXT("False"), bIsDead ? TEXT("True") : TEXT("False"));

			// GameState를 통한 진영 확인 및 인터페이스를 통한 사망 확인
			if (bIsEnemy && !bIsDead)
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

	// --- [최종 로그: 타겟 선정 결과] ---
	if (GEngine)
	{
		uint64 Key = (uint64)OwnerChar->GetUniqueID() + 31;
		FColor LogColor = BestTarget ? FColor::Green : FColor::Red;
		FString ResultMsg = BestTarget ?
			FString::Printf(TEXT("Best Target Found: %s (Dist: %.1f)"), *BestTarget->GetName(), MinDist) :
			TEXT("No Valid Enemy in Range");

		GEngine->AddOnScreenDebugMessage(Key, 2.0f, LogColor, ResultMsg);
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
	FString ActorName = OwnerChar ? OwnerChar->GetName() : TEXT("Unknown");
	uint64 Key = (uint64)GetOwner()->GetUniqueID() + 50; // UseSkill 전용 키

	// 1. [가드] 소유자 유효성 체크
	if (!OwnerChar) return false;

	// 2. [가드] 이미 공격 중인지 체크 (중복 실행 방지)
	if (bIsAttacking)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(Key, 1.0f, FColor::Silver,
			FString::Printf(TEXT("[%s] UseSkill: Already Attacking..."), *ActorName));
		return false;
	}

	// 3. 스킬 이름 결정 (입력값이 없으면 기본 공격)
	if (SkillName.IsNone())
	{
		SkillName = GetDefaultAttackSkillName();
	}

	// 4. 데이터 조회 및 Fallback 처리
	const FST_Skill* SkillData = m_SkillMap.Find(SkillName);

	if (!SkillData && m_SkillMap.Num() > 0)
	{
		// [수정] 어떤 이름을 찾으려 했는지 로그에 추가
		FName FailedName = SkillName;

		TArray<FName> Keys;
		m_SkillMap.GetKeys(Keys);
		SkillName = Keys[0];
		SkillData = m_SkillMap.Find(SkillName);

		UE_LOG(LogTemp, Warning, TEXT("[%s] Skill [%s] not found! Falling back to: [%s]"),
			*ActorName,
			*FailedName.ToString(), // 이 녀석이 None인지 다른 이름인지 확인해야 합니다.
			*SkillName.ToString());
	}

	// 5. [가드] 최종 데이터 유효성 검사
	if (!SkillData || !SkillData->SkillAnim)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] UseSkill Failed: Invalid SkillData or Anim for [%s]"), *ActorName, *SkillName.ToString());
		return false;
	}

	// --- [성공 로직: 여기서부터 실행] ---
	CurrentActiveSkillName = SkillName;

	// 타겟 방향으로 즉시 회전
	if (InTarget)
	{
		FVector Dir = InTarget->GetActorLocation() - OwnerChar->GetActorLocation();
		Dir.Z = 0.f;
		OwnerChar->SetActorRotation(FRotationMatrix::MakeFromX(Dir).Rotator());
	}

	// 애니메이션 실행
	bIsAttacking = true;
	float Duration = OwnerChar->PlayAnimMontage(SkillData->SkillAnim);

	// 최종 성공 로그 (화면: 녹색 / 출력창: Warning으로 강조)
	if (GEngine)
	{
		FString TargetName = InTarget ? InTarget->GetName() : TEXT("None");
		GEngine->AddOnScreenDebugMessage(Key, 2.0f, FColor::Green,
			FString::Printf(TEXT("[%s] UseSkill: %s -> %s"), *ActorName, *SkillName.ToString(), *TargetName));
	}

	OnSkillStarted.Broadcast(SkillName);

	// --- [추가] 활성화된 스킬 표시를 위해 위젯 갱신 ---
	UpdateDebugWidget();

	return true;
}

bool USkillComponent::ExecuteBestAttack(AActor* InTarget)
{
	// 1. 현재 상태에 맞는 최적의 스킬 이름 가져오기
	FName BestSkillName = GetDefaultAttackSkillName();

	// 2. 결정된 스킬로 실행 요청
	return UseSkill(BestSkillName, InTarget);
}

FName USkillComponent::GetDefaultAttackSkillName()
{
	if (!OwnerChar) return NAME_None;

	UEquipComponent* EquipComp = OwnerChar->FindComponentByClass<UEquipComponent>();
	if (!EquipComp || !EquipComp->WeaponTable) return NAME_None;

	// 1. 메인 무기 이름 결정 (오른손 우선, 없으면 왼손)
	FName MainWeaponName = !EquipComp->m_RightWeaponName.IsNone() ?
		EquipComp->m_RightWeaponName : EquipComp->m_LeftWeaponName;

	// 2. [가드] 양손 모두 무기가 없는 경우
	if (MainWeaponName.IsNone())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[%s] No weapon found in either hand."), *OwnerChar->GetName());
		return NAME_None;
	}

	// 3. 데이터 테이블에서 스킬 이름 찾기
	const FST_Weapon* WeaponData = EquipComp->WeaponTable->FindRow<FST_Weapon>(MainWeaponName, TEXT(""));
	if (!WeaponData) return NAME_None;

	// 4. 탑승 여부에 따른 최종 스킬 반환
	bool bIsRiding = EquipComp->IsRideState();
	return bIsRiding ? WeaponData->SkillNameRide : WeaponData->SkillName;
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

		// --- [추가] 스킬이 추가될 때마다 디버그 위젯 갱신 ---
		UpdateDebugWidget();

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

void USkillComponent::ExecuteSkillEffect()
{
	// 1. [가드 클로저] 데이터 유효성 검사
	if (!OwnerChar || CurrentActiveSkillName.IsNone()) return;

	const FST_Skill* SkillData = m_SkillMap.Find(CurrentActiveSkillName);
	if (!SkillData) return;

	// 2. 타겟 및 공격력 정보 확보
	UStateComponent* StateComp = OwnerChar->FindComponentByClass<UStateComponent>();
	AActor* Target = StateComp ? StateComp->GetAggroTarget() : nullptr;
	double FinalAtk = StateComp ? StateComp->GetTotalAttack() : 0.0;

	FString ActorName = OwnerChar->GetName();
	FString TargetName = Target ? Target->GetName() : TEXT("No Target");
	uint64 KeyBase = (uint64)GetOwner()->GetUniqueID();

	// --- [로그 1] 스킬 실행 기본 정보 (노란색) ---
	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("[%s] <Execute> %s (Target: %s)"),
			*ActorName, *CurrentActiveSkillName.ToString(), *TargetName);
		GEngine->AddOnScreenDebugMessage(KeyBase + 10, 2.0f, FColor::Yellow, Msg);
	}

	// 3. 스킬 타입에 따른 분기 처리
	if (SkillData->SkillType == ESkillType::Projectile)
	{
		UEquipComponent* EquipComp = OwnerChar->FindComponentByClass<UEquipComponent>();
		// 앞서 만든 GetMainMuzzleLocation 활용 (파라미터 없음)
		FVector MuzzleLoc = EquipComp ? EquipComp->GetMainMuzzleLocation() : OwnerChar->GetActorLocation();

		// --- [로그 2] 발사체 정보 (주황색) ---
		if (GEngine)
		{
			FString ProjMsg = FString::Printf(TEXT(">> Projectile Spawned at: %s"), *MuzzleLoc.ToCompactString());
			GEngine->AddOnScreenDebugMessage(KeyBase + 11, 2.0f, FColor::Orange, ProjMsg);
		}

		// --- 원거리: 발사체 생성 ---
		SpawnProjectile(SkillData, Target, MuzzleLoc);
	}
	else
	{
		// --- [로그 2] 근접 타격 정보 (빨간색) ---
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(KeyBase + 11, 2.0f, FColor::Red, TEXT(">> Melee Processing..."));
		}

		// --- 근접: 즉시 히트 처리 ---
		ProcessMeleeHit(SkillData, Target);
	}
}

void USkillComponent::SpawnProjectile(const FST_Skill* SkillData, AActor* Target, FVector MuzzleLoc)
{
	if (!SkillData->ProjectileClass || !OwnerChar) return;

	FRotator SpawnRotation = OwnerChar->GetActorRotation();

	if (Target)
	{
		SpawnRotation = (Target->GetActorLocation() - MuzzleLoc).Rotation();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerChar;
	SpawnParams.Instigator = OwnerChar;

	// 발사체 스폰 (Deferred Spawn 권장)
	// 발사체 클래스 내부에서 타겟 정보와 데미지를 받아 처리하도록 설계합니다.
	AActor* Projectile = GetWorld()->SpawnActor<AActor>(SkillData->ProjectileClass, MuzzleLoc, SpawnRotation, SpawnParams);

	if (Projectile->Implements<UProjectileInterface>())
	{
		double FinalAtk = OwnerChar->FindComponentByClass<UStateComponent>()->GetTotalAttack();

		// 화살이든, 레이저든, 검기든 똑같이 이 함수만 호출하면 끝!
		IProjectileInterface::Execute_SetupProjectile(Projectile, OwnerChar, Target, FinalAtk);
	}
}

void USkillComponent::UpdateDebugWidget()
{
	// 1. [가드] 디버그 컴포넌트 찾기
	UDebugWidgetComponent* DebugComp = GetOwner()->FindComponentByClass<UDebugWidgetComponent>();
	if (!DebugComp) return;

	// 2. 출력할 문자열 리스트 생성
	TArray<FString> SkillLogs;
	SkillLogs.Add(FString::Printf(TEXT("=== Owned Skills (%d) ==="), m_SkillMap.Num()));

	// 3. 맵을 순회하며 스킬 이름 담기
	for (auto& Elem : m_SkillMap)
	{
		FString ActiveMark = (Elem.Key == CurrentActiveSkillName) ? TEXT("[Active] ") : TEXT("- ");
		SkillLogs.Add(ActiveMark + Elem.Key.ToString());
	}

	// 4. 위젯 컴포넌트에 전달
	DebugComp->UpdateLogList(SkillLogs);
}

void USkillComponent::ProcessMeleeHit(const FST_Skill* SkillData, AActor* Target)
{
	// 1. [가드] 타겟 유효성 및 생존 확인
	if (!Target || IUnitInterface::Execute_IsDeath(Target)) return;

	// 2. [가드] 내 상태 및 공격력 확인
	UStateComponent* MyState = OwnerChar->FindComponentByClass<UStateComponent>();
	if (!MyState) return;

	double Damage = MyState->GetTotalAttack();

	// 3. 언리얼 표준 데미지 전달
	// 파라미터: 타겟, 데미지량, 가해자 컨트롤러, 가해자 액터, 데미지 타입 클래스
	UGameplayStatics::ApplyDamage(
		Target,
		(float)Damage,
		OwnerChar->GetController(),
		OwnerChar,
		nullptr // 필요 시 UDamageType 자식 클래스 지정 가능
	);

	// 시각적 확인용 로그
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
			FString::Printf(TEXT("[%s] ApplyDamage to [%s]: %.1f"), *OwnerChar->GetName(), *Target->GetName(), Damage));
	}
}

void USkillComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}
