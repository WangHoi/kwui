#include "InlineLayout.h"
#include "BlockLayout.h"
#include "scene2d/Node.h"
#include <utility>

namespace style {

struct LineBox {
	scene2d::PointF offset; // used while building

	float left; // BFC coord: left
	float avail_width;
	// float line_gap;  // leading

	scene2d::DimensionF used_size;
	float used_baseline; // offset from used_size's top

	std::vector<InlineBox*> inline_boxes;

	LineBox(float left_, float avail_w)
		: left(left_)
		, avail_width(avail_w)
		, used_baseline(0)
	{
	}
	~LineBox() = default;
	int addInlineBox(InlineBox* box)
	{
		int idx = (int)inline_boxes.size();
		inline_boxes.push_back(box);
		return idx;
	}
	void layoutArrange(float offset_y)
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
};

InlineBoxBuilder::InlineBoxBuilder(InlineFormatContext& ifc, InlineBox* root)
    : ifc_(ifc), root_(root), contg_(root) {}
void InlineBoxBuilder::addText(scene2d::Node* node)
{
    InlineBox* inline_box = &node->inline_box_;
    inline_box->type = InlineBoxType::WithTextBoxes;
    //inline_box->payload = &node->text_boxes_;
	//ifc_.setupBox(box);
	beginInline(inline_box);
	endInline();
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
	if (contg_->type == InlineBoxType::WithTextBoxes)
		ifc_.addBox(contg_);

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
    return lb->avail_width - lb->offset.x;
}

void InlineFormatContext::setupBox(InlineBox* box)
{
    LineBox* lb = getLineBox(box->size.width);
    box->line_box = lb;
    box->line_box_offset_x = lb->offset.x;
    lb->offset.x += box->size.width;
}

void InlineFormatContext::layoutText(const std::string& text, InlineBox& inline_box, TextBoxes& text_boxes)
{
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
	if (type == InlineBoxType::WithTextBoxes) {
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

}
