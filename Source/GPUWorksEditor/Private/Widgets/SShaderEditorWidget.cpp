#include "Widgets/SShaderEditorWidget.h"

#include "Styling/AppStyle.h"

#include "GPUWorksLib.h"

void SShaderEditorWidget::Construct(const FArguments& InArgs)
{
	mProgramLanguageOptions =
	{
		MakeShared<FString>("OpenCL_C"),
		MakeShared<FString>("CUDA_C"),
		MakeShared<FString>("SharedGPUDSL"),
	};

	mpProgramAsset = InArgs._ProgramAsset;

	// Initial Internal Data --------------------
	mpProgramData = MakeShared<ProgramData>();

	Gpu::FactoryDesc desc;
	desc.PreferredBackend = Gpu::Backend::OpenCL;
	desc.bAllowFallback = false;

	mpProgramData->mpGPUCore = Gpu::Factory::Create(desc);
	mpProgramData->mpGPUDevice = mpProgramData->mpGPUCore->GetDevice(0);
	mpProgramData->mpGPUContext = mpProgramData->mpGPUCore->CreateContext(mpProgramData->mpGPUDevice);
	// ------------------------------------------

	ChildSlot
	[
		SNew(SVerticalBox)

		// Top row: compile button and status
		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
			[
				SAssignNew(mpProgramLanguageComboBox, STextComboBox)
						   .OptionsSource(&mProgramLanguageOptions)
						   .InitiallySelectedItem(mProgramLanguageOptions[0])
						   .OnSelectionChanged(this, &SShaderEditorWidget::OnProgramLanguageChanged)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton).Text(FText::FromString("Compile"))
							 .OnClicked(this, &SShaderEditorWidget::OnCompileClicked)
			]
			+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(5, 0)
			[
				SAssignNew(mpStatusText, STextBlock).Text(FText::FromString("Idle"))
			]
		]

		// Main text editor with line numbers
		+ SVerticalBox::Slot().FillHeight(1).Padding(2)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().Padding(4)
			[
				SAssignNew(mpLineNumberDisplay, SLineNumberBox)
			]

			+ SHorizontalBox::Slot().FillWidth(1)
			[
				SAssignNew(mpSourceEditor, SMultiLineEditableTextBox).Text(FText::FromString(mpProgramAsset.IsValid() ? mpProgramAsset->GetSourceCodeForBackend(GetSelectedBackend()) : TEXT("")))
																     .OnTextChanged(this, &SShaderEditorWidget::OnSourceChanged)
																	 .OnKeyDownHandler(this, &SShaderEditorWidget::OnHandleKeyDown)
																     .Font(FAppStyle::GetFontStyle("MonoFont"))
			]
		]

		// Error log
		+ SVerticalBox::Slot().AutoHeight().MaxHeight(100.0f).Padding(2)
		[
			SNew(SScrollBox)
				.Orientation(Orient_Vertical)

			+ SScrollBox::Slot()
			[
				SAssignNew(mpErrorLogOutput, SMultiLineEditableTextBox).Text(FText::FromString(""))
																	   .IsReadOnly(true)
																	   .AutoWrapText(true)
			]
		]
	];

	// Check Initial Compile Status
	OnCompileClicked();
	
	if (mpProgramAsset.IsValid())
	{
		FString programSource = mpProgramAsset->GetSourceCodeForBackend(GetSelectedBackend());
		UpdateLineNumbers(programSource);
	}
}

FReply SShaderEditorWidget::OnCompileClicked()
{
	if (!mpProgramAsset.IsValid())
		return FReply::Handled();

	FString programSource = mpProgramAsset->GetSourceCodeForBackend(GetSelectedBackend());

	FString compileLog = "";
	bool success = true;

	if (programSource.IsEmpty())
	{
		success = false;
		compileLog = "Empty Program!";
	}
	else
	{
		const std::string sourceCode(TCHAR_TO_UTF8(*programSource));
		std::string buildLog;
		std::shared_ptr<Gpu::IProgram> program = mpProgramData->mpGPUContext->CreateProgramFromSource(sourceCode, &buildLog);

		if (!success)
			compileLog = UTF8_TO_TCHAR(buildLog.c_str());
	}

	// Update Status Line
	if (mpStatusText.IsValid())
	{
		mpStatusText->SetText(FText::FromString(success ? TEXT("Compiled") : TEXT("Failed")));
		mpStatusText->SetColorAndOpacity(success ? FLinearColor::Green : FLinearColor::Red);
	}

	// Update Error Log
	if (mpErrorLogOutput.IsValid())
		mpErrorLogOutput->SetText(FText::FromString(compileLog));

	return FReply::Handled();
}

