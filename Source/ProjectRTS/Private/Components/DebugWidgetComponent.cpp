// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DebugWidgetComponent.h"

// Sets default values for this component's properties
UDebugWidgetComponent::UDebugWidgetComponent()
{
	// 1. 기본 설정: 항상 카메라를 바라보게 만듭니다. 
	// Space를 Screen으로 설정하면 엔진이 알아서 항상 유저 화면에 정면으로 렌더링합니다. (가장 효율적)
	SetWidgetSpace(EWidgetSpace::Screen);

	// 2. 추가 UI 기본 설정
	SetDrawSize(FVector2D(200.f, 150.f)); // 기본 사이즈
	SetGenerateOverlapEvents(false);      // 성능을 위해 충돌 제외
	bAutoActivate = true;
}


void UDebugWidgetComponent::UpdateLogList(const TArray<FString>& NewLogs)
{
	// 1. [가드] 위젯 인스턴스 확인
	UUserWidget* DebugWidget = GetUserWidgetObject();
	if (!DebugWidget) return;

	// 2. [가드] 위젯 내부에 구현된 특정 인터페이스나 함수 호출
	// 위젯 블루프린트에 "UpdateList"라는 함수를 만들었다고 가정하거나, 인터페이스를 씁니다.
	FString CombinedLog = FString::Join(NewLogs, TEXT("\n"));

	// 블루프린트 함수 호출 (또는 전용 위젯 클래스로 캐스팅)
	UFunction* Func = DebugWidget->FindFunction(FName("UpdateLogText"));
	if (Func)
	{
		DebugWidget->ProcessEvent(Func, &CombinedLog);
	}
}
