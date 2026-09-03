// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UInventoryComponent::FindEmptySlot(int32& OutIndex)
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		// IsEmptySlot 함수를 재사용하여 빈 슬롯 여부 판별
		if (IsEmptySlot(i))
		{
			OutIndex = i;
			return true;
		}
	}

	OutIndex = INDEX_NONE; // 언리얼 기본 무효 인덱스 상수 (-1)
	return false;
}

void UInventoryComponent::SetEmptySlot(int32 Index)
{
	// 1. 유효한 슬롯 인덱스인지 확인
	if (!Slots.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetEmptySlot: Invalid Index [%d]."), Index);
		return;
	}

	// 2. 데이터 테이블 유효성 검사
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetEmptySlot: ItemDataTable is not set."));
		return;
	}

	// 3. Get Data Table Row (Row Name: "Empty")
	static const FString ContextString(TEXT("SetEmptySlotContext"));
	const FST_ItemBase* EmptyItemData = ItemDataTable->FindRow<FST_ItemBase>(FName("Empty"), ContextString);

	if (!EmptyItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetEmptySlot: 'Empty' row not found in DataTable."));
		return;
	}

	// 4. Set members in ST_ItemSlot (Slots[Index]의 ItemData 교체 및 수량 초기화)
	Slots[Index].ItemData = *EmptyItemData;
	Slots[Index].Count = 0; // 빈 슬롯이므로 수량도 0으로 초기화

	// 5. Call Event Dispatcher ChangeInven
	OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::IsEmptySlot(int32 Index) const
{
	// 유효하지 않은 인덱스인 경우 처리
	if (!Slots.IsValidIndex(Index))
	{
		return false;
	}

	// Slots[Index]의 ItemData.ItemType이 EItemType::None인지 확인하여 빈 슬롯 여부 반환
	return Slots[Index].ItemData.ItemType == EItemType::None;
}

void UInventoryComponent::RemoveItem(int32 Index)
{
	SetEmptySlot(Index);
}

void UInventoryComponent::SwapItem(int32 FromIndex, int32 ToIndex)
{
	// 1. 두 인덱스가 모두 유효한지 검사
	if (!Slots.IsValidIndex(FromIndex) || !Slots.IsValidIndex(ToIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SwapItem: Invalid index. FromIndex: %d, ToIndex: %d"), FromIndex, ToIndex);
		return;
	}

	// 2. 같은 인덱스라면 굳이 스왑 및 알림을 할 필요가 없으므로 반환
	if (FromIndex == ToIndex)
	{
		return;
	}

	// 3. Slots 배열의 두 요소 맞교환 (블루프린트 SWAP 매크로와 동일)
	Swap(Slots[FromIndex], Slots[ToIndex]);

	// 4. 이벤트 디스패처 호출 (Call Event Dispatcher Change Inven)
	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::CreateInvenSlots(int32 Count)
{
	// 1. 유효성 검사
	if (Count <= 0)
	{
		return;
	}

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateInvenSlots: ItemDataTable is not set."));
		return;
	}

	// 2. DT_Item에서 RowName이 "Empty"인 행 가져오기
	static const FString ContextString(TEXT("CreateInvenSlotsContext"));
	const FST_ItemBase* EmptyItemData = ItemDataTable->FindRow<FST_ItemBase>(FName("Empty"), ContextString);

	if (!EmptyItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateInvenSlots: 'Empty' row not found in DataTable."));
		return;
	}

	// 3. Make ST_ItemSlot (Count: 0, ItemData: EmptyItemData)
	FST_ItemSlot NewEmptySlot;
	NewEmptySlot.Count = 0;
	NewEmptySlot.ItemData = *EmptyItemData;

	// 메모리 재할당 방지를 위한 공간 미리 확보
	Slots.Reserve(Slots.Num() + Count);

	// 4. For Loop: 0 ~ (Count - 1) 만큼 슬롯 추가 (ADD Slots)
	for (int32 i = 0; i < Count; ++i)
	{
		Slots.Add(NewEmptySlot);
	}
}

void UInventoryComponent::AddItem(FName RowName)
{
	// 1. RowName 유효성 검사 (Branch: RowName != None)
	if (RowName.IsNone())
	{
		return;
	}

	// 2. 데이터 테이블 유효성 검사
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItem: ItemDataTable is not set."));
		return;
	}

	// 3. FItem 구조체로 데이터 테이블 행 검색
	static const FString ContextString(TEXT("AddItemContext"));
	const FST_ItemBase* FoundItem = ItemDataTable->FindRow<FST_ItemBase>(RowName, ContextString);

	if (!FoundItem)
	{
		// 데이터 테이블에 해당 Row가 없으면 중단 (Row Not Found)
		UE_LOG(LogTemp, Warning, TEXT("AddItem: Row [%s] not found in DataTable."), *RowName.ToString());
		return;
	}

	// 4. Find Empty Slot
	int32 EmptySlotIndex = -1;
	if (FindEmptySlot(EmptySlotIndex))
	{
		// 5. Set Array Elem: 찾은 슬롯 인덱스에 데이터 갱신
		if (Slots.IsValidIndex(EmptySlotIndex))
		{
			Slots[EmptySlotIndex].ItemData = *FoundItem;
			Slots[EmptySlotIndex].Count = 1;

			// 6. Call Event Dispatcher (ChangeInven 이벤트 브로드캐스트)
			OnInventoryChanged.Broadcast();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItem: No empty slot available."));
	}
}