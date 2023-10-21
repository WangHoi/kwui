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
};

class InlineBoxBuilder;

class TextFlowSource : public graph2d::TextFlowSourceInterface
{
public:
	TextFlowSource(InlineBoxBuilder& ibb);
	// 通过 TextFlowSourceInterface 继承
	void getNextLine(float fontHeight, float& left, float& width, bool& allow_overflow) override;

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
