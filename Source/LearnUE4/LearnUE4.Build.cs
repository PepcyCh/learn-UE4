// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LearnUE4 : ModuleRules
{
	public LearnUE4(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
