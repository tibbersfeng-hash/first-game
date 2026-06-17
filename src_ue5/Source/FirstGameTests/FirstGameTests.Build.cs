// Copyright 2026 格斗萌主 Team. All Rights Reserved.

using UnrealBuildTool;

public class FirstGameTests : ModuleRules
{
	public FirstGameTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"FirstGame",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"Blutility",
		});
	}
}
