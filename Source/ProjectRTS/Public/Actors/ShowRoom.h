// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/EquipableInterface.h"
#include "Global/ProjectRTSTypes.h"
#include "Misc/Guid.h"
#include "ShowRoom.generated.h"

class UEquipComponent;
class USceneComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class USceneCaptureComponent2D;
class USpotLightComponent;
class UPostProcessComponent;

UCLASS()
class PROJECTRTS_API AShowRoom : public AActor, public IEquipableInterface
{
	GENERATED_BODY()

public:
	AShowRoom();

public:
	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Scene")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UEquipComponent* EquipComp;

	// 캐릭터 부위별 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Appearance")
	USkeletalMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Appearance")
	USkeletalMeshComponent* HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Appearance")
	USkeletalMeshComponent* HorseMesh;

	// 카메라 시스템
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Capture")
	USpringArmComponent* CaptureSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Capture")
	USceneCaptureComponent2D* SceneCaptureComp;

	// 쇼룸 전용 스폿 조명 (태양광 모사 & 채널 1 격리)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Lighting")
	USpotLightComponent* FakeSunLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Showroom|Lighting")
	USpotLightComponent* FillLight;

	FName GetUnitRowName() const { return UnitRowName; }

	/** UI 드래그 시 캐릭터 메시만 회전 */
	UFUNCTION(BlueprintCallable, Category = "Showroom|Control")
	void AddCharacterRotation(float YawDelta);

	/** 수동 캡처 호출 */
	UFUNCTION(BlueprintCallable, Category = "Showroom|Capture")
	void RefreshCapture();

	/** UniqueId로 등록된 유닛 정보를 조회하여 쇼룸을 업데이트합니다. */
	UFUNCTION(BlueprintCallable, Category = "Showroom|Control")
	void UpdateUnitByUniqueId(const FGuid& UniqueId);

	// IEquipableInterface 구현
	virtual USkeletalMeshComponent* GetTargetAttachMesh_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data", meta = (ExposeOnSpawn = "true"))
	FName UnitRowName;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void UpdateArmorMesh(EEquipType Type, USkeletalMesh* NewMesh);
	void HandleUnitBodyUpdate(const FST_Unit& UnitData);

	/** 쇼룸 전용 라이팅 채널(Channel 1)을 일괄 적용하는 내부 헬퍼 */
	void ApplyShowroomLightingChannel(UPrimitiveComponent* TargetComp);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	virtual void Tick(float DeltaTime) override;
};