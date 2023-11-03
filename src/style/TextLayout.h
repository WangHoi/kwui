#pragma once
#include "graph2d/TextLayout.h"
#include "style/StyleValue.h"
#include <vector>
#include <memory>
#include <string>

namespace style {

struct LineBox;
struct InlineBox;

struct GlyphRunBoxes
{
	std::vector<std::unique_ptr<graph2d::GlyphRunInterface>> glyph_runs;
	std::vector<std::unique_ptr<InlineBox>> inline_boxes;

	void reset()
	{
		glyph_runs.clear();
		inline_boxes.clear();
	}
};

class LineBoxInterface {
public:
	virtual LineBox* getCurrentLine() = 0;
	virtual LineBox* getNextLine() = 0;
};

class TextFlowSource : public graph2d::TextFlowSourceInterface
{
public:
	TextFlowSource(LineBoxInterface* lbi);
	// 通过 TextFlowSourceInterface 继承
	LineBox* getCurrentLine(float fontHeight, float& left, float& width, bool& empty) override;
	LineBox* getNextLine(float fontHeight, float& left, float& width) override;

private:
	LineBoxInterface *lbi_;
};

class GlyphRunSinkInterface {
public:
	virtual void prepare(size_t glyph_count) = 0;
	virtual void addGlyphRun(style::LineBox* line,
		const scene2d::PointF& pos,
		std::unique_ptr<graph2d::GlyphRunInterface> glyph_run) = 0;
};
class TextFlowSink : public graph2d::TextFlowSinkInterface {
public:
	TextFlowSink(GlyphRunSinkInterface* sink);
	void prepare(size_t glyph_count) override;
	void addGlyphRun(style::LineBox* line,
		const scene2d::PointF& pos,
		std::unique_ptr<graph2d::GlyphRunInterface> glyph_run) override;

private:
	GlyphRunSinkInterface* sink_;
};

}
