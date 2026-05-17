#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"

#include "Toolkits/AssetEditorToolkit.h"
#include "Widgets/SShaderEditorWidget.h"

class UGPUProgramAsset;

class FGPUProgramAssetEditorToolkit : public FAssetEditorToolkit
{
public:
	void InitEditor(const EToolkitMode::Type& Mode, 
					const TSharedPtr<class IToolkitHost>& InitToolkitHost,
					TObjectPtr<UGPUProgramAsset> InAsset);

	void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;

	virtual FName GetToolkitFName() const override 
	{ 
		return "CLProgramEditor"; 
	}

	virtual FText GetBaseToolkitName() const override 
	{ 
		return NSLOCTEXT("CLWorks", "CLProgramEditor", "CL Program Editor");
	}

	virtual FString GetWorldCentricTabPrefix() const override 
	{ 
		return TEXT("CLProgram"); 
	}

	virtual FLinearColor GetWorldCentricTabColorScale() const override 
	{ 
		return FLinearColor::Red; 
	}
private:
	TSharedRef<SDockTab> SpawnEditorTab(const FSpawnTabArgs& Args);
private:
	TSharedPtr<SShaderEditorWidget> mpShaderEditor;

	TObjectPtr<UGPUProgramAsset> mpProgramAsset;
};

#endif