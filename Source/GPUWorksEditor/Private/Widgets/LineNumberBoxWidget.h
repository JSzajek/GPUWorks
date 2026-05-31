#pragma once

#include "Fonts/SlateFontInfo.h"

class SScrollBox;
class STextBlock;

class SLineNumberBox : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLineNumberBox) {}
        SLATE_ARGUMENT(FSlateFontInfo, Font)
        SLATE_ARGUMENT(TSharedPtr<SScrollBar>, ExternalScrollbar)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
public:
    void UpdateLineNumbers(int32_t NumLines);
private:
    int32 CachedLineCount = 0;

    FSlateFontInfo Font;
    TSharedPtr<SScrollBox> LineScrollBox;
    TSharedPtr<STextBlock> LineNumbersText;
};