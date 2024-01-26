#include "InlineLayout.h"
#include "BlockLayout.h"
#include "TextLayout.h"
#include "scene2d/Node.h"
#include "graph2d/graph2d.h"
#include <utility>

namespace style {

InlineFormatContext::InlineFormatContext(LayoutObject* owner, BlockFormatContext& bfc, float avail_width)
    : owner_(owner), bfc_(bfc), avail_width_(avail_width), height_(0)
{
    // LOG(INFO) << "ifc ctor width " << avail_width;
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
    if (lb->inline_frags.empty() || lb->offset_x + pref_min_width <= lb->avail_width)
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

void InlineFormatContext::arrangeX(style::TextAlign text_align)
{
    for (auto& line_box : line_boxes_) {
        line_box->arrangeX(text_align);
    }
}

void InlineFormatContext::arrangeY()
{
    float y = 0.0f;
    for (auto& line_box : line_boxes_) {
        line_box->arrangeY(owner_, y);
        y += line_box->line_height;
    }
    height_ = y;
}

LineBox::LineBox(float left_, float avail_w)
    : offset_x(0.0f)
    , left(left_)
    , avail_width(avail_w)
    , used_baseline(0.0f)
{
}

int LineBox::addInlineBox(LayoutObject* o, InlineBox* box)
{
    int idx = (int)inline_frags.size();
    InlineFragment frag;
    frag.initFrom(o, box, true);
    inline_frags.push_back(frag);
    return idx;
}

void LineBox::mergeInlineBox(LayoutObject* o, InlineBox* box,
    InlineBox* first_child, InlineBox* last_child)
{
    InlineFragment frag;
    frag.initFrom(o, box, false);
    size_t i, j;
    for (i = 0; i < inline_frags.size(); ++i) {
        if (inline_frags[i].box == first_child) {
            for (j = i; j < inline_frags.size(); ++j) {
                if (inline_frags[j].box == last_child) {
                    break;
                }
            }
            break;
        }
    }
    if (i < inline_frags.size() && j < inline_frags.size()) {
        for (size_t k = i; k <= j; ++k) {
            frag.children.push_back(inline_frags[k]);
        }
        inline_frags.erase(inline_frags.begin() + i, inline_frags.begin() + j);
        inline_frags.insert(inline_frags.begin() + i, frag);
    } else {
        LOG(ERROR) << "LineBox::mergeInlineBox failed, BUG!";
    }
}
void LineBox::arrangeX(style::TextAlign text_align)
{
    used_size.width = 0;
    float line_ascent = 0, line_descent = 0;
    for (auto& frag : inline_frags) {
        used_size.width += frag.box->size.width;
        line_ascent = std::max(line_ascent, frag.box->baseline);
        line_descent = std::max(line_descent, frag.box->size.height - frag.box->baseline);
    }

    used_size.height = line_ascent + line_descent;
    used_baseline = line_descent;

    float line_leading = 0.5f * (line_height - (line_ascent + line_descent));
    float x = left;
    for (auto& frag : inline_frags) {
        frag.box->pos.x = x;
        x += frag.box->size.width;
    }

    float align_offset_x = 0.0f;
    if (text_align == style::TextAlign::Center) {
        align_offset_x = 0.5f * (avail_width - (x - left));
    } else if (text_align == style::TextAlign::Right) {
        align_offset_x = avail_width - (x - left);
    }
    if (align_offset_x > 0.0f) {
        for (auto& frag : inline_frags) {
            frag.box->pos.x += align_offset_x;
        }
    }
}

void LineBox::arrangeY(LayoutObject* owner, float pos_y)
{
    InlineFragment strut;
    strut.initFrom(owner, nullptr, false);

    PlaceResult pr = placeY(strut, absl::MakeSpan(inline_frags));
    line_height = strut.line_height = pr.max_va + pr.max_vd;
    finalPlaceY(pos_y + strut.virtualAscent(), absl::MakeSpan(inline_frags));
}

LineBox::PlaceResult LineBox::placeY(const InlineFragment& strut, absl::Span<InlineFragment> slice)
{
    float boff = strut.baseline_offset;
    float max_virtual_ascent = std::max(strut.contentAscent(), strut.virtualAscent());
    float max_virtual_descent = std::max(strut.contentDescent(), strut.virtualDescent());
    for (auto& frag : slice) {
        bool defer_update = false;
        auto vtype = frag.vertical_align.type;
        if (vtype == VerticalAlignType::Top || vtype == VerticalAlignType::TextTop
            || vtype == VerticalAlignType::Bottom || vtype == VerticalAlignType::TextBottom) {
            defer_update = true;
        } else if (vtype == VerticalAlignType::Middle) {

        } else if (vtype == VerticalAlignType::Value) {

        } else {
            frag.baseline_offset = boff;
        }
        if (frag.line_height == 32)
            int kk = 1;
        
        max_virtual_ascent = std::max(max_virtual_ascent,
            frag.baseline_offset + std::max(frag.contentAscent(), frag.virtualAscent()));
        max_virtual_descent = std::max(max_virtual_descent,
            frag.baseline_offset + std::max(frag.contentDescent(), frag.virtualDescent()));

        if (!frag.children.empty()) {
            PlaceResult cpr = placeY(frag, absl::MakeSpan(frag.children));
            max_virtual_ascent = std::max(max_virtual_ascent, cpr.max_va);
            max_virtual_descent = std::max(max_virtual_descent, cpr.max_vd);
        }
    }
    return { max_virtual_ascent, max_virtual_descent };
}

void LineBox::finalPlaceY(float baseline, absl::Span<InlineFragment> slice)
{
    for (auto& frag : slice) {
        if (!frag.box) {
            LOG(ERROR) << "LineBox::finalPlaceY: no box pointer, bug!";
            continue;
        }

        frag.box->size.height = frag.virtualHeight();
        frag.box->baseline = frag.virtualAscent();

        // TODO: handle vertical-align
        frag.box->pos.y = baseline - frag.box->baseline;

        if (!frag.children.empty()) {
            finalPlaceY(frag.box->pos.y + frag.box->baseline, absl::MakeSpan(frag.children));
        }
    }
}

void InlineFragment::initFrom(LayoutObject* o, InlineBox* ib, bool is_atomic)
{
    if (!is_atomic) {
        line_height = o->style->lineHeightInPixels();
        font_metrics = graph2d::getFontMetrics(
            o->style->fontFamily().c_str(),
            o->style->fontSizeInPixels());
    }
    layout_object = o;
    box = ib;
}

float InlineFragment::contentAscent() const
{
    if (font_metrics.has_value()) {
        auto& fm = font_metrics.value();
        return fm.ascent + 0.5f * fm.line_gap;
    } else {
        return box ? box->baseline : 0.0f;
    }
}

float InlineFragment::contentDescent() const
{
    if (font_metrics.has_value()) {
        auto& fm = font_metrics.value();
        return fm.descent + 0.5f * fm.line_gap;
    } else {
        return box ? (box->size.height - box->baseline) : 0.0f;
    }
}

float InlineFragment::contentHeight() const
{
    if (font_metrics.has_value()) {
        return font_metrics.value().lineHeight();
    } else {
        return box ? box->size.height : 0.0f;
    }
}

float InlineFragment::virtualAscent() const
{
    return font_metrics.has_value()
        ? (0.5f * (line_height - contentHeight()) + contentAscent())
        : contentAscent();
}

float InlineFragment::virtualDescent() const
{
    return font_metrics.has_value()
        ? (0.5f * (line_height - contentHeight()) + contentDescent())
        : contentDescent();
}

float InlineFragment::virtualHeight() const
{
    return font_metrics.has_value()
        ? line_height
        : contentHeight();
}

}
