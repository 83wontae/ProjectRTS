// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DebugWidgetComponent.h"

// Sets default values for this component's properties
UDebugWidgetComponent::UDebugWidgetComponent()
{
	// Tick을 활성화합니다.
	PrimaryComponentTick.bCanEverTick = true;

	// 초기에는 체크할 데이터가 없으므로 꺼두었다가 데이터가 들어오면 로직 시작
	SetComponentTickEnabled(true);

	// 1. 기본 설정: 항상 카메라를 바라보게 만듭니다. 
	// Space를 Screen으로 설정하면 엔진이 알아서 항상 유저 화면에 정면으로 렌더링합니다. (가장 효율적)
	SetWidgetSpace(EWidgetSpace::Screen);

	// 2. 추가 UI 기본 설정
	SetDrawSize(FVector2D(200.f, 150.f)); // 기본 사이즈
	SetGenerateOverlapEvents(false);      // 성능을 위해 충돌 제외
	bAutoActivate = true;
}

void UDebugWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // [핵심 로직] 보낼 데이터가 있을 때만 위젯 생성을 체크합니다.
    if (bHasPendingLogs)
    {
        UUserWidget* DebugWidget = GetUserWidgetObject();

        // 위젯이 태어난 바로 그 프레임!
        if (IsValid(DebugWidget))
        {
            // 보관해둔 마지막 로그를 즉시 갱신
            UpdateLogList(PendingLogs);

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Tick: Widget Detected & Updated!"));
        }
    }
}

void UDebugWidgetComponent::UpdateLogList(const TArray<FString>& NewLogs)
{
    // 1. 위젯 확인
    UUserWidget* DebugWidget = GetUserWidgetObject();

    // 2. [가드] 위젯이 아직 없다면 보관함에 넣고 "확실하게" 대기
    if (!IsValid(DebugWidget))
    {
        PendingLogs = NewLogs;
        bHasPendingLogs = true;
        return;
    }

    // 3. [가드] 위젯은 있는데 함수가 없다면 종료
    UFunction* Func = DebugWidget->FindFunction(FName("UpdateLogText"));
    if (!Func) return;

    // 4. 정상 전송
    FString CombinedLog = FString::Join(NewLogs, TEXT("\n"));
    DebugWidget->ProcessEvent(Func, &CombinedLog);

    // 전송 완료 후 대기 상태 해제
    bHasPendingLogs = false;
}
