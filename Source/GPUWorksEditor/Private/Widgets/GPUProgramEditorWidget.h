#pragma once

#include "Widgets/SCompoundWidget.h"

#include "Interops/UE/GPUContextObject.h"

class STextComboBox;
class STextBlock;
class SMultiLineEditableTextBox;
class SLineNumberBox;
class SScrollBar;

class UGPUProgramAsset;

/// <summary>
/// GPU Program editor widget, which allows users to edit and compile GPU programs within the Unreal Editor.
/// </summary>
class SGPUProgramEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGPUProgramEditorWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UGPUProgramAsset>, ProgramAsset)
	SLATE_END_ARGS()

	/// <summary>
	/// Constructs the GPU Program Editor widget with the specified arguments.
	/// </summary>
	/// <param name="arguments">The arguments</param>
	void Construct(const FArguments& arguments);
private:
	/// <summary>
	/// On compile button clicked handler.
	/// </summary>
	/// <returns>The reply indicating whether the event was handled</returns>
	FReply OnCompileClicked();

	/// <summary>
	/// On source code changed handler.
	/// </summary>
	/// <param name="NewText">The new text</param>
	void OnSourceChanged(const FText& NewText);

	/// <summary>
	/// On program language selection changed handler.
	/// </summary>
	/// <param name="newSelection">The new selection</param>
	/// <param name="selectInfo">The selection info</param>
	void OnProgramLanguageChanged(TSharedPtr<FString> newSelection,
								  ESelectInfo::Type selectInfo);

	/// <summary>
	/// On key down handler for the source editor.
	/// </summary>
	/// <param name="MyGeometry">The geometry of the widget</param>
	/// <param name="InKeyEvent">The key event</param>
	/// <returns>The reply indicating whether the event was handled</returns>
	FReply OnHandleKeyDown(const FGeometry& MyGeometry,
						   const FKeyEvent& InKeyEvent);

	/// <summary>
	/// Triggers compilation of the program or project.
	/// </summary>
	void CompileProgram();

	/// <summary>
	/// Inserts a tab character at the current cursor position or un-indents the current line if shift is held.
	/// </summary>
	/// <param name="shiftMod">Indicates whether the shift key is held</param>
	void InsertTabOrUnindent(bool shiftMod);

	/// <summary>
	/// Updates the line number display based on the current text in the source editor.
	/// </summary>
	/// <param name="Text">The source editor text</param>
	void UpdateLineNumbers(const FString& Text);

	/// <summary>
	/// Sets the result of the compilation process, updating the UI to reflect success or failure and displaying any relevant messages.
	/// </summary>
	/// <param name="success">The result of the compilation process</param>
	/// <param name="message">The message to display in the UI</param>
	void SetCompileResult(bool success,
						  const FString& message);

	/// <summary>
	/// Sets the dirty state of the editor, which indicates whether there are unsaved changes.
	/// </summary>
	/// <param name="isDirty">The new dirty state</param>
	void SetDirty(bool isDirty);

	/// <summary>
	/// Retrieves the currently selected GPU backend based on the user's selection in the program language combo box.
	/// </summary>
	/// <returns>The currently selected GPU backend</returns>
	EGPUBackend GetSelectedBackend() const;
private:
	TWeakObjectPtr<UGPUProgramAsset> mpProgramAsset;

	TArray<TSharedPtr<FString>> mProgramLanguageOptions;
	TSharedPtr<STextComboBox> mpProgramLanguageComboBox;

	bool mIsDirty = false;
	bool mIsCompiling = false;

	TSharedPtr<SButton> mpCompileButton;
	TSharedPtr<STextBlock> mpStatusText;

	TSharedPtr<SScrollBar> mpEditorVScrollBar;
	TSharedPtr<SMultiLineEditableTextBox> mpSourceEditor;
	TSharedPtr<SLineNumberBox> mpLineNumberDisplay;

	TSharedPtr<SMultiLineEditableTextBox> mpLogOutput;
};