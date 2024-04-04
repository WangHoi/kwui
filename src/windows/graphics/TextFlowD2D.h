#pragma once

#include "TextAnalysis.h"
#include "graph2d/TextLayout.h"
#include <string>

namespace windows {
namespace graphics {

class TextFlowD2D;

class GlyphRun : public graph2d::GlyphRunInterface
{
public:
	GlyphRun(const TextFlowD2D* flow,
		UINT32 glyphCount,
		const UINT16* glyphIndices, // [glyphCount]
		const float* glyphAdvances, // [glyphCount]
		const DWRITE_GLYPH_OFFSET* glyphOffsets, // [glyphCount]
		ComPtr<IDWriteFontFace> fontFace,
		float fontEmSize,
		UINT8 bidiLevel,
		bool isSideway);
	scene2d::RectF boundingRect() override;
	inline const DWRITE_GLYPH_RUN* raw() const { return &raw_; }

private:
	const TextFlowD2D* flow_;
	DWRITE_GLYPH_RUN raw_;
	std::vector<float> glyph_advances_;
};

class TextFlowD2D : public graph2d::TextFlowInterface
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
	TextFlowD2D(ComPtr<IDWriteFactory> dwriteFactory)
		: dwriteFactory_(dwriteFactory),
		fontFace_(),
		numberSubstitution_(),
		readingDirection_(DWRITE_READING_DIRECTION_LEFT_TO_RIGHT),
		fontEmSize_(12),
		maxSpaceWidth_(8),
		isTextAnalysisComplete_(false)
	{
	}

	~TextFlowD2D()
	{
	}

	inline ComPtr<IDWriteTextFormat> textFormat() const
	{
		return text_format_;
	}
	inline const std::wstring& text() const
	{
		return text_;
	}
	HRESULT setTextFormat(ComPtr<IDWriteTextFormat> textFormat);

	HRESULT SetNumberSubstitution(ComPtr<IDWriteNumberSubstitution> numberSubstitution);

	// Perform analysis on the given text, converting text to glyphs.
	HRESULT AnalyzeText(
		const wchar_t* text,            // [textLength]
		UINT32 textLength
	);

	style::FontMetrics fontMetrics() override;
	std::tuple<float, float> measureWidth() override;

	// Reflow the text analysis into 
	void flowText(
		graph2d::TextFlowSourceInterface* flowSource,
		graph2d::TextFlowSinkInterface* flowSink
	) override;

protected:
	HRESULT ShapeGlyphRuns(IDWriteTextAnalyzer* textAnalyzer);

	HRESULT ShapeGlyphRun(
		IDWriteTextAnalyzer* textAnalyzer,
		UINT32 runIndex,
		IN OUT UINT32& glyphStart
	);

	bool FitText(
		const ClusterPosition& clusterStart,
		UINT32 textEnd,
		float maxWidth,
		bool empty_line,
		OUT ClusterPosition* clusterEnd
	);

	HRESULT ProduceGlyphRuns(
		graph2d::TextFlowSinkInterface* flowSink,
		style::LineBox* line,
		const scene2d::RectF& rect,
		const ClusterPosition& clusterStart,
		const ClusterPosition& clusterEnd
	) const throw();

	HRESULT ProduceJustifiedAdvances(
		const scene2d::RectF& rect,
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
	ComPtr<IDWriteTextFormat> text_format_;
	std::wstring text_;
	absl::optional<float> line_height_;
	wchar_t localeName_[LOCALE_NAME_MAX_LENGTH];
	style::FontMetrics font_metrics_;
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

	friend class GlyphRun;
};

}
}
