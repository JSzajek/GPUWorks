// Copyright Epic Games, Inc. All Rights Reserved.

#include "GPUWorksEditor.h"

#include "AssetToolsModule.h"

#include "Assets/GPUProgramAssetActions.h"
#include "Editor/GPUProgramAssetEditorToolkit.h"

#define LOCTEXT_NAMESPACE "FCLWorksEditorModule"

void FGPUWorksEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	EAssetTypeCategories::Type CLCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("GPUWorksEditor")),		// Internal category name
																					 FText::FromString(TEXT("CL")));	// Display name

	// Register Custom Asset Actions
	AssetActions = MakeShareable(new FGPUProgramAssetActions(CLCategory));
	AssetTools.RegisterAssetTypeActions(AssetActions.ToSharedRef());
}

void FGPUWorksEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGPUWorksEditorModule, GPUWorksEditor)