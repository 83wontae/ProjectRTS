#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/UnitInterface.h"
#include "Global/ProjectRTSTypes.h"
#include "RtsUnitCharacter.generated.h"

UCLASS()
class PROJECTRTS_API ARtsUnitCharacter : public ACharacter, public IUnitInterface
{
    GENERATED_BODY()

public:
    ARtsUnitCharacter();

protected:
    virtual void BeginPlay() override;

    /** 에디터 상에서의 시각적 동기화를 위한 Construction Script */
    virtual void OnConstruction(const FTransform& Transform) override;

    /** EquipComponent의 데이터 갱신 알림을 처리하는 콜백 함수 */
    UFUNCTION()
    void HandleUnitBodyUpdate(const FST_Unit& UnitData);

public:
    // --- IUnitInterface 구현 ---
    virtual int32 GetFaction_Implementation() const override;

    /** 유닛의 데이터를 강제 업데이트 (생성 및 데이터 변경 시 호출) */
    UFUNCTION(BlueprintCallable, Category = "RTS|Unit")
    void InitializeUnit();

    /** 실제 메시 컴포넌트의 SkeletalMesh를 교체하는 시각적 실행 함수 */
    void UpdateArmorMesh(EEquipType Type, class USkeletalMesh* NewMesh);

public:
    // --- 핵심 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UTeamSysComponent* TeamComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStateComponent* StateComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UEquipComponent* EquipComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkillComponent* SkillComp;

    // --- 파츠 메시 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
    class USkeletalMeshComponent* HeadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
    class USkeletalMeshComponent* HorseMesh;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Data")
    FName UnitRowName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS|Selection")
    class UDecalComponent* SelectionDecal;
};