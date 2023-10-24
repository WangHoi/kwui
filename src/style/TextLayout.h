#pragma once
#include "InlineLayout.h"
#include "graph2d/TextLayout.h"
#include "style/StyleValue.h"
#include <vector>
#include <memory>
#include <string>

namespace style {

struct TextBoxes
{
	std::vector<std::unique_ptr<graph2d::GlyphRunInterface>> glyph_runs;
	std::vector<std::unique_ptr<InlineBox>> inline_boxes;

	void reset()
	{
		glyph_runs.clear();
		inline_boxes.clear();
	}
};

class InlineBoxBuilder;

class TextFlowSource : public graph2d::TextFlowSourceInterface
{
public:
	TextFlowSource(InlineBoxBuilder& ibb);
	// 通过 TextFlowSourceInterface 继承
	void getCurrentLine(float fontHeight, float& left, float& width, bool& empty) override;
	void getNextLine(float fontHeight, float& left, float& width) override;

private:
	InlineBoxBuilder& ibb_;
};

class TextFlowSink : public graph2d::TextFlowSinkInterface {
public:
	TextFlowSink(InlineBoxBuilder& ibb);
	void prepare(size_t glyph_count) override;
	void setGlyphRun(
		const scene2d::PointF& pos,
		std::unique_ptr<graph2d::GlyphRunInterface> glyph_run) override;

private:
	InlineBoxBuilder& ibb_;
};

}
