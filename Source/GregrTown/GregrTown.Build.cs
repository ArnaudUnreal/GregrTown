// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GregrTown : ModuleRules
{
	public GregrTown(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "DeveloperSettings", "GeometryCollectionEngine" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Chaos", "FieldSystemEngine", "Niagara", "UMG", "Slate", "SlateCore", "Landscape" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
