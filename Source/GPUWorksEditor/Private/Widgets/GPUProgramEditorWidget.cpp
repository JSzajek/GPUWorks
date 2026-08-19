#include "Widgets/GPUProgramEditorWidget.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/LineNumberBoxWidget.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SScrollBar.h"

#include "Styling/AppStyle.h"

#include "GPUWorksLib.h"

void SGPUProgramEditorWidget::Construct(const FArguments& arguments)
{
	mProgramLanguageOptions =
	{
		MakeShared<FString>("OpenCL_C"),
		MakeShared<FString>("CUDA_C"),
		MakeShared<FString>("SharedGPUDSL"),
	};

	mIsCompiling = false;

	mpProgramAsset = arguments._ProgramAsset;

	SAssignNew(mpEditorVScrollBar, SScrollBar)
			   .Orientation(Orient_Vertical);

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
						   .OnSelectionChanged(this, &SGPUProgramEditorWidget::OnProgramLanguageChanged)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SAssignNew(mpCompileButton, SButton)
						   .Text_Lambda([this]()
						   {
								return FText::FromString(mIsCompiling ? TEXT("Compiling...") : TEXT("Compile [Ctrl+Enter]"));
						   })
						   .IsEnabled_Lambda([this]()
						   {
						   		return !mIsCompiling;
						   })
						   .OnClicked(this, &SGPUProgramEditorWidget::OnCompileClicked)
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
				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
				[
					SAssignNew(mpLineNumberDisplay, SLineNumberBox)
								.Font(FAppStyle::GetFontStyle("MonoFont"))
								.ExternalScrollbar(mpEditorVScrollBar)
				]

				+ SHorizontalBox::Slot().FillWidth(1)
				[
					SAssignNew(mpSourceEditor, SMultiLineEditableTextBox).Text(FText::FromString(mpProgramAsset.IsValid() ? mpProgramAsset->GetSourceCodeForBackend(GetSelectedBackend()) : TEXT("")))
								.OnTextChanged(this, &SGPUProgramEditorWidget::OnSourceChanged)
								.OnKeyDownHandler(this, &SGPUProgramEditorWidget::OnHandleKeyDown)
								.Font(FAppStyle::GetFontStyle("MonoFont"))
								.VScrollBar(mpEditorVScrollBar)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					mpEditorVScrollBar.ToSharedRef()
				]
		]

		// Error log
		+ SVerticalBox::Slot().AutoHeight().MaxHeight(100.0f).Padding(2)
		[
			SNew(SScrollBox)
				.Orientation(Orient_Vertical)

			+ SScrollBox::Slot()
			[
				SAssignNew(mpLogOutput, SMultiLineEditableTextBox).Text(FText::FromString(""))
						   .IsReadOnly(true)
						   .AutoWrapText(true)
						   .Font(FAppStyle::GetFontStyle("SmallFont"))
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

FReply SGPUProgramEditorWidget::OnCompileClicked()
{
	mIsCompiling = true;

	if (mpStatusText.IsValid())
	{
		mpStatusText->SetText(FText::FromString(TEXT("Compiling...")));
		mpStatusText->SetColorAndOpacity(FLinearColor::White);

		CompileProgram();

		return FReply::Handled();
	}
	return FReply::Handled();
}

void SGPUProgramEditorWidget::OnSourceChanged(const FText& NewText)
{
	if (mpProgramAsset.IsValid())
	{
		mpProgramAsset->Modify();
		mpProgramAsset->SetSourceCodeForBackend(GetSelectedBackend(), NewText.ToString());
	}

	SetDirty(true);
	UpdateLineNumbers(NewText.ToString());
}

void SGPUProgramEditorWidget::OnProgramLanguageChanged(TSharedPtr<FString> newSelection,
												   ESelectInfo::Type selectInfo)
{
	FString newSource = mpProgramAsset.IsValid() ? mpProgramAsset->GetSourceCodeForBackend(GetSelectedBackend()) : TEXT("");

	mpSourceEditor->SetText(FText::FromString(newSource));

	UpdateLineNumbers(newSource);
}

FReply SGPUProgramEditorWidget::OnHandleKeyDown(const FGeometry& MyGeometry, 
											const FKeyEvent& InKeyEvent)
{
	if (!mpSourceEditor.IsValid())
		return FReply::Unhandled();

	const FKey key = InKeyEvent.GetKey();
	const bool shiftMod = InKeyEvent.IsShiftDown();
	const bool ctrlMod = InKeyEvent.IsControlDown();

	// Compile Shortcut: Ctrl + Enter
	if (key == EKeys::Enter && ctrlMod)
	{
		return OnCompileClicked();
	}

	if (key == EKeys::Tab)
	{
		InsertTabOrUnindent(shiftMod);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SGPUProgramEditorWidget::CompileProgram()
{
	TWeakPtr<SGPUProgramEditorWidget> weakPtr = SharedThis(this);
	AsyncTask(ENamedThreads::BackgroundThreadPriority, [weakPtr]()
	{
		auto self = weakPtr.Pin();
		if (!self)
			return;

		if (!self->mpProgramAsset.IsValid())
			return;

		FString programSource = self->mpProgramAsset->GetSourceCodeForBackend(self->GetSelectedBackend());

		if (programSource.IsEmpty())
		{
			self->SetCompileResult(false, "Empty Program!");
			return;
		}
		else
		{
			const std::string sourceCode(TCHAR_TO_UTF8(*programSource));

			// Internal Data ------------------------------------------------------
			Gpu::FactoryDesc desc;
			desc.mAllowFallback = false;

			desc.mPreferredBackend = Gpu::Backend::Unknown;
			switch (self->GetSelectedBackend())
			{
				case EGPUBackend::OpenCL:
					desc.mPreferredBackend = Gpu::Backend::OpenCL;
					break;
				case EGPUBackend::CUDA:
					desc.mPreferredBackend = Gpu::Backend::CUDA;
					break;
			}

			std::shared_ptr<Gpu::ICore> core = Gpu::Factory::Create(desc);
			if (!core)
			{
				self->SetCompileResult(false, "Failed to create GPU Core with the selected backend. Please ensure your system supports the selected GPU backend and try again.");
				return;
			}

			std::shared_ptr<Gpu::IDevice> device = core->GetDevice(0);
			if (!device)
			{
				self->SetCompileResult(false, "Failed to create GPU Device with the selected backend. Please ensure your system supports the selected GPU backend and try again.");
				return;
			}

			std::shared_ptr<Gpu::IContext> context = core->CreateContext(device);
			if (!context)
			{
				self->SetCompileResult(false, "Failed to create GPU Context with the selected backend. Please ensure your system supports the selected GPU backend and try again.");
				return;
			}
			// --------------------------------------------------------------------

			std::string buildLog;
			std::shared_ptr<Gpu::IProgram> program = context->CreateProgramFromSource(sourceCode, &buildLog);
			FString compileLog = UTF8_TO_TCHAR(buildLog.c_str());
			if (!program)
			{
				self->SetCompileResult(false, compileLog);
				return;
			}

			self->SetCompileResult(true, compileLog);
		}
	});
}

void SGPUProgramEditorWidget::InsertTabOrUnindent(bool shiftMod)
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

void SGPUProgramEditorWidget::UpdateLineNumbers(const FString& Text)
{
	int32_t NumLines = 1;
	for (TCHAR C : Text)
	{
		if (C == TEXT('\n'))
		{
			++NumLines;
		}
	}

	if (mpLineNumberDisplay.IsValid())
	{
		mpLineNumberDisplay->UpdateLineNumbers(NumLines);
	}
}

void SGPUProgramEditorWidget::SetCompileResult(bool success,
											   const FString& message)
{
	AsyncTask(ENamedThreads::GameThread, [this, success, message]()
	{
		mIsCompiling = false;
		mIsDirty = !success;

		// Update Status Line
		if (mpStatusText.IsValid())
		{
			mpStatusText->SetText(FText::FromString(success ? TEXT("Compiled") : TEXT("Failed")));
			mpStatusText->SetColorAndOpacity(success ? FLinearColor::Green : FLinearColor::Red);
		}

		// Update Error Log
		if (mpLogOutput.IsValid())
		{
			mpLogOutput->SetText(FText::FromString(message));
		}
	});
}

void SGPUProgramEditorWidget::SetDirty(bool isDirty)
{
	mIsDirty = isDirty;

	if (mpStatusText.IsValid() && mIsDirty)
	{
		mpStatusText->SetText(FText::FromString(TEXT("Modified")));
		mpStatusText->SetColorAndOpacity(FLinearColor::Yellow);
	}
}

EGPUBackend SGPUProgramEditorWidget::GetSelectedBackend() const
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