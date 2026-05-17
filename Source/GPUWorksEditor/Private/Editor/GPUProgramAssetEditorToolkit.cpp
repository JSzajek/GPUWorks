#include "GPUProgramAssetEditorToolkit.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

#include "GPUWorksLib.h"

const FName GPUProgramEditorTab(TEXT("ShaderSourceTab"));

void FGPUProgramAssetEditorToolkit::InitEditor(const EToolkitMode::Type& Mode, 
											   const TSharedPtr<class IToolkitHost>& InitToolkitHost, 
											   TObjectPtr<UGPUProgramAsset> InAsset)
{
	mpProgramAsset = InAsset;

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("GPUProgramEditorLayout_v2")
													->AddArea(FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
														->Split(FTabManager::NewStack()
															->AddTab(GPUProgramEditorTab, ETabState::OpenedTab)
																->SetHideTabWell(true)));

	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, "GPUProgramEditorApp", Layout, true, true, Cast<UObject>(InAsset));

	RegenerateMenusAndToolbars();
}

void FGPUProgramAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(GPUProgramEditorTab, FOnSpawnTab::CreateSP(this, &FGPUProgramAssetEditorToolkit::SpawnEditorTab))
																				 .SetDisplayName(FText::FromString("Source"))
																				 .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

TSharedRef<SDockTab> FGPUProgramAssetEditorToolkit::SpawnEditorTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(NSLOCTEXT("GPUWorks", "ShaderSourceTab", "GPU Source"))
		   [
			   SNew(SShaderEditorWidget).ProgramAsset(mpProgramAsset)
		   ];
}
