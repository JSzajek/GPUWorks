#include "LineNumberBoxWidget.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBar.h"

#include "Styling/AppStyle.h"

void SLineNumberBox::Construct(const FArguments& InArgs)
{
    Font = InArgs._Font;

    ChildSlot
    [
        SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
			.Padding(FMargin(6.f, 2.f))
		[
			SAssignNew(LineScrollBox, SScrollBox)
					   .Orientation(Orient_Vertical)
					   .ExternalScrollbar(InArgs._ExternalScrollbar)

			+ SScrollBox::Slot()
			[
				SAssignNew(LineNumbersText, STextBlock)
						   .Text(FText::GetEmpty())
						   .Font(Font)
						   .Justification(ETextJustify::Right)
						   .MinDesiredWidth(42.f)
						   .ColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f)))
			]
		]
    ];
}


void SLineNumberBox::UpdateLineNumbers(int32_t NumLines)
{
    NumLines = FMath::Max(1, NumLines);

	if (NumLines == CachedLineCount)
		return;

	CachedLineCount = NumLines;

    FString Text;
    Text.Reserve(NumLines * 5);

    for (int32 i = 1; i <= NumLines; ++i)
    {
        Text += FString::Printf(TEXT("%d\n"), i);
    }

    LineNumbersText->SetText(FText::FromString(Text));
}