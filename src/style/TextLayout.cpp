#include "TextLayout.h"
#include "InlineLayout.h"

namespace style {

TextFlowSource::TextFlowSource(LineBoxInterface* lbi)
    : lbi_(lbi)
{}

LineBox* TextFlowSource::getCurrentLine(float fontHeight, float& left, float& width, bool& empty)
{
    LineBox* lb = lbi_->getCurrentLine();
    left = lb->left + lb->offset_x;
    width = lb->avail_width - lb->offset_x;
    empty = !(lb->offset_x > 0.0f);
    return lb;
}

LineBox* TextFlowSource::getNextLine(float fontHeight, float& left, float& width)
{
    LineBox* lb = lbi_->getNextLine();
    left = lb->left + lb->offset_x;
    width = lb->avail_width - lb->offset_x;
    return lb;
}

TextFlowSink::TextFlowSink(GlyphRunSinkInterface* sink)
    : sink_(sink)
{}

void TextFlowSink::prepare(size_t glyph_count)
{

}

void TextFlowSink::addGlyphRun(LineBox* line,
    const scene2d::PointF& pos,
    std::unique_ptr<graph2d::GlyphRunInterface> glyph_run)
{
    sink_->addGlyphRun(line, pos, std::move(glyph_run));
}

}
