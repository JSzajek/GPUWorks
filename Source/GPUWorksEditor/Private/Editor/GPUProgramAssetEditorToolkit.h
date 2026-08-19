#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"

#include "Toolkits/AssetEditorToolkit.h"

class UGPUProgramAsset;
class SGPUProgramEditorWidget;

/// <summary>
/// GPU Program Asset Editor Toolkit is responsible for managing the editor interface for GPUProgramAsset.
/// </summary>
class FGPUProgramAssetEditorToolkit : public FAssetEditorToolkit
{
public:
	/// <summary>
	/// Initializes the editor toolkit with the specified mode, toolkit host, and GPU program asset.
	/// </summary>
	/// <param name="Mode">The toolkit mode</param>
	/// <param name="InitToolkitHost">The toolkit host</param>
	/// <param name="InAsset">The GPU program asset</param>
	void InitEditor(const EToolkitMode::Type& Mode, 
					const TSharedPtr<class IToolkitHost>& InitToolkitHost,
					TObjectPtr<UGPUProgramAsset> InAsset);

	/// <summary>
	/// Registers the tab spawners for the editor toolkit, allowing the editor to create and manage tabs for the GPU program asset.
	/// </summary>
	/// <param name="TabManager">The tab manager</param>
	void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;

	/// <summary>
	/// Retrieves the name of the toolkit.
	/// </summary>
	/// <returns>The toolkit name</returns>
	virtual FName GetToolkitFName() const override 
	{ 
		return "GPUProgramEditor"; 
	}

	/// <summary>
	/// Retrieves the name of the base toolkit.
	/// </summary>
	/// <returns>The base toolkit name</returns>
	virtual FText GetBaseToolkitName() const override 
	{ 
		return NSLOCTEXT("GPUWorks", "GPUProgramEditor", "GPU Program Editor");
	}

	/// <summary>
	/// Retrieves the world-centric tab prefix for the editor toolkit.
	/// </summary>
	/// <returns>The world-centric tab prefix</returns>
	virtual FString GetWorldCentricTabPrefix() const override 
	{ 
		return TEXT("GPUProgram"); 
	}

	/// <summary>
	/// Retrieves the world-centric tab color scale for the editor toolkit.
	/// </summary>
	/// <returns>The world-centric tab color scale</returns>
	virtual FLinearColor GetWorldCentricTabColorScale() const override 
	{ 
		return FLinearColor::Red; 
	}
private:
	/// <summary>
	/// Spawn the editor tab for the GPU program asset, creating an instance of the SGPUProgramEditorWidget to display the asset's properties and allow editing.
	/// </summary>
	/// <param name="Args">The spawn arguments</param>
	/// <returns>The spawned dock tab</returns>
	TSharedRef<SDockTab> SpawnEditorTab(const FSpawnTabArgs& Args);
private:
	TSharedPtr<SGPUProgramEditorWidget> mpShaderEditor;

	TObjectPtr<UGPUProgramAsset> mpProgramAsset;
};

#endif