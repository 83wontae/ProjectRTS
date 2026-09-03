// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Global/ProjectRTSTypes.h"
#include "InventoryComponent.generated.h"

// 변경 알림용 이벤트 디스패처 (Event Dispatcher)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChangedSignature);


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTRTS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

	// AddItem 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FName RowName);

	// 빈 슬롯 인덱스를 찾는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool FindEmptySlot(int32& OutIndex);

	// 특정 인덱스의 슬롯을 'Empty' 상태로 설정하는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetEmptySlot(int32 Index);

	// 특정 슬롯이 비어있는지('Empty'인지) 확인하는 Pure 함수
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsEmptySlot(int32 Index) const;

	// RemoveItem 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItem(int32 Index);

	// 두 슬롯의 아이템을 맞바꾸는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapItem(int32 FromIndex, int32 ToIndex);

	// 지정된 슬롯 개수만큼 빈 슬롯을 생성하여 Slots 배열에 추가하는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CreateInvenSlots(int32 Count);

public:
	// 아이템 데이터 테이블 (에디터 디테일 패널에서 DT 지정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UDataTable* ItemDataTable;

	// 인벤토리 슬롯 배열 (Slots 변수)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FST_ItemSlot> Slots;

	// OnInventoryChanged / ChangeInven 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChangedSignature OnInventoryChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


};
