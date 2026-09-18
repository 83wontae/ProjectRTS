// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ShowRoom.h"
#include "Global/RtsGameSettings.h"
#include "Components/EquipComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PostProcessComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/RecruitmentComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"

AShowRoom::AShowRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BodyMesh);

	HorseMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HorseMesh"));
	HorseMesh->SetupAttachment(BodyMesh);

	EquipComp = CreateDefaultSubobject<UEquipComponent>(TEXT("EquipComponent"));

	// 스프링암
	CaptureSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CaptureSpringArm"));
	CaptureSpringArm->SetupAttachment(RootComponent);
	CaptureSpringArm->TargetArmLength = 220.0f;
	CaptureSpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	CaptureSpringArm->SetRelativeRotation(FRotator(-5.0f, 0.0f, 0.0f));
	CaptureSpringArm->bDoCollisionTest = false;

	// 카메라 세팅
	SceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComp"));
	SceneCaptureComp->SetupAttachment(CaptureSpringArm, USpringArmComponent::SocketName);
	SceneCaptureComp->FOVAngle = 45.0f;
	SceneCaptureComp->bCaptureEveryFrame = true;
	SceneCaptureComp->bCaptureOnMovement = false;
	SceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	// 잔상/고스팅 차단 플래그
	SceneCaptureComp->ShowFlags.SetTemporalAA(false);
	SceneCaptureComp->ShowFlags.SetAntiAliasing(true);
	SceneCaptureComp->ShowFlags.SetMotionBlur(false);

	// 월드 간섭광 차단
	SceneCaptureComp->ShowFlags.SetFog(false);
	SceneCaptureComp->ShowFlags.SetAtmosphere(false);
	SceneCaptureComp->ShowFlags.SetSkyLighting(false);
	SceneCaptureComp->ShowFlags.SetGlobalIllumination(false);

	// ★ 1. 노출을 Manual 대신 '기본 자동 노출'로 풀거나 Bias를 넉넉하게 보정
	SceneCaptureComp->PostProcessSettings.bOverride_AutoExposureMethod = true;
	SceneCaptureComp->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Basic; // 자동 밝기 적응
	SceneCaptureComp->PostProcessSettings.bOverride_AutoExposureBias = true;
	SceneCaptureComp->PostProcessSettings.AutoExposureBias = 1.5f; // 화사하게 보정

	// 4. 조명 세팅 (거리를 캐릭터 쪽으로 조금 더 가깝게 조정하여 광량 확보)
	FakeSunLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FakeSunLight"));
	FakeSunLight->SetupAttachment(RootComponent);
	FakeSunLight->SetRelativeLocation(FVector(300.0f, 250.0f, 400.0f)); // 거리 축소 (감쇠 완화)
	FakeSunLight->SetRelativeRotation(FRotator(-40.0f, -140.0f, 0.0f));
	FakeSunLight->Intensity = 30000.0f;
	FakeSunLight->InnerConeAngle = 30.0f;
	FakeSunLight->OuterConeAngle = 55.0f;
	FakeSunLight->AttenuationRadius = 1500.0f;
	FakeSunLight->SetCastShadows(true);
	FakeSunLight->bCastVolumetricShadow = false;

	// 라이팅 채널 1 설정
	FakeSunLight->LightingChannels.bChannel0 = false;
	FakeSunLight->LightingChannels.bChannel1 = true;
	FakeSunLight->LightingChannels.bChannel2 = false;

	// 보조광
	FillLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(RootComponent);
	FillLight->SetRelativeLocation(FVector(200.0f, -250.0f, 200.0f));
	FillLight->SetRelativeRotation(FRotator(-25.0f, 135.0f, 0.0f));
	FillLight->Intensity = 10000.0f;
	FillLight->InnerConeAngle = 35.0f;
	FillLight->OuterConeAngle = 60.0f;
	FillLight->AttenuationRadius = 1200.0f;
	FillLight->SetCastShadows(false);

	FillLight->LightingChannels.bChannel0 = false;
	FillLight->LightingChannels.bChannel1 = true;
	FillLight->LightingChannels.bChannel2 = false;

	// 메시 채널 1 적용
	ApplyShowroomLightingChannel(BodyMesh);
	ApplyShowroomLightingChannel(HeadMesh);
	ApplyShowroomLightingChannel(HorseMesh);
}

void AShowRoom::ApplyShowroomLightingChannel(UPrimitiveComponent* TargetComp)
{
	if (TargetComp)
	{
		TargetComp->LightingChannels.bChannel0 = false;
		TargetComp->LightingChannels.bChannel1 = true;
		TargetComp->LightingChannels.bChannel2 = false;
	}
}

void AShowRoom::UpdateArmorMesh(EEquipType Type, USkeletalMesh* NewMesh)
{
	switch (Type)
	{
	case EEquipType::Head:
		if (HeadMesh) HeadMesh->SetSkeletalMesh(NewMesh);
		break;
	case EEquipType::Body:
		if (BodyMesh) BodyMesh->SetSkeletalMesh(NewMesh);
		break;
	case EEquipType::Horse:
		if (HorseMesh)
		{
			HorseMesh->SetSkeletalMesh(NewMesh);
			HorseMesh->SetVisibility(NewMesh != nullptr);
		}
		break;
	default:
		break;
	}
}

