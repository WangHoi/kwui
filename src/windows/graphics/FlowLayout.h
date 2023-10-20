#pragma once

#include "FlowSource.h"
#include "FlowSink.h"
#include "TextAnalysis.h"
#include <string>

namespace windows {
namespace graphics {

class FlowLayout
{
    // This custom layout processes layout in two stages.
    //
    // 1. Analyze the text, given the current font and size
    //      a. Bidirectional analysis
    //      b. Script analysis
    //      c. Number substitution analysis
    //      d. Shape glyphs
    //      e. Intersect run results
    //
    // 2. Fill the text to the given shape
    //      a. Pull next rect from flow source
    //      b. Fit as much text as will go in
    //      c. Push text to flow sink

public:
    struct ClusterPosition
    {
        ClusterPosition()
            : textPosition(),
            runIndex(),
            runEndPosition()
        { }

        UINT32 textPosition;    // Current text position
        UINT32 runIndex;        // Associated analysis run covering this position
        UINT32 runEndPosition;  // Text position where this run ends
    };

public:
    FlowLayout(ComPtr<IDWriteFactory> dwriteFactory)
        : dwriteFactory_(dwriteFactory),
        fontFace_(),
        numberSubstitution_(),
        readingDirection_(DWRITE_READING_DIRECTION_LEFT_TO_RIGHT),
        fontEmSize_(12),
        maxSpaceWidth_(8),
        isTextAnalysisComplete_(false)
    {
    }

    ~FlowLayout()
    {
    }

    HRESULT SetTextFormat(ComPtr<IDWriteTextFormat> textFormat);

    HRESULT SetNumberSubstitution(ComPtr<IDWriteNumberSubstitution> numberSubstitution);

    // Perform analysis on the given text, converting text to glyphs.
    HRESULT AnalyzeText(
        const wchar_t* text,            // [textLength]
        UINT32 textLength
    );

    // Reflow the text analysis into 
    HRESULT FlowText(
        FlowLayoutSource* flowSource,
        FlowLayoutSink* flowSink
    );

protected:
    HRESULT ShapeGlyphRuns(IDWriteTextAnalyzer* textAnalyzer);

    HRESULT ShapeGlyphRun(
        IDWriteTextAnalyzer* textAnalyzer,
        UINT32 runIndex,
        IN OUT UINT32& glyphStart
    );

    HRESULT FitText(
        const ClusterPosition& clusterStart,
        UINT32 textEnd,
        float maxWidth,
        OUT ClusterPosition* clusterEnd
    );

    HRESULT ProduceGlyphRuns(
        FlowLayoutSink* flowSink,
        const FlowLayoutSource::RectF& rect,
        const ClusterPosition& clusterStart,
        const ClusterPosition& clusterEnd
    ) const throw();

    HRESULT ProduceJustifiedAdvances(
        const FlowLayoutSource::RectF& rect,
        const ClusterPosition& clusterStart,
        const ClusterPosition& clusterEnd,
        OUT std::vector<float>& justifiedAdvances
    ) const throw();

    void ProduceBidiOrdering(
        UINT32 spanStart,
        UINT32 spanCount,
        OUT UINT32* spanIndices         // [spanCount]
    ) const throw();

    void SetClusterPosition(
        IN OUT ClusterPosition& cluster,
        UINT32 textPosition
    ) const throw();

    void AdvanceClusterPosition(
        IN OUT ClusterPosition& cluster
    ) const throw();

    UINT32 GetClusterGlyphStart(
        const ClusterPosition& cluster
    ) const throw();

    float GetClusterRangeWidth(
        const ClusterPosition& clusterStart,
        const ClusterPosition& clusterEnd
    ) const throw();

    float GetClusterRangeWidth(
        UINT32 glyphStart,
        UINT32 glyphEnd,
        const float* glyphAdvances      // [glyphEnd]
    ) const throw();

protected:
    ComPtr<IDWriteFactory> dwriteFactory_;

    // Input information.
    std::wstring text_;
    wchar_t localeName_[LOCALE_NAME_MAX_LENGTH];
    DWRITE_READING_DIRECTION readingDirection_;
    ComPtr<IDWriteFontFace> fontFace_;
    ComPtr<IDWriteNumberSubstitution> numberSubstitution_;
    float fontEmSize_;

    // Output text analysis results
    std::vector<TextAnalysis::Run> runs_;
    std::vector<DWRITE_LINE_BREAKPOINT> breakpoints_;
    std::vector<DWRITE_GLYPH_OFFSET> glyphOffsets_;
    std::vector<UINT16> glyphClusters_;
    std::vector<UINT16> glyphIndices_;
    std::vector<float>  glyphAdvances_;

    float maxSpaceWidth_;           // maximum stretch of space allowed for justification
    bool isTextAnalysisComplete_;   // text analysis was done.
};

}
}
