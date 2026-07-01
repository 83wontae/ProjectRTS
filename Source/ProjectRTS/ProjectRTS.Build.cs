// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectRTS : ModuleRules
{
	public ProjectRTS(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 1. 유니티 빌드 활성화 (명령행 길이 단축에 필수)
        bUseUnity = true;

        // 2. 외부 인클루드 경로 주입 최소화
        // 엔진 모듈은 PublicDependencyModuleNames를 통해서만 참조해야 합니다.
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "AIModule",
            "NavigationSystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}
