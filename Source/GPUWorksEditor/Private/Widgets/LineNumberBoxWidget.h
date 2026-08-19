#pragma once

#include "Fonts/SlateFontInfo.h"

class SScrollBox;
class STextBlock;

/// <summary>
/// Line number box widget, which is used to display line numbers in the shader editor.
/// </summary>
class SLineNumberBox : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLineNumberBox) {}
        SLATE_ARGUMENT(FSlateFontInfo, Font)
        SLATE_ARGUMENT(TSharedPtr<SScrollBar>, ExternalScrollbar)
    SLATE_END_ARGS()

    /// <summary>
    /// Constructs the line number box widget with the specified arguments.
    /// </summary>
    /// <param name="arguments">The arguments</param>
    void Construct(const FArguments& arguments);
public:
    /// <summary>
	/// Updates the line numbers displayed in the widget based on the specified number of lines.
    /// </summary>
    /// <param name="numLines">The number of lines to display</param>
    void UpdateLineNumbers(int32_t numLines);
private:
    int32 mCachedLineCount = 0;

    FSlateFontInfo Font;
    TSharedPtr<SScrollBox> mLineScrollBox;
    TSharedPtr<STextBlock> mLineNumbersText;
};