#include "InlineLayout.h"
#include "BlockLayout.h"
#include "scene2d/Node.h"
#include <utility>

namespace style {

LineBox::LineBox(float left_, float avail_w)
    : left(left_)
    , avail_width(avail_w)
    , used_baseline(0)
{
}

LineBox::~LineBox() = default;

InlineFormatContext::InlineFormatContext(BlockFormatContext& bfc, float left, float avail_width)
    : bfc_(bfc), left_(left), avail_width_(avail_width), height_(0)
{}

InlineFormatContext::~InlineFormatContext() = default;

float InlineFormatContext::getAvailWidth() const
{
    if (line_boxes_.empty())
        return avail_width_;
    LineBox* lb = line_boxes_.back().get();
    return lb->avail_width - lb->offset.x;
}

void InlineFormatContext::setupBox(InlineBox* box)
{
    LineBox* lb = getLineBox(box->size.width);
    box->line_box = lb;
    box->line_box_offset_x = lb->offset.x;
    lb->offset.x += box->size.width;
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
    if (lb->inline_boxes.empty() || lb->offset.x + pref_min_width <= lb->avail_width)
        return lb;
    return newLineBox();
}

void InlineFormatContext::layout()
{
    float y = 0;
    for (auto& line_box : line_boxes_) {
        line_box->layout(bfc_.margin_bottom + y);
        y += line_box->used_size.height;
    }
    height_ = y;
}

}
