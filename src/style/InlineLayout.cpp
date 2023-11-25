#include "InlineLayout.h"
#include "BlockLayout.h"
#include "TextLayout.h"
#include "scene2d/Node.h"
#include "graph2d/graph2d.h"
#include <utility>

namespace style {

InlineFormatContext::InlineFormatContext(BlockFormatContext& bfc, float left, float avail_width, float top)
    : bfc_(bfc), left_(left), avail_width_(avail_width), top_(top), height_(0)
{
    LOG(INFO) << "ifc ctor " << left_ << ", " << top_ << " width " << avail_width;
}

InlineFormatContext::~InlineFormatContext() = default;

float InlineFormatContext::getAvailWidth() const
{
    if (line_boxes_.empty())
        return avail_width_;
    LineBox* lb = line_boxes_.back().get();
    return lb->avail_width - lb->offset_x;
}

void InlineFormatContext::setupBox(InlineBox* box)
{
    LineBox* lb = getLineBox(box->size.width);
    box->line_box = lb;
    box->line_box_offset_x = lb->offset_x;
    lb->offset_x += box->size.width;
}

void InlineFormatContext::addBox(InlineBox* box)
{
    box->line_box->addInlineBox(box);
}

float InlineFormatContext::getLayoutWidth() const
{
    float w = 0.0f;
    for (const auto& lb : line_boxes_) {
        w = std::max(w, lb->used_size.width);
    }
    return w;
}

LineBox* InlineFormatContext::newLineBox()
{
    auto lb = std::make_unique<LineBox>(left_, avail_width_);
    auto ret = lb.get();
    line_boxes_.emplace_back(std::move(lb));
    return ret;
}

LineBox* InlineFormatContext::getLineBox(float pref_min_width)
{
    if (line_boxes_.empty())
        return newLineBox();
    LineBox* lb = line_boxes_.back().get();
    if (lb->inline_boxes.empty() || lb->offset_x + pref_min_width <= lb->avail_width)
        return lb;
    return newLineBox();
}

LineBox* InlineFormatContext::getCurrentLine()
{
    return getLineBox(0.0f);
}

LineBox* InlineFormatContext::getNextLine()
{
    return newLineBox();
}

void InlineFormatContext::arrange(style::TextAlign text_align)
{
    float y = top_;
    for (auto& line_box : line_boxes_) {
        line_box->arrange(y, text_align);
        y += line_box->line_height;
    }
    height_ = y - top_;
}

LineBox::LineBox(float left_, float avail_w)
    : offset_x(0.0f)
    , left(left_)
    , avail_width(avail_w)
    , used_baseline(0.0f)
{
}

int LineBox::addInlineBox(InlineBox* box)
{
    int idx = (int)inline_boxes.size();
    inline_boxes.push_back(box);
    return idx;
}

void LineBox::arrange(float pos_y, style::TextAlign text_align)
{
    used_size.width = 0;
    float line_ascent = 0, line_descent = 0;
    for (InlineBox* b : inline_boxes) {
        used_size.width += b->size.width;
        line_ascent = std::max(line_ascent, b->baseline);
        line_descent = std::max(line_descent, b->size.height - b->baseline);
    }

    used_size.height = line_ascent + line_descent;
    used_baseline = line_descent;

    float line_leading = 0.5f * (line_height - (line_ascent + line_descent));
    float x = left;
    for (style::InlineBox* b : inline_boxes) {
        b->pos.x = x;
        b->pos.y = pos_y + line_leading + line_ascent - b->baseline;
        x += b->size.width;
    }

    float align_offset_x = 0.0f;
    if (text_align == style::TextAlign::Center) {
        align_offset_x = 0.5f * (avail_width - (x - left));
    } else if (text_align == style::TextAlign::Right) {
        align_offset_x = avail_width - (x - left);
    }
    if (align_offset_x > 0.0f) {
        for (style::InlineBox* b : inline_boxes) {
            b->pos.x += align_offset_x;
        }
    }
}

}
