#include "Core/RtsUnitCharacter.h"
#include "Components/TeamSysComponent.h"
#include "Components/StateComponent.h"
#include "Components/EquipComponent.h"
#include "Components/SkillComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/DecalComponent.h"

ARtsUnitCharacter::ARtsUnitCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // RTS 기본 설정
    bUseControllerRotationYaw = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bRequestedMoveUseAcceleration = true; // 가속도 활성화
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
    }

    // 메시 컴포넌트 생성 및 부착
    HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
    HeadMesh->SetupAttachment(GetMesh());

    HorseMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HorseMesh"));
    HorseMesh->SetupAttachment(GetMesh());

    // 기능 컴포넌트 생성
    TeamComp = CreateDefaultSubobject<UTeamSysComponent>(TEXT("TeamComponent"));
    StateComp = CreateDefaultSubobject<UStateComponent>(TEXT("StateComponent"));
    EquipComp = CreateDefaultSubobject<UEquipComponent>(TEXT("EquipComponent"));
    SkillComp = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComponent"));

    SelectionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectionDecal"));
    SelectionDecal->SetupAttachment(RootComponent);
    SelectionDecal->SetRelativeRotation(FRotator(90.0f, 0.0f, -90.0f));
    SelectionDecal->SetVisibility(false);
}

void ARtsUnitCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 런타임: 장착 컴포넌트의 데이터 변경 이벤트를 구독합니다.
    if (EquipComp)
    {
        EquipComp->OnUpdateUnitBody.AddDynamic(this, &ARtsUnitCharacter::HandleUnitBodyUpdate);
    }

    InitializeUnit();
}

void ARtsUnitCharacter::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 에디터: 애니메이션 포즈 동기화
    if (GetMesh())
    {
        if (HeadMesh) HeadMesh->SetLeaderPoseComponent(GetMesh());
        if (HorseMesh) HorseMesh->SetLeaderPoseComponent(GetMesh());
    }

    // 에디터 뷰포트 업데이트
    if (EquipComp && !UnitRowName.IsNone())
    {
        // 1. 컴포넌트가 소유한 테이블에서 데이터를 직접 가져옵니다.
        const FST_Unit* UnitData = EquipComp->GetUnitData(UnitRowName);
        if (UnitData)
        {
            // 2. 메시 업데이트 함수 실행
            HandleUnitBodyUpdate(*UnitData);
        }
    }
}

void ARtsUnitCharacter::InitializeUnit()
{
    // 캐릭터는 '어떤 유닛인지' 식별자만 컴포넌트에 넘깁니다.
    if (EquipComp && !UnitRowName.IsNone())
    {
        EquipComp->EquipToUnitData(UnitRowName);
    }
}

void ARtsUnitCharacter::HandleUnitBodyUpdate(const FST_Unit& UnitData)
{
    // 시각적 업데이트 실행
    UpdateArmorMesh(EEquipType::Head, UnitData.HeadPart);
    UpdateArmorMesh(EEquipType::Body, UnitData.BodyPart);

    // 스탯 반영
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = UnitData.Speed;
    }
}

void ARtsUnitCharacter::UpdateArmorMesh(EEquipType Type, USkeletalMesh* NewMesh)
{
    switch (Type)
    {
    case EEquipType::Head:
        if (HeadMesh) HeadMesh->SetSkeletalMesh(NewMesh);
        break;
    case EEquipType::Body:
        if (GetMesh()) GetMesh()->SetSkeletalMesh(NewMesh);
        break;
    case EEquipType::Horse:
        if (HorseMesh)
        {
            HorseMesh->SetSkeletalMesh(NewMesh);
            HorseMesh->SetVisibility(NewMesh != nullptr); // 말이 없으면 숨김 처리
        }
        break;
    default:
        break;
    }
}

int32 ARtsUnitCharacter::GetFaction_Implementation() const
{
    return TeamComp ? TeamComp->m_Faction : -1;
}