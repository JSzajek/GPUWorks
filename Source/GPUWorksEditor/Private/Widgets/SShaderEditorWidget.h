#pragma once

#include "Widgets/Text/STextBlock.h"
#include "Widgets//SLineNumberBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#include "GPUWorksLib.h"

class UGPUProgramAsset;

class SShaderEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShaderEditorWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UGPUProgramAsset>, ProgramAsset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
private:
	FReply OnCompileClicked();
	void OnSourceChanged(const FText& NewText);
	FReply OnHandleKeyDown(const FGeometry& MyGeometry,
						   const FKeyEvent& InKeyEvent);

	void InsertTabOrUnindent(bool shiftMod);
	void UpdateLineNumbers(const FString& Text);
private:
	TWeakObjectPtr<UGPUProgramAsset> mpProgramAsset;

	TSharedPtr<SMultiLineEditableTextBox> mpSourceEditor;
	TSharedPtr<STextBlock> mpStatusText;
	TSharedPtr<SMultiLineEditableTextBox> mpErrorLogOutput;
	TSharedPtr<SLineNumberBox> mpLineNumberDisplay;

	struct ProgramData
	{
	public:
		ProgramData() = default;
	public:
		std::shared_ptr<Gpu::ICore> mpGPUCore = nullptr;
		std::shared_ptr<Gpu::IDevice> mpGPUDevice = nullptr;
		std::shared_ptr<Gpu::IContext> mpGPUContext = nullptr;
	};
	TSharedPtr<ProgramData> mpProgramData;
};