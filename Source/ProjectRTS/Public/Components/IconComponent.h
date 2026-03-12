// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "IconComponent.generated.h"


UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTRTS_API UIconComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UIconComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** 미니맵 HUD에서 사용할 아이콘 텍스처 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Icon")
	class UTexture2D* MinimapIcon;

	/** 월드 공간에서 시각적 표시로 사용할 메시 (필요 시 에디터에서 설정) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Icon")
	class UStaticMeshComponent* WorldMarkerMesh;

	/** 진영에 따른 색상 업데이트 */
	void UpdateIconColorByFaction(int32 Faction);
};