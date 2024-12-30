// Copyright 2024 romaktion@gmail.com. All Rights Reserved.

using UnrealBuildTool;

public class SkeletalToProcedural : ModuleRules
{
	public SkeletalToProcedural(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[]
			{
				//"SkeletalToProcedural/Public"
				// ... add public include paths required here ...
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				//System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "Source/Editor/Blutility/Private",
				//"SkeletalToProcedural/Private",
				// ... add other private include paths required here ...
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
				// ... add other public dependencies that you statically link with here ...
			}
		);
		
		PrivateIncludePathModuleNames.AddRange(
			new[]
			{
				"MessageLog",
				"PackagesDialog",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"UnrealEd",
				"EditorStyle",
				"PropertyEditor",
				"LevelEditor",
				"SkeletalToProceduralRuntime",
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new[]
			{
				// ... add any modules that your module loads dynamically here ...

				"MessageLog",
				"PackagesDialog",
			}
		);
	}
}
