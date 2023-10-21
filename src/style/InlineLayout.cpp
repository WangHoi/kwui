#include "InlineLayout.h"
#include "BlockLayout.h"
#include "TextLayout.h"
#include "scene2d/Node.h"
#include "graph2d/graph2d.h"
#include <utility>

namespace style {

InlineBoxBuilder::InlineBoxBuilder(InlineFormatContext& ifc, InlineBox* root)
    : ifc_(ifc), root_(root), contg_(root) {}
void InlineBoxBuilder::addText(scene2d::Node* node)
{
    text_node_ = node;
    beginInline(&text_node_->inline_box_);
	TextFlowSource source(*this);
	TextFlowSink sink(*this);
	node->text_flow_->flowText(&source, &sink);
    endInline();
    text_node_ = nullptr;
}

void InlineBoxBuilder::addGlyphRun(const scene2d::PointF& pos, std::unique_ptr<graph2d::GlyphRunInterface> glyph_run)
{
    std::unique_ptr<InlineBox> inline_box = std::make_unique<InlineBox>();
    inline_box->type = InlineBoxType::WithGlyphRun;
    inline_box->payload = glyph_run.get();
    auto fm = text_node_->text_flow_->flowMetrics();
    inline_box->baseline = fm.baseline;
    inline_box->size = glyph_run->boundingRect().size();

    appendGlyphRun(inline_box.get());
    
    text_node_->text_boxes_.inline_boxes.push_back(std::move(inline_box));
    text_node_->text_boxes_.glyph_runs.push_back(std::move(glyph_run));
}

LineBox* InlineBoxBuilder::getNextLine(float pref_min_width)
{
    line_ = ifc_.getLineBox(pref_min_width);
    return line_;
}

void InlineBoxBuilder::appendGlyphRun(InlineBox* box)
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
    if (contg_->type == InlineBoxType::Empty) {
        contg_->type = InlineBoxType::WithInlineChildren;
        contg_->payload = box;
    }

    box->line_box = line_;
    box->line_box->addInlineBox(box);
    box->line_box_offset_x = box->line_box->offset_x;
    box->line_box->offset_x += box->size.width;
}

void InlineBoxBuilder::beginInline(InlineBox* box)
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
    if (contg_->type == InlineBoxType::Empty) {
        contg_->type = InlineBoxType::WithInlineChildren;
        contg_->payload = box;
    }
    stack_.push_back(std::make_tuple(contg_, last_child_));

    contg_ = box;
    last_child_ = nullptr;
}

void InlineBoxBuilder::endInline()
{
	std::tie(contg_, last_child_) = stack_.back();
    stack_.pop_back();
}

InlineFormatContext::InlineFormatContext(BlockFormatContext& bfc, float left, float avail_width)
    : bfc_(bfc), left_(left), avail_width_(avail_width), height_(0)
{}

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

void InlineFormatContext::layoutArrange()
{
    float y = 0;
    for (auto& line_box : line_boxes_) {
        line_box->layoutArrange(bfc_.margin_bottom + y);
        y += line_box->used_size.height;
    }
    height_ = y;
}

scene2d::RectF InlineBox::boundingRect() const
{
	if (type == InlineBoxType::WithGlyphRun) {
		return scene2d::RectF::fromXYWH(pos.x, pos.y, size.width, size.height);
	} else if (type == InlineBoxType::WithInlineChildren) {
		InlineBox* first_child = absl::get<InlineBox*>(payload);
		if (first_child->prev_sibling != first_child) {
			InlineBox* last_child = first_child->prev_sibling;

			scene2d::RectF first_brect = first_child->boundingRect();
			scene2d::RectF last_brect = last_child->boundingRect();
			return scene2d::RectF::fromLTRB(
				std::min(first_brect.left, last_brect.left),
				std::min(first_brect.top, last_brect.top),
				std::max(first_brect.right, last_brect.right),
				std::max(first_brect.bottom, last_brect.bottom));
		} else {
			return first_child->boundingRect();
		}
	} else {
		return scene2d::RectF();
	}
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

void LineBox::layoutArrange(float offset_y)
{
    used_size.width = 0;
    float top = 0, bottom = 0;
    for (InlineBox* b : inline_boxes) {
        used_size.width += b->size.width;
        top = std::max(top, b->baseline);
        bottom = std::max(bottom, b->size.height - b->baseline);
    }
    used_size.height = top + bottom;
    used_baseline = bottom;

    float x = left;
    for (style::InlineBox* b : inline_boxes) {
        b->pos.x = x;
        b->pos.y = offset_y + top - b->baseline;
        x += b->size.width;
    }
}

}
