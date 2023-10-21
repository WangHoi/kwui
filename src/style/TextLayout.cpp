#include "TextLayout.h"
#include "InlineLayout.h"

namespace style {

TextFlowSource::TextFlowSource(InlineBoxBuilder& ibb)
    : ibb_(ibb)
{}

void TextFlowSource::getCurrentLine(float fontHeight, float& left, float& width, bool& empty)
{
    LineBox* lb = ibb_.getCurrentLine();
    left = lb->left + lb->offset_x;
    width = lb->avail_width - lb->offset_x;
    empty = !(lb->offset_x > 0.0f);
}

void TextFlowSource::getNextLine(float fontHeight, float& left, float& width)
{
    LineBox* lb = ibb_.getNextLine();
    left = lb->left + lb->offset_x;
    width = lb->avail_width - lb->offset_x;
}

TextFlowSink::TextFlowSink(InlineBoxBuilder& ibb)
    : ibb_(ibb)
{}

void TextFlowSink::prepare(size_t glyph_count)
{

}

void TextFlowSink::setGlyphRun(
    const scene2d::PointF& pos,
    std::unique_ptr<graph2d::GlyphRunInterface> glyph_run)
{
    ibb_.addGlyphRun(pos, std::move(glyph_run));
}

}
