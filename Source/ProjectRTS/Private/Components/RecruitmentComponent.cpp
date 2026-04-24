// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/RecruitmentComponent.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Core/RtsUnitCharacter.h"
#include "Components/EquipComponent.h"
#include "Kismet/KismetMathLibrary.h"

URecruitmentComponent::URecruitmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URecruitmentComponent::BeginPlay()
{
    Super::BeginPlay();
}

void URecruitmentComponent::AddUnitToRoster(FName UnitRowName, FName HandR, FName HandL)
{
    if (UnitRowName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("URecruitmentComponent: Attempted to add unit with None RowName."));
        return;
    }

    FST_RecruitUnit NewUnit;
    NewUnit.UnitRowName = UnitRowName;
    NewUnit.HandR = HandR;
    NewUnit.HandL = HandL;

    RecruitedUnits.Add(NewUnit);

    // --- [추가된 로직: 이벤트 전파] ---
    // 구독 중인 모든 블루프린트나 C++ 클래스에 알림을 보냅니다.
    if (OnUnitRecruited.IsBound())
    {
        OnUnitRecruited.Broadcast(NewUnit);
    }

    UE_LOG(LogTemp, Log, TEXT("URecruitmentComponent: Unit [%s] added to roster. Total: %d"),
        *UnitRowName.ToString(), RecruitedUnits.Num());
}

void URecruitmentComponent::SpawnRecruitedUnits(FName StartTag, float SpawnRadius)
{
    if (!UnitDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("URecruitmentComponent: UnitDataTable is NOT assigned!"));
        return;
    }

    // 1. 해당 태그를 가진 PlayerStart 찾기
    APlayerStart* TargetStart = nullptr;
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        if (It->PlayerStartTag == StartTag)
        {
            TargetStart = *It;
            break;
        }
    }

    if (!TargetStart)
    {
        UE_LOG(LogTemp, Warning, TEXT("URecruitmentComponent: PlayerStart with Tag [%s] not found!"), *StartTag.ToString());
        return;
    }

    FVector CenterLocation = TargetStart->GetActorLocation();

    // 2. 리스트의 모든 유닛 스폰 루프
    for (const FST_RecruitUnit& RecruitData : RecruitedUnits)
    {
        // 데이터 테이블에서 유닛 정보 가져오기
        const FST_Unit* UnitInfo = UnitDataTable->FindRow<FST_Unit>(RecruitData.UnitRowName, TEXT(""));
        if (!UnitInfo || !UnitInfo->UnitClass) continue;

        // 3. 랜덤 위치 계산 (원형 영역)
        FVector2D RandomCircle = FMath::RandPointInCircle(SpawnRadius);
        FVector SpawnLocation = CenterLocation + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);

        // 지형 높이에 맞추기 위한 간단한 조정 (필요 시)
        SpawnLocation.Z = CenterLocation.Z;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // 4. 유닛 스폰
        ARtsUnitCharacter* SpawnedUnit = GetWorld()->SpawnActor<ARtsUnitCharacter>(
            UnitInfo->UnitClass,
            SpawnLocation,
            TargetStart->GetActorRotation(),
            SpawnParams
        );

        if (SpawnedUnit)
        {
            // 5. 무기 장착 (EquipComponent 활용)
            if (SpawnedUnit->EquipComp)
            {
                // 오른손 무기 장착
                if (!RecruitData.HandR.IsNone())
                    SpawnedUnit->EquipComp->EquipToWeapon(RecruitData.HandR);

                // 왼손 무기 장착
                if (!RecruitData.HandL.IsNone())
                    SpawnedUnit->EquipComp->EquipToWeapon(RecruitData.HandL);
            }

            UE_LOG(LogTemp, Log, TEXT("URecruitmentComponent: Spawned [%s] at %s"),
                *RecruitData.UnitRowName.ToString(), *SpawnLocation.ToString());
        }
    }
}