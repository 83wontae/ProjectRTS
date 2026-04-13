// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RtsPlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h" // 필수 헤더
#include "Kismet/GameplayStatics.h"       // 필수 헤더
#include "Actors/PreviewBuilding.h"        // 고스트 건물 클래스 헤더

bool ARtsPlayerController::IsInSideDragRect(FVector LocationToTest, FVector2D Start, FVector2D End)
{
    FVector2D ScreenPos;
    // 2. 월드 좌표를 위젯/화면 좌표로 변환 (Player Viewport Relative = true와 동일한 역할)
    // ProjectWorldLocationToWidgetPosition의 C++ 내부 구현을 활용하거나 아래 함수 사용
    if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(this, LocationToTest, ScreenPos, true) == false)
        return false;

    // 3. 앞서 만든 판정 로직 적용
    float MinX = FMath::Min(Start.X, End.X);
    float MaxX = FMath::Max(Start.X, End.X);
    float MinY = FMath::Min(Start.Y, End.Y);
    float MaxY = FMath::Max(Start.Y, End.Y);

    return (ScreenPos.X >= MinX && ScreenPos.X <= MaxX && ScreenPos.Y >= MinY && ScreenPos.Y <= MaxY);
}

void ARtsPlayerController::GetActorsInsideDragRect(TSubclassOf<UInterface> TargetInterface, FVector2D Start, FVector2D End, TArray<AActor*>& OutActors)
{
    OutActors.Empty();

    if (!TargetInterface) return;

    // 1. 월드 내 모든 액터를 가져옵니다. 
    // (성능을 위해 실제로는 모든 액터보다는 유닛들의 부모 클래스나 필터링된 리스트를 쓰는 것이 좋습니다)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), TargetInterface, FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor)
        {
            // 2. 이미 구현된 IsInSideDragRect 호출 (이전 답변에서 만든 함수)
            if (IsInSideDragRect(Actor->GetActorLocation(), Start, End))
            {
                OutActors.Add(Actor);
            }
        }
    }
}

void ARtsPlayerController::StartPlacementMode(FName BuildingRowName)
{
    // 1. 이미 다른 고스트가 있다면 제거 (중복 생성 방지)
    ClearCurrentPreview();

    if (!PreviewBuildingClass) return;

    // 2. 고스트 스폰
    FActorSpawnParameters SpawnParams;
    CurrentPreviewActor = GetWorld()->SpawnActor<APreviewBuilding>(PreviewBuildingClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

    if (CurrentPreviewActor)
    {
        // 3. 생성된 객체에 RowName을 전달 (그러면 PreviewBuilding이 스스로 초기화함)
        CurrentPreviewActor->SetupPreviewBuilding(BuildingRowName);
    }
}

void ARtsPlayerController::ClearCurrentPreview()
{
    if (CurrentPreviewActor)
    {
        CurrentPreviewActor->Destroy();
        CurrentPreviewActor = nullptr; // 포인터 초기화 중요!
    }
}