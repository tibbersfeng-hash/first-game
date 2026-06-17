// Copyright 2026 格斗萌主 Team. All Rights Reserved.

using UnrealBuildTool;

public class FirstGameEditorTarget : TargetRules
{
	public FirstGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("FirstGame");
	}
}
