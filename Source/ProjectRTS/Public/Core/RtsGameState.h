// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Interface/UnitInterface.h"
#include "RtsGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTRTS_API ARtsGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
    // 유닛들로부터 진영 ID를 받아 아군인지 판정하는 중앙 함수
    UFUNCTION(BlueprintPure, Category = "RTS|Team")
    bool IsAlly(int32 FactionA, int32 FactionB) const
    {
        // 현재 로직: 번호가 같으면 아군
        // 추후 여기서 '관계 행렬(Matrix)'을 참조하도록 확장 가능
        // 2차원 배열이나 TMap을 사용하여 관계 조회
        // return RelationshipMatrix[FactionA][FactionB] == EAttitude::Friendly;

        return FactionA == FactionB;
    }

    UFUNCTION(BlueprintPure, Category = "RTS|Team")
    bool IsEnemy(int32 FactionA, int32 FactionB) const
    {
        return !IsAlly(FactionA, FactionB);
    }

    UFUNCTION(BlueprintPure, Category = "RTS|Team")
    bool IsEnemyByActor(AActor* ActorA, AActor* ActorB) const
    {
        if (!IsValid(ActorA) || !IsValid(ActorB)) return false;

        // 인터페이스 구현 여부 확인 및 Faction 추출
        int32 FactionA = ActorA->Implements<UUnitInterface>() ? IUnitInterface::Execute_GetFaction(ActorA) : -1;
        int32 FactionB = ActorB->Implements<UUnitInterface>() ? IUnitInterface::Execute_GetFaction(ActorB) : -1;

        // 하나라도 유효한 진영이 없다면 적으로 간주하지 않음 (예: 중립 프롭 등)
        if (FactionA == -1 || FactionB == -1) return false;

        return IsEnemy(FactionA, FactionB);
    }

    UFUNCTION(BlueprintPure, Category = "RTS|Team")
    bool IsAllyByActor(AActor* ActorA, AActor* ActorB) const
    {
        if (!IsValid(ActorA) || !IsValid(ActorB)) return false;

        // 인터페이스 구현 여부 확인 및 Faction 추출
        int32 FactionA = ActorA->Implements<UUnitInterface>() ? IUnitInterface::Execute_GetFaction(ActorA) : -1;
        int32 FactionB = ActorB->Implements<UUnitInterface>() ? IUnitInterface::Execute_GetFaction(ActorB) : -1;

        // 하나라도 유효한 진영이 없다면 적으로 간주하지 않음 (예: 중립 프롭 등)
        if (FactionA == -1 || FactionB == -1) return false;

        return IsAlly(FactionA, FactionB);
    }
};
