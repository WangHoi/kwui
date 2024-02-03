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
        y += line_box->used_size.height;
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

int LineBox::addInlineBox(LayoutObject* o, InlineBox* box, bool is_atomic)
{
    int idx = (int)inline_frags.size();
    InlineFragment frag;
    frag.initFrom(o, box, is_atomic);
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
        inline_frags.erase(inline_frags.begin() + i, inline_frags.begin() + j + 1);
        inline_frags.insert(inline_frags.begin() + i, frag);
    } else {
        LOG(ERROR) << "LineBox::mergeInlineBox failed, BUG!";
    }
}
void LineBox::arrangeX(style::TextAlign text_align)
{
    used_size.width = 0;
    for (auto& frag : inline_frags) {
        used_size.width += frag.box->size.width;
    }

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
    used_size.height = pr.line_height;
    finalPlaceY(pos_y, pos_y + used_size.height, pos_y + strut.subtree_ascent, absl::MakeSpan(inline_frags));
}

LineBox::PlaceResult LineBox::placeY(InlineFragment& parent, absl::Span<InlineFragment> slice)
{
    PlaceResult pr;
    doPlaceY(pr, parent, slice);
    pr.line_height = std::max(pr.line_height, pr.max_va + pr.max_vd);
    float leading = pr.line_height - pr.max_va - pr.max_vd;
    parent.subtree_ascent = std::max(parent.subtree_ascent, pr.max_va + 0.5f * leading);
    parent.subtree_descent = std::max(parent.subtree_descent, pr.max_vd + 0.5f * leading);
    return pr;
}

void LineBox::doPlaceY(LineBox::PlaceResult& pr, const InlineFragment& parent, absl::Span<InlineFragment> slice)
{
    pr.max_va = std::max(parent.contentAscent(), parent.virtualAscent());
    pr.max_vd = std::max(parent.contentDescent(), parent.virtualDescent());
    for (auto& frag : slice) {
        bool skip_ascent_descent = false;
        auto& va = frag.vertical_align;
        if (va.type == VerticalAlignType::Top || va.type == VerticalAlignType::Bottom) {
            skip_ascent_descent = true;
            frag.baseline_offset = 0;
        } else if (va.type == VerticalAlignType::TextTop) {
            float p1 = parent.contentAscent();
            float y1 = frag.virtualAscent();
            frag.baseline_offset = p1 - y1;
        } else if (va.type == VerticalAlignType::TextBottom) {
            float p1 = -parent.contentDescent();
            float y1 = -frag.virtualDescent();
            frag.baseline_offset = p1 - y1;
        } else if (va.type == VerticalAlignType::Super) {
            float p1 = (parent.font_metrics.has_value()
                ? parent.font_metrics.value().x_height
                : parent.contentAscent() * 0.75);
            float y1 = (frag.font_metrics.has_value()
                ? frag.font_metrics.value().x_height * 0.5f
                : frag.contentAscent() * 0.5f);
            frag.baseline_offset = p1 - y1;
        } else if (va.type == VerticalAlignType::Sub) {
            float y1 = (frag.font_metrics.has_value()
                ? frag.font_metrics.value().x_height * 0.5f
                : frag.contentAscent() * 0.5f);
            frag.baseline_offset = -y1;
        } else if (va.type == VerticalAlignType::Middle) {
            float p1 = (parent.font_metrics.has_value()
                ? parent.font_metrics.value().x_height * 0.5f
                : parent.contentAscent() * 0.5f);
            float y1 = (frag.font_metrics.has_value()
                ? frag.font_metrics.value().x_height * 0.5f
                : frag.contentAscent() * 0.5f);
            frag.baseline_offset = p1 - y1;
        } else if (va.type == VerticalAlignType::Value) {
            if (frag.vertical_align.value.value().unit == ValueUnit::Percent) {
                frag.baseline_offset = frag.virtualHeight() * (frag.vertical_align.value.value().f32_val / 100.0f);
            } else {
                frag.baseline_offset = frag.vertical_align.value.value().pixelOrZero();
            }
        } else {
            frag.baseline_offset = 0;
        }

        if (!skip_ascent_descent) {
            //auto ca = frag.contentAscent();
            //auto va = frag.virtualAscent();
            //auto cd = frag.contentDescent();
            //auto vd = frag.virtualDescent();
            pr.max_va = std::max(pr.max_va,
                frag.baseline_offset + std::max(frag.contentAscent(), frag.virtualAscent()));
            pr.max_vd = std::max(pr.max_vd,
                -frag.baseline_offset + std::max(frag.contentDescent(), frag.virtualDescent()));
        }

        if (!frag.children.empty()) {
            PlaceResult cpr = placeY(frag, absl::MakeSpan(frag.children));
            if (!skip_ascent_descent) {
                pr.max_va = std::max(pr.max_va,
                    frag.baseline_offset + cpr.max_va);
                pr.max_vd = std::max(pr.max_vd,
                    -frag.baseline_offset + cpr.max_vd);
            } else {
                pr.line_height = std::max(pr.line_height, cpr.line_height);
            }
        } else {
            float va = frag.virtualAscent();
            float vd = frag.virtualDescent();
            float ca = frag.contentAscent();
            float cd = frag.contentDescent();
            frag.subtree_ascent = std::max(va, ca);
            frag.subtree_descent = std::max(vd, cd);
        }
    }
}

void LineBox::finalPlaceY(float line_top, float line_bottom, float parent_baseline, absl::Span<InlineFragment> slice)
{
    for (auto& frag : slice) {
        if (!frag.box) {
            LOG(ERROR) << "LineBox::finalPlaceY: no box pointer, bug!";
            continue;
        }

        frag.box->size.height = frag.virtualHeight();
        frag.box->baseline = frag.virtualAscent();

        auto& va = frag.vertical_align;
        if (va.type == VerticalAlignType::Top) {
            frag.box->pos.y = line_top + frag.subtree_ascent - frag.box->baseline;
        } else if (va.type == VerticalAlignType::Bottom) {
            frag.box->pos.y = line_bottom - frag.subtree_descent - frag.box->baseline;
        } else {
            frag.box->pos.y = parent_baseline - frag.baseline_offset - frag.box->baseline;
        }

        if (!frag.children.empty()) {
            float baseline = frag.box->pos.y + frag.box->baseline;
            finalPlaceY(line_top, line_bottom, baseline, absl::MakeSpan(frag.children));
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
    vertical_align = o->style->vertical_align;
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
