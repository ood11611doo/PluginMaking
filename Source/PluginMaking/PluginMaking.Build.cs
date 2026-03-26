// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PluginMaking : ModuleRules
{
	public PluginMaking(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
