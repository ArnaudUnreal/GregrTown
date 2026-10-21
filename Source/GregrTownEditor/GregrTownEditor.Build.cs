using UnrealBuildTool;

public class GregrTownEditor : ModuleRules
{
	public GregrTownEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "GregrTown" });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"AssetRegistry",
			"GeometryCollectionEngine",
			"Chaos"
		});
	}
}
