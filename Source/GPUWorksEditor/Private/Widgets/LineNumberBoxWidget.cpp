#include "LineNumberBoxWidget.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBar.h"

#include "Styling/AppStyle.h"

void SLineNumberBox::Construct(const FArguments& arguments)
{
    Font = arguments._Font;

    ChildSlot
    [
        SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
			.Padding(FMargin(6.f, 2.f))
		[
			SAssignNew(mLineScrollBox, SScrollBox)
					   .Orientation(Orient_Vertical)
					   .ExternalScrollbar(arguments._ExternalScrollbar)

			+ SScrollBox::Slot()
			[
				SAssignNew(mLineNumbersText, STextBlock)
						   .Text(FText::GetEmpty())
						   .Font(Font)
						   .Justification(ETextJustify::Right)
						   .MinDesiredWidth(42.f)
						   .ColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f)))
			]
		]
    ];
}


void SLineNumberBox::UpdateLineNumbers(int32_t numLines)
{
    numLines = FMath::Max(1, numLines);
	if (numLines == mCachedLineCount)
		return;

	mCachedLineCount = numLines;

    FString Text;
    Text.Reserve(numLines * 5);

    for (int32 i = 1; i <= numLines; ++i)
    {
        Text += FString::Printf(TEXT("%d\n"), i);
    }

    mLineNumbersText->SetText(FText::FromString(Text));
}