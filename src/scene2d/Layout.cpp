#include "Layout.h"
#include "Node.h"
#include <utility>

namespace scene2d {

void try_convert_to_px(style::Value& v, float percent_base)
{
    v = style::Value::fromPixel(try_resolve_to_px(v, percent_base).value_or(0));
}

absl::optional<float> try_resolve_to_px(const style::Value& v, float percent_base)
{
    if (v.unit == style::ValueUnit::Percent) {
        return v.f32_val / 100.0f * percent_base;
    } else if (v.unit == style::ValueUnit::Pixel) {
        return v.f32_val;
    } else if (v.unit == style::ValueUnit::Raw) {
        return v.f32_val;
    }
    return 0.0f;
}

absl::optional<float> try_resolve_to_px(const style::Value& v, absl::optional<float> percent_base)
{
    if (v.unit == style::ValueUnit::Percent) {
        return percent_base.has_value()
            ? absl::optional<float>(v.f32_val / 100.0f * percent_base.value())
            : absl::nullopt;
    } else if (v.unit == style::ValueUnit::Pixel) {
        return v.f32_val;
    } else if (v.unit == style::ValueUnit::Raw) {
        return v.f32_val;
    }
    return absl::nullopt;
}

float collapse_margin(float m1, float m2)
{
    if (m1 >= 0 && m2 >= 0)
        return std::max(m1, m2);
    else if (m1 < 0 && m2 < 0)
        return std::min(m1, m2);
    else
        return m1 + m2;
}

struct LineBox {
    PointF offset;
    
    float left;
    float avail_width;
    // float line_gap;  // leading
    
    DimensionF used_size;
    float used_baseline; // offset from used_size's bottom

    std::vector<InlineBox*> inline_boxes;

    LineBox(float left_, float avail_w)
        : left(left_)
        , avail_width(avail_w)
        , used_baseline(0)
    {
    }
    int addInlineBox(InlineBox *box)
    {
        int idx = (int)inline_boxes.size();
        inline_boxes.push_back(box);
        return idx;
    }
    /*
    std::tuple<float, float> getAvailWidth() const
    {
        float x = left;
        float w = avail_width;
        for (auto& b : inline_boxes) {
            x += b->size.width;
            w -= b->size.width;
        }
        return std::make_tuple(x, w);
    }
    float remainWidth() const
    {
        float w = avail_width;
        for (auto& b : inline_boxes) {
            w -= b->size.width;
        }
        return w;
    }
    */
    void layout(float base_y)
    {
        used_size.width = 0;
        float top = 0, bottom = 0;
        for (auto& b : inline_boxes) {
            used_size.width += b->size.width;
            top = std::max(top, b->size.width - b->baseline);
            bottom = std::max(bottom, b->baseline);
        }
        used_size.height = top + bottom;
        used_baseline = bottom;

        float x = left;
        for (auto& b : inline_boxes) {
            b->offset.x = x;
            b->offset.y = base_y + (top - (b->size.width - b->baseline));
            x += b->size.width;
        }
    }
};

InlineFormatContext::InlineFormatContext(float avail_width)
    : avail_width_(avail_width), height_(0)
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
    auto lb = std::make_unique<LineBox>(0.0f, avail_width_);
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
        line_box->layout(y);
        y += line_box->used_size.height;
    }
    height_ = y;
}

BlockBoxBuilder::BlockBoxBuilder(BlockBox* root)
    : root_(root), contg_(root) {}

float BlockBoxBuilder::containingBlockWidth() const
{
    return contg_->avail_width;
}

void BlockBoxBuilder::addText(Node* node)
{
    contg_->type = BlockBoxType::WithInlineChildren;
    contg_->payload = node->parent();
}

void BlockBoxBuilder::beginInline(Node* node)
{
}

void BlockBoxBuilder::endInline()
{
}

void BlockBoxBuilder::beginBlock(BlockBox* box)
{
    if (last_child_) {
        box->next_sibling = last_child_->next_sibling;
        box->prev_sibling = last_child_;
        last_child_->next_sibling = box;
        box->next_sibling->prev_sibling = box;
    } else {
        box->next_sibling = box->prev_sibling = box;
    }
    last_child_ = box;
    stack_.push_back(std::make_tuple(contg_, last_child_));
    
    contg_->type = BlockBoxType::WithBlockChildren;
    contg_->payload = box;
    contg_ = box;
    last_child_ = nullptr;
}

void BlockBoxBuilder::endBlock()
{
    std::tie(contg_, last_child_) = stack_.back();
    stack_.pop_back();
}

}
