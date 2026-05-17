#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"

class SLineNumberBox : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLineNumberBox) 
    {
    }

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        ChildSlot
        [
            SAssignNew(mLineNumbersText, STextBlock).Text(FText::FromString(""))
                                                    .Justification(ETextJustify::Right)
                                                    .MinDesiredWidth(20)
        ];
    }

    void UpdateLineNumbers(int32 NumLines)
    {
        FString LineNumString;
        for (int32 i = 1; i <= NumLines; ++i)
            LineNumString += FString::Printf(TEXT("%d\n"), i);

        mLineNumbersText->SetText(FText::FromString(LineNumString));
    }
private:
    TSharedPtr<STextBlock> mLineNumbersText;
};