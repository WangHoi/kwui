#pragma once
#include "TextFlow.h"
#include "StyleValue.h"
#include "StyleFont.h"
#include "scene2d/geom_types.h"
#include <vector>
#include <memory>
#include <string>

namespace style {

struct LineBox;
struct InlineBox;

class GlyphRunInterface {
public:
	virtual ~GlyphRunInterface() = default;
	virtual scene2d::RectF boundingRect() = 0;
};

class TextFlowSourceInterface {
public:
	virtual ~TextFlowSourceInterface() = default;
	virtual LineBox* getCurrentLine(float fontHeight, float& left, float& width, bool& empty) = 0;
	virtual LineBox* getNextLine(float fontHeight, float& left, float& width) = 0;
};

class TextFlowSinkInterface {
public:
	virtual ~TextFlowSinkInterface() = default;
	virtual void prepare(size_t glyph_count) = 0;
	virtual void addGlyphRun(
		LineBox* line,
		const scene2d::PointF& pos,
		std::unique_ptr<GlyphRunInterface> glyph_run) = 0;
};

class TextFlowInterface {
public:
	virtual ~TextFlowInterface() = default;
	virtual FontMetrics fontMetrics() = 0;
	virtual std::tuple<float, float> measureWidth() = 0;
	virtual void flowText(TextFlowSourceInterface* source, TextFlowSinkInterface* sink) = 0;
};

struct GlyphRunBoxes
{
	std::vector<std::unique_ptr<style::GlyphRunInterface>> glyph_runs;
	std::vector<std::unique_ptr<InlineBox>> inline_boxes;

	void reset();
};

class LineBoxInterface {
public:
	virtual ~LineBoxInterface() = default;
	virtual LineBox* getCurrentLine() = 0;
	virtual LineBox* getNextLine() = 0;
};

class TextFlowSource : public style::TextFlowSourceInterface
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
	virtual ~GlyphRunSinkInterface() = default;
	virtual void prepare(size_t glyph_count) = 0;
	virtual void addGlyphRun(style::LineBox* line,
		const scene2d::PointF& pos,
		std::unique_ptr<style::GlyphRunInterface> glyph_run) = 0;
};
class TextFlowSink : public style::TextFlowSinkInterface {
public:
	TextFlowSink(GlyphRunSinkInterface* sink);
	void prepare(size_t glyph_count) override;
	void addGlyphRun(style::LineBox* line,
		const scene2d::PointF& pos,
		std::unique_ptr<style::GlyphRunInterface> glyph_run) override;

private:
	GlyphRunSinkInterface* sink_;
};

}