void AShowRoom::HandleUnitBodyUpdate(const FST_Unit& UnitData)
{
	UpdateArmorMesh(EEquipType::Head, UnitData.HeadPart);
	UpdateArmorMesh(EEquipType::Body, UnitData.BodyPart);
}

void AShowRoom::AddCharacterRotation(float YawDelta)
{
	if (BodyMesh)
	{
		BodyMesh->AddRelativeRotation(FRotator(0.0f, YawDelta, 0.0f));
		RefreshCapture();
	}
}

void AShowRoom::RefreshCapture()
{
	if (SceneCaptureComp)
	{
		SceneCaptureComp->CaptureScene();
	}
}

void AShowRoom::UpdateUnitByUniqueId(const FGuid& UniqueId)
{
	if (!UniqueId.IsValid()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC) return;

	URecruitmentComponent* RecruitComp = PC->FindComponentByClass<URecruitmentComponent>();
	if (!RecruitComp) return;

	FST_RuntimeUnitInfo Info;
	if (!RecruitComp->TryGetActiveUnit(UniqueId, Info)) return;

	// 기본 유닛 데이터로 본체 메시 교체
	UnitRowName = Info.UnitRowName;

	UDataTable* UnitTable = RtsSettings::GetUnitTable(this);
	if (UnitTable && !UnitRowName.IsNone())
	{
		const FST_Unit* UnitData = UnitTable->FindRow<FST_Unit>(UnitRowName, TEXT("ShowRoom_UpdateUnitById"));
		if (UnitData)
		{
			HandleUnitBodyUpdate(*UnitData);
		}
	}

	// 장비 정보 적용
	if (EquipComp)
	{
		if (!Info.EquipHandR.IsNone()) EquipComp->EquipToWeapon(Info.EquipHandR);
		if (!Info.EquipHandL.IsNone()) EquipComp->EquipToWeapon(Info.EquipHandL);
		if (!Info.EquipHead.IsNone()) EquipComp->EquipArmorByName(Info.EquipHead, EEquipType::Head);
		if (!Info.EquipBody.IsNone()) EquipComp->EquipArmorByName(Info.EquipBody, EEquipType::Body);
	}

	// 캡처 갱신
	RefreshCapture();
}

USkeletalMeshComponent* AShowRoom::GetTargetAttachMesh_Implementation() const
{
	return BodyMesh;
}

#if WITH_EDITOR
void AShowRoom::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AShowRoom, UnitRowName))
	{
		UDataTable* UnitTable = RtsSettings::GetUnitTable(GetOwner());
		if (UnitTable && !UnitRowName.IsNone())
		{
			const FST_Unit* UnitData = UnitTable->FindRow<FST_Unit>(UnitRowName, TEXT("ShowRoom_EditorUpdate"));
			if (UnitData)
			{
				HandleUnitBodyUpdate(*UnitData);
			}
		}
	}
}
#endif

void AShowRoom::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UDataTable* UnitTable = RtsSettings::GetUnitTable(GetOwner());
	if (!UnitTable) return;

	if (BodyMesh)
	{
		if (HeadMesh) HeadMesh->SetLeaderPoseComponent(BodyMesh);
		if (HorseMesh) HorseMesh->SetLeaderPoseComponent(BodyMesh);
	}

	if (!UnitRowName.IsNone())
	{
		const FST_Unit* UnitData = UnitTable->FindRow<FST_Unit>(UnitRowName, TEXT("ShowRoom_OnConstruction"));
		if (UnitData)
		{
			HandleUnitBodyUpdate(*UnitData);
		}
	}

	// ★ 에디터 뷰포트에서도 캡처가 갱신되도록 호출
	RefreshCapture();
}

void AShowRoom::BeginPlay()
{
	Super::BeginPlay();

	if (SceneCaptureComp)
	{
		// ★ 핵심: ShowOnlyActors 대신 '컴포넌트 리스트'를 사용하여 캐릭터 메시와 전용 라이트만 명확히 등록
		SceneCaptureComp->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		SceneCaptureComp->ShowOnlyComponents.Empty();

		// 1. 캐릭터 메시 등록
		if (BodyMesh) SceneCaptureComp->ShowOnlyComponent(BodyMesh);
		if (HeadMesh) SceneCaptureComp->ShowOnlyComponent(HeadMesh);
		if (HorseMesh) SceneCaptureComp->ShowOnlyComponent(HorseMesh);

		// 2. 캡처 대상에 전용 라이트도 함께 보여야 하므로 쇼룸 액터 본체 등록
		SceneCaptureComp->ShowOnlyActors.Empty();
		SceneCaptureComp->ShowOnlyActors.Add(this);

		SceneCaptureComp->CaptureScene();
	}
}

void AShowRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}