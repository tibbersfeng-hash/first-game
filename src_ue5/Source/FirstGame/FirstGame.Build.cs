// Copyright 2026 格斗萌主 Team. All Rights Reserved.

using UnrealBuildTool;

public class FirstGame : ModuleRules
{
	public FirstGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"UMG",
			"Slate",
			"SlateCore",
			"Paper2D",
			"AIModule",
			"GameplayCameras"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore"
		});

		// Enable UE5 Live Coding for faster iteration
		if (Target.Configuration == UnrealTargetConfiguration.Development)
		{
			bUseUnity = false;
		}
	}
}