void SShaderEditorWidget::OnSourceChanged(const FText& NewText)
{
	if (mpProgramAsset.IsValid())
	{
		mpProgramAsset->Modify();
		mpProgramAsset->SetSourceCodeForBackend(GetSelectedBackend(), NewText.ToString());
	}

	UpdateLineNumbers(NewText.ToString());
}

void SShaderEditorWidget::OnProgramLanguageChanged(TSharedPtr<FString> newSelection,
												   ESelectInfo::Type selectInfo)
{
	mpSourceEditor->SetText(FText::FromString(mpProgramAsset.IsValid() ? mpProgramAsset->GetSourceCodeForBackend(GetSelectedBackend()) : TEXT("")));
}

FReply SShaderEditorWidget::OnHandleKeyDown(const FGeometry& MyGeometry, 
										  const FKeyEvent& InKeyEvent)
{
	if (!mpSourceEditor.IsValid())
		return FReply::Unhandled();

	const FKey key = InKeyEvent.GetKey();
	const bool shiftMod = InKeyEvent.IsShiftDown();

	if (key == EKeys::Tab)
	{
		InsertTabOrUnindent(shiftMod);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SShaderEditorWidget::InsertTabOrUnindent(bool shiftMod)
{
	FText currentText = mpSourceEditor->GetText();
	FString textString = currentText.ToString();

	FTextLocation cursorLocation = mpSourceEditor->GetCursorLocation();

	const int32 cursorLine = cursorLocation.GetLineIndex();
	const int32 cursorColumn = cursorLocation.GetOffset();

	TArray<FString> lines;
	textString.ParseIntoArrayLines(lines, false);

	// No valid line?
	if (!lines.IsValidIndex(cursorLine))
		return;

	FString& targetLine = lines[cursorLine];
	int32_t modDelta = 0;
	if (shiftMod) // Un-Indent
	{
		if (targetLine.StartsWith(TEXT("\t")))
		{
			targetLine.RemoveAt(0, 1, EAllowShrinking::No);
			modDelta = -1;
		}
		else if (targetLine.StartsWith(TEXT("    ")))
		{
			targetLine.RemoveAt(0, 4, EAllowShrinking::No);
			modDelta = -4;
		}
	}
	else
	{
		targetLine = TEXT("\t") + targetLine;
		modDelta = 1;
	}

	// Reconstruct Full Text --------------------
	const FString rebuiltText = FString::Join(lines, TEXT("\n"));
	mpSourceEditor->SetText(FText::FromString(rebuiltText));

	mpSourceEditor->GoTo(FTextLocation(cursorLocation, modDelta));

	UpdateLineNumbers(rebuiltText);
	// ------------------------------------------
}

void SShaderEditorWidget::UpdateLineNumbers(const FString& Text)
{
	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, false);

	if (mpLineNumberDisplay.IsValid())
	{
		mpLineNumberDisplay->UpdateLineNumbers(Lines.Num());
	}
}

EGPUBackend SShaderEditorWidget::GetSelectedBackend() const
{
	if (mpProgramLanguageComboBox.IsValid())
	{
		TSharedPtr<FString> selectedItem = mpProgramLanguageComboBox->GetSelectedItem();
		if (selectedItem.IsValid())
		{
			if (*selectedItem == "OpenCL_C")
				return EGPUBackend::OpenCL;
			else if (*selectedItem == "CUDA_C")
				return EGPUBackend::CUDA;
			else if (*selectedItem == "SharedGPUDSL")
				return EGPUBackend::Unknown;
		}
	}
	return EGPUBackend::Unknown;
}