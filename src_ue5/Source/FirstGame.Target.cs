// Copyright 2026 格斗萌主 Team. All Rights Reserved.

using UnrealBuildTool;

public class FirstGameTarget : TargetRules
{
	public FirstGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("FirstGame");
	}
}
