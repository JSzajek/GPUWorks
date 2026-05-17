// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class GPUWorksEditor : ModuleRules
{
	public GPUWorksEditor(ReadOnlyTargetRules Target) : base(Target)
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
            "InputCore"
        });

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Blutility",
                "UMGEditor",
                "UnrealEd"
            });
        }

        DynamicallyLoadedModuleNames.AddRange(new string[]
		{
		});
	}
}
