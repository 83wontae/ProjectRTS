// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/RecruitmentComponent.h"
#include "Core/RtsUnitCharacter.h"
#include "Components/StateComponent.h" // 추가
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"

URecruitmentComponent::URecruitmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URecruitmentComponent::AddUnitToRoster(FName UnitRowName, FName HandR, FName HandL)
{
    if (UnitRowName.IsNone()) return;

    // 1. 초기화된 통합 구조체 생성
    FST_UnitSaveData NewUnit;
    NewUnit.UnitRowName = UnitRowName;
    NewUnit.HandR = HandR;
    NewUnit.HandL = HandL;
    NewUnit.Level = 1; // 신규 고용은 1레벨부터

    RecruitedUnits.Add(NewUnit);

    if (OnUnitRecruited.IsBound())
    {
        OnUnitRecruited.Broadcast(NewUnit);
    }
}

void URecruitmentComponent::SpawnRecruitedUnits(FName StartTag, float SpawnRadius)
{
    // 1. 데이터 테이블 유효성 확인
    if (!UnitDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("URecruitmentComponent: UnitDataTable is NULL!"));
        return;
    }

    // 2. PlayerStart 찾기
    APlayerStart* TargetStart = nullptr;
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        if (It->PlayerStartTag == StartTag) { TargetStart = *It; break; }
    }
    if (!TargetStart) return;

    FVector CenterLocation = TargetStart->GetActorLocation();

    // 3. 리스트의 모든 유닛 순회하며 스폰
    for (const FST_UnitSaveData& RecruitData : RecruitedUnits)
    {
        // 데이터 테이블에서 유닛 정보(UnitClass 포함)를 가져옵니다.
        const FST_Unit* UnitInfo = UnitDataTable->FindRow<FST_Unit>(RecruitData.UnitRowName, TEXT(""));

        // 유효성 및 클래스 설정 확인
        if (!UnitInfo || !UnitInfo->UnitClass) continue;

        // 4. 위치 계산
        FVector2D RandomCircle = FMath::RandPointInCircle(SpawnRadius);
        FVector SpawnLocation = CenterLocation + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // 5. [수정 포인트] 데이터 테이블에 정의된 클래스로 스폰을 실행합니다.
        ARtsUnitCharacter* SpawnedUnit = GetWorld()->SpawnActor<ARtsUnitCharacter>(
            UnitInfo->UnitClass,
            SpawnLocation,
            TargetStart->GetActorRotation(),
            SpawnParams
        );

        if (SpawnedUnit)
        {
            SpawnedUnit->SpawnDefaultController(); // AI 빙의

            // 6. 상태 및 장비 복구 (통합 데이터 주입)
            if (SpawnedUnit->StateComp)
            {
                SpawnedUnit->StateComp->LoadFromFullSaveData(RecruitData);
            }
        }
    }
}