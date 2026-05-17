// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class GPUWorksBlueprint : ModuleRules
{
	public GPUWorksBlueprint(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[] 
		{
            Path.Combine(ModuleDirectory, "Public"),
        });	
		
		PrivateIncludePaths.AddRange(new string[] 
		{
			Path.Combine(ModuleDirectory),
        });
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",

			"OpenCL",
			"GPUWorks",
		});	
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
            "RHI",
            "RenderCore",
            "Engine",
			"Projects",
			"Slate",
			"SlateCore",
        });

        DynamicallyLoadedModuleNames.AddRange(new string[]
		{
		});
	}
}
