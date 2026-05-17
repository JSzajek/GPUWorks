#include "Assets/GPUProgramAssetActions.h"


#if WITH_EDITOR

#include "Editor/GPUProgramAssetEditorToolkit.h"

void FGPUProgramAssetActions::OpenAssetEditor(const TArray<UObject*>& InObjects, 
											 TSharedPtr<IToolkitHost> ToolkitHost)
{
	for (UObject* Obj : InObjects)
	{
		if (UGPUProgramAsset* Asset = Cast<UGPUProgramAsset>(Obj))
		{
			TSharedRef<FGPUProgramAssetEditorToolkit> EditorToolkit = MakeShared<FGPUProgramAssetEditorToolkit>();
			EditorToolkit->InitEditor(EToolkitMode::Standalone, ToolkitHost, Asset);
		}
	}
}

#endif