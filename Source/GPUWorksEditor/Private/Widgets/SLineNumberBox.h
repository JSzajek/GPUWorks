#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Fonts/SlateFontInfo.h"

class SLineNumberBox : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLineNumberBox) {}
        SLATE_ARGUMENT(FSlateFontInfo, Font)
        SLATE_ARGUMENT(TSharedPtr<SScrollBar>, ExternalScrollbar)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        Font = InArgs._Font;

        ChildSlot
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
                .Margin(FMargin(4.f, 2.f))
            ]
        ];
    }

    void UpdateLineNumbers(int32 NumLines)
    {
        NumLines = FMath::Max(1, NumLines);

        FString Text;
        Text.Reserve(NumLines * 4);

        for (int32 i = 1; i <= NumLines; ++i)
        {
            Text += FString::Printf(TEXT("%d\n"), i);
        }

        LineNumbersText->SetText(FText::FromString(Text));
    }
private:
    FSlateFontInfo Font;
    TSharedPtr<SScrollBox> LineScrollBox;
    TSharedPtr<STextBlock> LineNumbersText;
};