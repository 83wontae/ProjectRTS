// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Building.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"

ABuilding::ABuilding()
{
    PrimaryActorTick.bCanEverTick = false; // 건물은 보통 매 프레임 연산이 필요 없으므로 끕니다.

    // 메시 컴포넌트 생성 및 루트 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
    RootComponent = MeshComponent;
}

void ABuilding::BeginPlay()
{
    Super::BeginPlay();

    // 스폰되자마자 초기화 시도
    InitializeBuilding();
}

void ABuilding::InitializeBuilding()
{
    // 가드 클로저: 데이터 테이블이 없거나 RowName이 유효하지 않으면 중단
    if (!BuildingDataTable || BuildingRowName.IsNone()) return;

    // 가드 클로저: 데이터 테이블에서 행을 찾지 못하면 중단
    const FST_Building* FoundData = BuildingDataTable->FindRow<FST_Building>(BuildingRowName, TEXT(""));
    if (!FoundData || !FoundData->BuildingMesh) return;

    // 모든 검사를 통과하면 메시 설정 (들여쓰기 최소화)
    MeshComponent->SetStaticMesh(FoundData->BuildingMesh);

    // 추가적인 초기화 로직(체력 설정 등)을 여기에 작성하면 됩니다.
}