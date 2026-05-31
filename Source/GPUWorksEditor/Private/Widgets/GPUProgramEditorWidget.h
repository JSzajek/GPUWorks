#pragma once

#include "Widgets/SCompoundWidget.h"

#include "Interops/UE/GPUContextObject.h"

class STextComboBox;
class STextBlock;
class SMultiLineEditableTextBox;
class SLineNumberBox;
class SScrollBar;

class UGPUProgramAsset;

class SGPUProgramEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGPUProgramEditorWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UGPUProgramAsset>, ProgramAsset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
private:
	FReply OnCompileClicked();

	void CompileProgram();

	void OnSourceChanged(const FText& NewText);
	void OnProgramLanguageChanged(TSharedPtr<FString> newSelection,
								  ESelectInfo::Type selectInfo);
	FReply OnHandleKeyDown(const FGeometry& MyGeometry,
						   const FKeyEvent& InKeyEvent);

	void InsertTabOrUnindent(bool shiftMod);
	void UpdateLineNumbers(const FString& Text);

	void SetCompileResult(bool success,
						  const FString& message);

	void SetDirty(bool isDirty);

	EGPUBackend GetSelectedBackend() const;
private:
	TWeakObjectPtr<UGPUProgramAsset> mpProgramAsset;

	TArray<TSharedPtr<FString>> mProgramLanguageOptions;
	TSharedPtr<STextComboBox> mpProgramLanguageComboBox;

	bool bDirty = false;
	bool bIsCompiling = false;

	TSharedPtr<SButton> mpCompileButton;
	TSharedPtr<STextBlock> mpStatusText;

	TSharedPtr<SScrollBar> mpEditorVScrollBar;
	TSharedPtr<SMultiLineEditableTextBox> mpSourceEditor;
	TSharedPtr<SLineNumberBox> mpLineNumberDisplay;

	TSharedPtr<SMultiLineEditableTextBox> mpLogOutput;
};