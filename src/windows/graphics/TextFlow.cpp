#include "TextFlow.h"
#include "TextAnalysis.h"
#include "graph2d/graph2d.h"
#include "base/log.h"
#include "windows/EncodingManager.h"
#include "windows/graphics/GraphicDevice.h"
#include <numeric>

namespace windows {
namespace graphics {

namespace
{
// Estimates the maximum number of glyph indices needed to hold a string of 
// a given length.  This is the formula given in the Uniscribe SDK and should
// cover most cases. Degenerate cases will require a reallocation.
UINT32 EstimateGlyphCount(UINT32 textLength)
{
	return 3 * textLength / 2 + 16;
}
}

GlyphRun::GlyphRun(const TextFlow* flow,
	UINT32 glyphCount,
	const UINT16* glyphIndices,
	const float* glyphAdvances,
	const DWRITE_GLYPH_OFFSET* glyphOffsets,
	ComPtr<IDWriteFontFace> fontFace,
	float fontEmSize,
	UINT8 bidiLevel,
	bool isSideway)
	: flow_(flow)
{
	glyph_advances_.assign(glyphAdvances, glyphAdvances + glyphCount);

	raw_.glyphIndices = glyphIndices;
	raw_.glyphAdvances = glyph_advances_.data();
	raw_.glyphOffsets = glyphOffsets;
	raw_.glyphCount = glyphCount;
	raw_.fontEmSize = fontEmSize;
	raw_.fontFace = fontFace.Get();
	raw_.bidiLevel = bidiLevel;
	raw_.isSideways = isSideway;
}

scene2d::RectF GlyphRun::boundingRect()
{
	scene2d::RectF rect;
	rect.bottom = flow_->font_metrics_.lineHeight();
	rect.right = std::accumulate(raw_.glyphAdvances, raw_.glyphAdvances + raw_.glyphCount, 0.0f);
	return rect;
}

HRESULT TextFlow::setTextFormat(ComPtr<IDWriteTextFormat> textFormat)
{
	text_format_ = textFormat;
	// Initializes properties using a text format, like font family, font size,
	// and reading direction. For simplicity, this custom layout supports
	// minimal formatting. No mixed formatting or alignment modes are supported.
	HRESULT hr = S_OK;

	ComPtr<IDWriteFontCollection> fontCollection;
	ComPtr<IDWriteFontFamily> fontFamily;
	ComPtr<IDWriteFont> font;

	wchar_t fontFamilyName[100];

	readingDirection_ = textFormat->GetReadingDirection();
	fontEmSize_ = textFormat->GetFontSize();

	hr = textFormat->GetLocaleName(localeName_, ARRAYSIZE(localeName_));

	////////////////////
	// Map font and style to fontFace.

	if (SUCCEEDED(hr))
	{
		// Need the font collection to map from font name to actual font.
		textFormat->GetFontCollection(fontCollection.GetAddressOf());
		if (fontCollection == NULL)
		{
			// No font collection was set in the format, so use the system default.
			hr = dwriteFactory_->GetSystemFontCollection(fontCollection.GetAddressOf());
		}
	}

	// Find matching family name in collection.
	if (SUCCEEDED(hr))
	{
		hr = textFormat->GetFontFamilyName(fontFamilyName, ARRAYSIZE(fontFamilyName));
	}

	UINT32 fontIndex = 0;
	if (SUCCEEDED(hr))
	{
		BOOL fontExists = false;
		hr = fontCollection->FindFamilyName(fontFamilyName, &fontIndex, &fontExists);
		if (!fontExists)
		{
			// If the given font does not exist, take what we can get
			// (displaying something instead nothing), choosing the foremost
			// font in the collection.
			fontIndex = 0;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = fontCollection->GetFontFamily(fontIndex, fontFamily.GetAddressOf());
	}

	if (SUCCEEDED(hr))
	{
		hr = fontFamily->GetFirstMatchingFont(
			textFormat->GetFontWeight(),
			textFormat->GetFontStretch(),
			textFormat->GetFontStyle(),
			font.GetAddressOf()
		);
	}

	if (SUCCEEDED(hr))
	{
		hr = font->CreateFontFace(fontFace_.GetAddressOf());
	}
	DWRITE_FONT_METRICS fm;
	fontFace_->GetMetrics(&fm);
	const float factor = fontEmSize_ / fm.designUnitsPerEm;
	font_metrics_.ascent = fm.ascent * factor;
	font_metrics_.descent = fm.descent * factor;
	font_metrics_.line_gap = fm.lineGap * factor;
	font_metrics_.cap_height = fm.capHeight * factor;
	font_metrics_.x_height = fm.xHeight * factor;

	return S_OK;
}

HRESULT TextFlow::SetNumberSubstitution(ComPtr<IDWriteNumberSubstitution> numberSubstitution)
{
	numberSubstitution_ = numberSubstitution;

	return S_OK;
}


HRESULT TextFlow::AnalyzeText(
	const wchar_t* text,                // [textLength]
	UINT32 textLength
) throw()
{
	// Analyzes the given text and keeps the results for later reflow.

	isTextAnalysisComplete_ = false;

	if (fontFace_ == NULL)
		return E_FAIL; // Need a font face to determine metrics.

	HRESULT hr = S_OK;

	try
	{
		text_.assign(text, textLength);
	} catch (...)
	{
		hr = E_FAIL;
	}

	// Query for the text analyzer's interface.
	ComPtr<IDWriteTextAnalyzer> textAnalyzer;
	if (SUCCEEDED(hr))
	{
		hr = dwriteFactory_->CreateTextAnalyzer(textAnalyzer.GetAddressOf());
	}

	// Record the analyzer's results.
	if (SUCCEEDED(hr))
	{
		ComPtr<TextAnalysis> textAnalysis;
		hr = WRL::MakeAndInitialize<TextAnalysis>(
			&textAnalysis,
			text, textLength,
			localeName_,
			numberSubstitution_.Get(),
			readingDirection_);
		hr = textAnalysis->GenerateResults(textAnalyzer.Get(), runs_, breakpoints_);
	}

	// Convert the entire text to glyphs.
	if (SUCCEEDED(hr))
	{
		hr = ShapeGlyphRuns(textAnalyzer.Get());
	}

	if (SUCCEEDED(hr))
	{
		isTextAnalysisComplete_ = true;
	}

	return hr;
}


HRESULT TextFlow::ShapeGlyphRuns(IDWriteTextAnalyzer* textAnalyzer)
{
	// Shapes all the glyph runs in the layout.

	HRESULT hr = S_OK;

	UINT32 textLength = static_cast<UINT32>(text_.size());

	// Estimate the maximum number of glyph indices needed to hold a string.
	UINT32 estimatedGlyphCount = EstimateGlyphCount(textLength);

	try
	{
		glyphIndices_.resize(estimatedGlyphCount);
		glyphOffsets_.resize(estimatedGlyphCount);
		glyphAdvances_.resize(estimatedGlyphCount);
		glyphClusters_.resize(textLength);

		UINT32 glyphStart = 0;

		// Shape each run separately. This is needed whenever script, locale,
		// or reading direction changes.
		for (UINT32 runIndex = 0; runIndex < runs_.size(); ++runIndex)
		{
			hr = ShapeGlyphRun(textAnalyzer, runIndex, glyphStart);
			if (FAILED(hr))
				break;
		}

		glyphIndices_.resize(glyphStart);
		glyphOffsets_.resize(glyphStart);
		glyphAdvances_.resize(glyphStart);
	} catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}


HRESULT TextFlow::ShapeGlyphRun(
	IDWriteTextAnalyzer* textAnalyzer,
	UINT32 runIndex,
	IN OUT UINT32& glyphStart
)
{
	// Shapes a single run of text into glyphs.
	// Alternately, you could iteratively interleave shaping and line
	// breaking to reduce the number glyphs held onto at once. It's simpler
	// for this demostration to just do shaping and line breaking as two
	// separate processes, but realize that this does have the consequence that
	// certain advanced fonts containing line specific features (like Gabriola)
	// will shape as if the line is not broken.

	HRESULT hr = S_OK;

	try
	{
		TextAnalysis::Run& run = runs_[runIndex];
		UINT32 textStart = run.textStart;
		UINT32 textLength = run.textLength;
		UINT32 maxGlyphCount = static_cast<UINT32>(glyphIndices_.size() - glyphStart);
		UINT32 actualGlyphCount = 0;

		run.glyphStart = glyphStart;
		run.glyphCount = 0;

		if (textLength == 0)
			return S_OK; // Nothing to do..

		HRESULT hr = S_OK;

		////////////////////
		// Allocate space for shaping to fill with glyphs and other information,
		// with about as many glyphs as there are text characters. We'll actually
		// need more glyphs than codepoints if they are decomposed into separate
		// glyphs, or fewer glyphs than codepoints if multiple are substituted
		// into a single glyph. In any case, the shaping process will need some
		// room to apply those rules to even make that determintation.

		if (textLength > maxGlyphCount)
		{
			maxGlyphCount = EstimateGlyphCount(textLength);
			UINT32 totalGlyphsArrayCount = glyphStart + maxGlyphCount;
			glyphIndices_.resize(totalGlyphsArrayCount);
		}

		std::vector<DWRITE_SHAPING_TEXT_PROPERTIES>  textProps(textLength);
		std::vector<DWRITE_SHAPING_GLYPH_PROPERTIES> glyphProps(maxGlyphCount);

		////////////////////
		// Get the glyphs from the text, retrying if needed.

		int tries = 0;
		do
		{
			hr = textAnalyzer->GetGlyphs(
				&text_[textStart],
				textLength,
				fontFace_.Get(),
				run.isSideways,         // isSideways,
				(run.bidiLevel & 1),    // isRightToLeft
				&run.script,
				localeName_,
				(run.isNumberSubstituted) ? numberSubstitution_.Get() : NULL,
				NULL,                   // features
				NULL,                   // featureLengths
				0,                      // featureCount
				maxGlyphCount,          // maxGlyphCount
				&glyphClusters_[textStart],
				&textProps[0],
				&glyphIndices_[glyphStart],
				&glyphProps[0],
				&actualGlyphCount
			);
			tries++;

			if (hr == E_NOT_SUFFICIENT_BUFFER)
			{
				// Try again using a larger buffer.
				maxGlyphCount = EstimateGlyphCount(maxGlyphCount);
				UINT32 totalGlyphsArrayCount = glyphStart + maxGlyphCount;

				glyphProps.resize(maxGlyphCount);
				glyphIndices_.resize(totalGlyphsArrayCount);
			} else
			{
				break;
			}
		} while (tries < 2); // We'll give it two chances.

		if (FAILED(hr))
			return hr;

		////////////////////
		// Get the placement of the all the glyphs.

		glyphAdvances_.resize(std::max(static_cast<size_t>(glyphStart + actualGlyphCount), glyphAdvances_.size()));
		glyphOffsets_.resize(std::max(static_cast<size_t>(glyphStart + actualGlyphCount), glyphOffsets_.size()));

		hr = textAnalyzer->GetGlyphPlacements(
			&text_[textStart],
			&glyphClusters_[textStart],
			&textProps[0],
			textLength,
			&glyphIndices_[glyphStart],
			&glyphProps[0],
			actualGlyphCount,
			fontFace_.Get(),
			fontEmSize_,
			run.isSideways,
			(run.bidiLevel & 1),    // isRightToLeft
			&run.script,
			localeName_,
			NULL,                   // features
			NULL,                   // featureRangeLengths
			0,                      // featureRanges
			&glyphAdvances_[glyphStart],
			&glyphOffsets_[glyphStart]
		);

		if (FAILED(hr))
			return hr;

		////////////////////
		// Certain fonts, like Batang, contain glyphs for hidden control
		// and formatting characters. So we'll want to explicitly force their
		// advance to zero.
		if (run.script.shapes & DWRITE_SCRIPT_SHAPES_NO_VISUAL)
		{
			std::fill(glyphAdvances_.begin() + glyphStart,
				glyphAdvances_.begin() + glyphStart + actualGlyphCount,
				0.0f
			);
		}

		////////////////////
		// Set the final glyph count of this run and advance the starting glyph.
		run.glyphCount = actualGlyphCount;
		glyphStart += actualGlyphCount;
	} catch (...)
	{
		hr = E_FAIL;
	}

	return hr;
}

style::FontMetrics TextFlow::fontMetrics()
{
	return font_metrics_;
}

std::tuple<float, float> TextFlow::measureWidth()
{
	if (!isTextAnalysisComplete_)
		return std::tuple<float, float>(0.0f, std::numeric_limits<float>::max());

	ClusterPosition min_start_cluster, max_start_cluster, next_cluster;
	SetClusterPosition(min_start_cluster, 0);
	next_cluster = max_start_cluster = min_start_cluster;
	UINT32 text_length = (UINT32)text_.length();
	float min_text_width = 0.0f;
	float max_text_width = 0.0f;
	while (next_cluster.textPosition < text_length) {
		AdvanceClusterPosition(next_cluster);
		const DWRITE_LINE_BREAKPOINT breakpoint = breakpoints_[next_cluster.textPosition - 1];
		// Update min width
		if (breakpoint.breakConditionAfter != DWRITE_BREAK_CONDITION_MAY_NOT_BREAK) {
			float text_width = GetClusterRangeWidth(min_start_cluster, next_cluster);
			min_text_width = std::max(min_text_width, text_width);
			min_start_cluster = next_cluster;
		}
		// Update max width
		if (breakpoint.breakConditionAfter == DWRITE_BREAK_CONDITION_MUST_BREAK) {
			float text_width = GetClusterRangeWidth(max_start_cluster, next_cluster);
			max_text_width = std::max(max_text_width, text_width);
			max_start_cluster = next_cluster;
		}
	}
	if (min_start_cluster.textPosition < next_cluster.textPosition) {
		float text_width = GetClusterRangeWidth(min_start_cluster, next_cluster);
		min_text_width = std::max(min_text_width, text_width);
	}
	if (max_start_cluster.textPosition < next_cluster.textPosition) {
		float text_width = GetClusterRangeWidth(max_start_cluster, next_cluster);
		max_text_width = std::max(max_text_width, text_width);
	}

	// LOG(INFO) << "measure " << min_text_width << "/" << max_text_width;
	return std::make_tuple(min_text_width, max_text_width);
}
void TextFlow::flowText(graph2d::TextFlowSourceInterface* flowSource, graph2d::TextFlowSinkInterface* flowSink)
{
	// Reflow all the text, from source to sink.

	if (!isTextAnalysisComplete_)
		return;

	HRESULT hr = S_OK;

	// Determine the font line height, needed by the flow source.
	DWRITE_FONT_METRICS fontMetrics = {};
	fontFace_->GetMetrics(&fontMetrics);

	float fontHeight = (fontMetrics.ascent + fontMetrics.descent + fontMetrics.lineGap)
		* fontEmSize_ / fontMetrics.designUnitsPerEm;

	// Get ready for series of glyph runs.
	flowSink->prepare(glyphIndices_.size());

	if (SUCCEEDED(hr))
	{
		// Set initial cluster position to beginning of text.
		ClusterPosition cluster, nextCluster;
		SetClusterPosition(cluster, 0);

		scene2d::RectF rect;
		UINT32 textLength = static_cast<UINT32>(text_.size());

		// Iteratively pull rect's from the source,
		// and push as much text will fit to the sink.
		float first = true;
		while (cluster.textPosition < textLength)
		{
			// Pull the next rect from the source.
			style::LineBox* line = nullptr;
			float width;
			bool empty_line;
			if (first) {
				line = flowSource->getCurrentLine(fontHeight, rect.left, width, empty_line);
			} else {
				line = flowSource->getNextLine(fontHeight, rect.left, width);
				empty_line = true;
			}
			first = false;

			//LOG(INFO) << "nextLine:"
			//	<< " left=" << rect.left
			//	<< ", width=" << width
			//	<< ", empty=" << empty_line;
			rect.right = rect.left + width;

			if (rect.right - rect.left <= 0)
				break; // Stop upon reaching zero sized rects.

			// Fit as many clusters between breakpoints that will go in.
			bool overflow = FitText(cluster, textLength, rect.right - rect.left, empty_line, &nextCluster);
			std::wstring seg = text_.substr(cluster.textPosition, nextCluster.textPosition - cluster.textPosition);
			// LOG(INFO) << "text fit [" << windows::EncodingManager::WideToUTF8(seg) << "] overflow=" << overflow;

			// Check overflow
			if (!empty_line && overflow) {
				// LOG(INFO) << "retry fit text";
				continue;
			}

			// Push the glyph runs to the sink.
			if (FAILED(ProduceGlyphRuns(flowSink, line, rect, cluster, nextCluster)))
				break;

			cluster = nextCluster;
		}
	}
}


bool TextFlow::FitText(
	const ClusterPosition& clusterStart,
	UINT32 textEnd,
	float maxWidth,
	bool empty_line,
	OUT ClusterPosition* clusterEnd
)
{
	// Fits as much text as possible into the given width,
	// using the clusters and advances returned by DWrite.

	////////////////////////////////////////
	// Set the starting cluster to the starting text position,
	// and continue until we exceed the maximum width or hit
	// a hard break.
	ClusterPosition cluster(clusterStart);
	ClusterPosition nextCluster(clusterStart);
	UINT32 validBreakPosition = cluster.textPosition;
	UINT32 bestBreakPosition = cluster.textPosition;
	float textWidth = 0;
	float best_break_text_width = 0;
	bool overflow = false;

	while (cluster.textPosition < textEnd)
	{
		// Use breakpoint information to find where we can safely break words.
		AdvanceClusterPosition(nextCluster);
		const DWRITE_LINE_BREAKPOINT breakpoint = breakpoints_[nextCluster.textPosition - 1];

		// Check whether we exceeded the amount of text that can fit,
		// unless it's whitespace, which we allow to flow beyond the end.
		
		float cluster_width = GetClusterRangeWidth(cluster, nextCluster);
		textWidth += cluster_width;
 		if (textWidth > maxWidth && !breakpoint.isWhitespace)
		{
			// Want a best cluster break.
			if (bestBreakPosition > clusterStart.textPosition) {
				break;
			}
		}

		validBreakPosition = nextCluster.textPosition;

		// See if we can break after this character cluster, and if so,
		// mark it as the new potential break point.
		if (breakpoint.breakConditionAfter != DWRITE_BREAK_CONDITION_MAY_NOT_BREAK)
		{
			bestBreakPosition = validBreakPosition;
			best_break_text_width = textWidth;
			if (breakpoint.breakConditionAfter == DWRITE_BREAK_CONDITION_MUST_BREAK)
				break; // we have a hard return, so we've fit all we can.
		}
		cluster = nextCluster;
	}

	overflow = (best_break_text_width > maxWidth);

	////////////////////////////////////////
	// Want last best position that didn't break a word, but if that's not available,
	// fit at least one cluster (emergency line breaking).
	if (bestBreakPosition == clusterStart.textPosition)
		bestBreakPosition = validBreakPosition;

	SetClusterPosition(cluster, bestBreakPosition);

	*clusterEnd = cluster;

	return overflow;
}


HRESULT TextFlow::ProduceGlyphRuns(
	graph2d::TextFlowSinkInterface* flowSink,
	style::LineBox* line,
	const scene2d::RectF& rect,
	const ClusterPosition& clusterStart,
	const ClusterPosition& clusterEnd
) const throw()
{
	// Produce a series of glyph runs from the given range
	// and send them to the sink. If the entire text fit
	// into the rect, then we'll only pass on a single glyph
	// run.

	HRESULT hr = S_OK;

	////////////////////////////////////////
	// Figure out how many runs we cross, because this is the number
	// of distinct glyph runs we'll need to reorder visually.

	UINT32 runIndexEnd = clusterEnd.runIndex;
	if (clusterEnd.textPosition > runs_[runIndexEnd].textStart)
		++runIndexEnd; // Only partially cover the run, so round up.

	// Fill in mapping from visual run to logical sequential run.
	UINT32 bidiOrdering[100];
	UINT32 totalRuns = runIndexEnd - clusterStart.runIndex;
	totalRuns = std::min(totalRuns, static_cast<UINT32>(ARRAYSIZE(bidiOrdering)));

	ProduceBidiOrdering(
		clusterStart.runIndex,
		totalRuns,
		&bidiOrdering[0]
	);

	// No whitespace trimming
	ClusterPosition clusterWsEnd(clusterEnd);

	////////////////////////////////////////
	// Produce justified advances to reduce the jagged edge.

	std::vector<float> justifiedAdvances;
	hr = ProduceJustifiedAdvances(rect, clusterStart, clusterWsEnd, justifiedAdvances);
	UINT32 justificationGlyphStart = GetClusterGlyphStart(clusterStart);


	////////////////////////////////////////
	// Determine starting point for alignment.

	float x = rect.left;
	float y = rect.bottom;

	if (SUCCEEDED(hr))
	{
		DWRITE_FONT_METRICS fontMetrics;
		fontFace_->GetMetrics(&fontMetrics);

		float descent = (fontMetrics.descent * fontEmSize_ / fontMetrics.designUnitsPerEm);
		y -= descent;

		if (readingDirection_ == DWRITE_READING_DIRECTION_RIGHT_TO_LEFT)
		{
			// For RTL, we neeed the run width to adjust the origin
			// so it starts on the right side.
			UINT32 glyphStart = GetClusterGlyphStart(clusterStart);
			UINT32 glyphEnd = GetClusterGlyphStart(clusterWsEnd);

			if (glyphStart < glyphEnd)
			{
				float lineWidth = GetClusterRangeWidth(
					glyphStart - justificationGlyphStart,
					glyphEnd - justificationGlyphStart,
					&justifiedAdvances.front()
				);
				x = rect.right - lineWidth;
			}
		}
	}

	////////////////////////////////////////
	// Send each glyph run to the sink.

	if (SUCCEEDED(hr))
	{
		for (size_t i = 0; i < totalRuns; ++i)
		{
			const TextAnalysis::Run& run = runs_[bidiOrdering[i]];
			UINT32 glyphStart = run.glyphStart;
			UINT32 glyphEnd = glyphStart + run.glyphCount;

			// If the run is only partially covered, we'll need to find
			// the subsection of glyphs that were fit.
			if (clusterStart.textPosition > run.textStart)
			{
				glyphStart = GetClusterGlyphStart(clusterStart);
			}
			if (clusterWsEnd.textPosition < run.textStart + run.textLength)
			{
				glyphEnd = GetClusterGlyphStart(clusterWsEnd);
			}
			if ((glyphStart >= glyphEnd)
				|| (run.script.shapes & DWRITE_SCRIPT_SHAPES_NO_VISUAL))
			{
				// The itemizer told us not to draw this character,
				// either because it was a formatting, control, or other hidden character.
				continue;
			}

			// The run width is needed to know how far to move forward,
			// and to flip the origin for right-to-left text.
			float runWidth = GetClusterRangeWidth(
				glyphStart - justificationGlyphStart,
				glyphEnd - justificationGlyphStart,
				&justifiedAdvances.front()
			);

			// Flush this glyph run.
			std::unique_ptr<GlyphRun> glyph_run = std::make_unique<GlyphRun>(
				this,
				glyphEnd - glyphStart,
				&glyphIndices_[glyphStart],
				&justifiedAdvances[glyphStart - justificationGlyphStart],
				&glyphOffsets_[glyphStart],
				fontFace_,
				fontEmSize_,
				run.bidiLevel,
				run.isSideways);
			scene2d::PointF pos(
				(run.bidiLevel & 1) ? (x + runWidth) : (x), // origin starts from right if RTL
				y);
			flowSink->addGlyphRun(line, pos, std::move(glyph_run));

			x += runWidth;
		}
	}

	return hr;
}


void TextFlow::ProduceBidiOrdering(
	UINT32 spanStart,
	UINT32 spanCount,
	OUT UINT32* spanIndices     // [spanCount]
) const throw()
{
	// Produces an index mapping from sequential order to visual bidi order.
	// The function progresses forward, checking the bidi level of each
	// pair of spans, reversing when needed.
	//
	// See the Unicode technical report 9 for an explanation.
	// http://www.unicode.org/reports/tr9/tr9-17.html 

	// Fill all entries with initial indices
	for (UINT32 i = 0; i < spanCount; ++i)
	{
		spanIndices[i] = spanStart + i;
	}

	if (spanCount <= 1)
		return;

	size_t runStart = 0;
	UINT32 currentLevel = runs_[spanStart].bidiLevel;

	// Rearrange each run to produced reordered spans.
	for (size_t i = 0; i < spanCount; ++i)
	{
		size_t runEnd = i + 1;
		UINT32 nextLevel = (runEnd < spanCount)
			? runs_[spanIndices[runEnd]].bidiLevel
			: 0; // past last element

		// We only care about transitions, particularly high to low,
		// because that means we have a run behind us where we can
		// do something.

		if (currentLevel <= nextLevel) // This is now the beginning of the next run.
		{
			if (currentLevel < nextLevel)
			{
				currentLevel = nextLevel;
				runStart = i + 1;
			}
			continue; // Skip past equal levels, or increasing stairsteps.
		}

		do // currentLevel > nextLevel
		{
			// Recede to find start of the run and previous level.
			UINT32 previousLevel;
			for (;;)
			{
				if (runStart <= 0) // reached front of index list
				{
					previousLevel = 0; // position before string has bidi level of 0
					break;
				}
				if (runs_[spanIndices[--runStart]].bidiLevel < currentLevel)
				{
					previousLevel = runs_[spanIndices[runStart]].bidiLevel;
					++runStart; // compensate for going one element past
					break;
				}
			}

			// Reverse the indices, if the difference between the current and
			// next/previous levels is odd. Otherwise, it would be a no-op, so
			// don't bother.
			if ((std::min(currentLevel - nextLevel, currentLevel - previousLevel) & 1) != 0)
			{
				std::reverse(spanIndices + runStart, spanIndices + runEnd);
			}

			// Descend to the next lower level, the greater of previous and next
			currentLevel = std::max(previousLevel, nextLevel);
		} while (currentLevel > nextLevel); // Continue until completely flattened.
	}
}


HRESULT TextFlow::ProduceJustifiedAdvances(
	const scene2d::RectF& rect,
	const ClusterPosition& clusterStart,
	const ClusterPosition& clusterEnd,
	OUT std::vector<float>& justifiedAdvances
) const throw()
{
	// Performs simple inter-word justification
	// using the breakpoint analysis whitespace property.

	// Copy out default, unjustified advances.
	UINT32 glyphStart = GetClusterGlyphStart(clusterStart);
	UINT32 glyphEnd = GetClusterGlyphStart(clusterEnd);

	try
	{
		justifiedAdvances.assign(glyphAdvances_.begin() + glyphStart, glyphAdvances_.begin() + glyphEnd);
	} catch (...)
	{
		return E_FAIL;
	}

	// skip
	return S_OK;

	if (glyphEnd - glyphStart == 0)
		return S_OK; // No glyphs to modify.

	float maxWidth = rect.right - rect.left;
	if (maxWidth <= 0)
		return S_OK; // Text can't fit anyway.


	////////////////////////////////////////
	// First, count how many spaces there are in the text range.

	ClusterPosition cluster(clusterStart);
	UINT32 whitespaceCount = 0;

	while (cluster.textPosition < clusterEnd.textPosition)
	{
		if (breakpoints_[cluster.textPosition].isWhitespace)
			++whitespaceCount;
		AdvanceClusterPosition(cluster);
	}
	if (whitespaceCount <= 0)
		return S_OK; // Can't justify using spaces, since none exist.


	////////////////////////////////////////
	// Second, determine the needed contribution to each space.

	float lineWidth = GetClusterRangeWidth(glyphStart, glyphEnd, &glyphAdvances_.front());
	float justificationPerSpace = (maxWidth - lineWidth) / whitespaceCount;

	if (justificationPerSpace <= 0)
		return S_OK; // Either already justified or would be negative justification.

	if (justificationPerSpace > maxSpaceWidth_)
		return S_OK; // Avoid justification if it would space the line out awkwardly far.


	////////////////////////////////////////
	// Lastly, adjust the advance widths, adding the difference to each space character.

	cluster = clusterStart;
	while (cluster.textPosition < clusterEnd.textPosition)
	{
		if (breakpoints_[cluster.textPosition].isWhitespace)
			justifiedAdvances[GetClusterGlyphStart(cluster) - glyphStart] += justificationPerSpace;

		AdvanceClusterPosition(cluster);
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
// Text/cluster navigation.
//
// Since layout should never split text clusters, we want to move ahead whole
// clusters at a time.

void TextFlow::SetClusterPosition(
	IN OUT ClusterPosition& cluster,
	UINT32 textPosition
) const throw()
{
	// Updates the current position and seeks its matching text analysis run.

	cluster.textPosition = textPosition;

	// If the new text position is outside the previous analysis run,
	// find the right one.

	if (textPosition >= cluster.runEndPosition
		|| !runs_[cluster.runIndex].ContainsTextPosition(textPosition))
	{
		// If we can resume the search from the previous run index,
		// (meaning the new text position comes after the previously
		// seeked one), we can save some time. Otherwise restart from
		// the beginning.

		UINT32 newRunIndex = 0;
		if (textPosition >= runs_[cluster.runIndex].textStart)
		{
			newRunIndex = cluster.runIndex;
		}

		// Find new run that contains the text position.
		newRunIndex = static_cast<UINT32>(
			std::find(runs_.begin() + newRunIndex, runs_.end(), textPosition)
			- runs_.begin()
			);

		// Keep run index within the list, rather than pointing off the end.
		if (newRunIndex >= runs_.size())
		{
			newRunIndex = static_cast<UINT32>(runs_.size() - 1);
		}

		// Cache the position of the next analysis run to efficiently
		// move forward in the clustermap.
		const TextAnalysis::Run& matchingRun = runs_[newRunIndex];
		cluster.runIndex = newRunIndex;
		cluster.runEndPosition = matchingRun.textStart + matchingRun.textLength;
	}
}


void TextFlow::AdvanceClusterPosition(
	IN OUT ClusterPosition& cluster
) const throw()
{
	// Looks forward in the cluster map until finding a new cluster,
	// or until we reach the end of a cluster run returned by shaping.
	//
	// Glyph shaping can produce a clustermap where a:
	//  - A single codepoint maps to a single glyph (simple Latin and precomposed CJK)
	//  - A single codepoint to several glyphs (diacritics decomposed into distinct glyphs)
	//  - Multiple codepoints are coalesced into a single glyph.
	//
	UINT32 textPosition = cluster.textPosition;
	UINT32 clusterId = glyphClusters_[textPosition];

	for (++textPosition; textPosition < cluster.runEndPosition; ++textPosition)
	{
		if (glyphClusters_[textPosition] != clusterId)
		{
			// Now pointing to the next cluster.
			cluster.textPosition = textPosition;
			return;
		}
	}
	if (textPosition == cluster.runEndPosition)
	{
		// We crossed a text analysis run.
		SetClusterPosition(cluster, textPosition);
	}
}


UINT32 TextFlow::GetClusterGlyphStart(const ClusterPosition& cluster) const throw()
{
	// Maps from text position to corresponding starting index in the glyph array.
	// This is needed because there isn't a 1:1 correspondence between text and
	// glyphs produced.

	UINT32 glyphStart = runs_[cluster.runIndex].glyphStart;

	return (cluster.textPosition < glyphClusters_.size())
		? glyphStart + glyphClusters_[cluster.textPosition]
		: glyphStart + runs_[cluster.runIndex].glyphCount;
}


float TextFlow::GetClusterRangeWidth(
	const ClusterPosition& clusterStart,
	const ClusterPosition& clusterEnd
) const throw()
{
	// Sums the glyph advances between two cluster positions,
	// useful for determining how long a line or word is.
	return GetClusterRangeWidth(
		GetClusterGlyphStart(clusterStart),
		GetClusterGlyphStart(clusterEnd),
		&glyphAdvances_.front()
	);
}


float TextFlow::GetClusterRangeWidth(
	UINT32 glyphStart,
	UINT32 glyphEnd,
	const float* glyphAdvances          // [glyphEnd]
) const throw()
{
	// Sums the glyph advances between two glyph offsets, given an explicit
	// advances array - useful for determining how long a line or word is.
	return std::accumulate(glyphAdvances + glyphStart, glyphAdvances + glyphEnd, 0.0f);
}

}
}
