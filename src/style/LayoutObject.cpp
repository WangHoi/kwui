#include "LayoutObject.h"
#include "BoxConstraintSolver.h"
#include "scene2d/Node.h"
#include "graph2d/Painter.h"
#include "absl/functional/bind_front.h"
#include "base/log.h"
#include <algorithm>

namespace style {

static const float SCROLLBAR_GUTTER_WIDTH = 12.0f;

void LayoutObject::init(const Style* st)
{
	style = st;
	next_sibling = prev_sibling = this;
}

void LayoutObject::reset()
{
	flags = 0;
	min_width = 0.0f;
	max_width = std::numeric_limits<float>::infinity();
	prefer_height = absl::nullopt;
	parent = nullptr;
	first_child = nullptr;
	next_sibling = prev_sibling = this;
}

void LayoutObject::reflow(FlowRoot fl, const scene2d::DimensionF& viewport_size)
{
	CHECK(fl.root->flags & LayoutObject::NEW_BFC_FLAG)
		<< "LayoutObject::reflow: root expect NEW_BFC_FLAG";
	CHECK(absl::holds_alternative<BlockBox>(fl.root->box))
		<< "LayoutObject::reflow: root must be block-level";

	const Style& st = *fl.root->style;
	BlockBox& b = absl::get<BlockBox>(fl.root->box);

	// Measure min(max)-content width
	measure(fl.root, viewport_size.height);

	// Find containing bfc
	BlockFormatContext bfc(nullptr);
	if (fl.positioned_parent) {
		CHECK(absl::holds_alternative<BlockBox>(fl.positioned_parent->box));
		const auto& rect = absl::get<BlockBox>(fl.positioned_parent->box).paddingRect();
		bfc.contg_left_edge = rect.left;
		bfc.contg_right_edge = rect.right;
		bfc.border_right_edge = rect.left;
		bfc.border_bottom_edge = bfc.margin_bottom_edge = rect.top;
		bfc.contg_height.emplace(rect.height());
	} else {
		bfc.contg_left_edge = 0.0f;
		bfc.contg_right_edge = viewport_size.width;
		bfc.contg_height.emplace(viewport_size.height);
	}

	arrange(fl.root, bfc, viewport_size);
}

void LayoutObject::paint(FlowRoot fl, graph2d::PainterInterface* painter)
{
	paint(fl.root, painter);
}

void LayoutObject::measure(LayoutObject* o, float viewport_height)
{
	const style::Style& st = *o->style;

	if (o->flags & HAS_BLOCK_CHILD_FLAG) {
		float min_width = 0.0f, max_width = std::numeric_limits<float>::infinity();
		LayoutObject* child = o->first_child;
		do {
			const style::Style& st = *child->style;
			float margin_left = try_resolve_to_px(st.margin_left, absl::nullopt).value_or(0);
			float border_left = try_resolve_to_px(st.border_left_width, absl::nullopt).value_or(0);
			float padding_left = try_resolve_to_px(st.padding_left, absl::nullopt).value_or(0);
			float width = try_resolve_to_px(st.width, absl::nullopt).value_or(0);
			float padding_right = try_resolve_to_px(st.padding_right, absl::nullopt).value_or(0);
			float border_right = try_resolve_to_px(st.border_right_width, absl::nullopt).value_or(0);
			float margin_right = try_resolve_to_px(st.margin_right, absl::nullopt).value_or(0);
			float mbp_width = margin_left + border_left + padding_left + padding_right + border_right + margin_right;

			if (st.width.isPixel() || st.width.isRaw()) {
				min_width = std::max(min_width, mbp_width + width);
				if (max_width == std::numeric_limits<float>::infinity()) {
					max_width = mbp_width + width;
				} else {
					max_width = std::max(max_width, mbp_width + width);
				}
			} else {
				measure(child, viewport_height);
				min_width = std::max(min_width, mbp_width + child->min_width);
				if (max_width == std::numeric_limits<float>::infinity()) {
					max_width = mbp_width + child->max_width;
				} else {
					max_width = std::max(max_width, mbp_width + child->max_width);
				}
			}

			child = child->next_sibling;
		} while (child != o->first_child);
		o->min_width = min_width;
		o->max_width = max_width;
	} else if (o->flags & HAS_INLINE_CHILD_FLAG) {
		float min_width = 0.0f, max_width = std::numeric_limits<float>::infinity();
		LayoutObject* child = o->first_child;
		do {
			measure(child, viewport_height);

			min_width = std::max(min_width, child->min_width);
			if (max_width == std::numeric_limits<float>::infinity()) {
				max_width = child->max_width;
			} else {
				max_width = std::max(max_width, child->max_width);
			}

			child = child->next_sibling;
		} while (child != o->first_child);
		o->min_width = min_width;
		o->max_width = max_width;
	} else if (absl::holds_alternative<TextBox>(o->box)) {
		TextBox& tb = absl::get<TextBox>(o->box);
		// TODO: handle parent margin_border_padding_left(right)
		std::tie(o->min_width, o->max_width) = tb.text_flow->measureWidth();
	}
}

void LayoutObject::arrange(LayoutObject* o, BlockFormatContext& bfc, const scene2d::DimensionF& viewport_size)
{
	const Style& st = *o->style;
	ScrollbarPolicy scroll_y = ScrollbarPolicy::Hidden;
	if (st.overflow_y == OverflowType::Scroll)
		scroll_y = ScrollbarPolicy::Stable;
	arrange(o, bfc, viewport_size, scroll_y);

	if (st.overflow_y == OverflowType::Auto && false) {
		// handle overflow-y
		scroll_y = ScrollbarPolicy::Stable;
		arrange(o, bfc, viewport_size, scroll_y);
	}
}

void LayoutObject::arrange(LayoutObject* o,
	BlockFormatContext& bfc,
	const scene2d::DimensionF& viewport_size,
	ScrollbarPolicy scroll_y)
{
	const Style& st = *o->style;
	if (st.display == DisplayType::Block) {
		arrangeBlockX(o, bfc, viewport_size, scroll_y);
		arrangeBlockTop(o, bfc);
		arrangeBlockChildren(o, bfc, viewport_size, scroll_y);
		arrangeBlockBottom(o, bfc);
	} else {
		LOG(WARNING) << "arrange " << st.display << " not implemented.";
	}
}

void LayoutObject::arrangeBlockX(LayoutObject* o,
	BlockFormatContext& bfc,
	const scene2d::DimensionF& viewport_size,
	ScrollbarPolicy scroll_y)
{
	const Style& st = *o->style;
	CHECK(st.display == DisplayType::Block);

	float contg_width = bfc.contg_right_edge - bfc.contg_left_edge;
	float clean_contg_width = contg_width
		- try_resolve_to_px(st.border_left_width, contg_width).value_or(0)
		- try_resolve_to_px(st.padding_left, contg_width).value_or(0)
		- try_resolve_to_px(st.padding_right, contg_width).value_or(0)
		- try_resolve_to_px(st.border_right_width, contg_width).value_or(0);
	if (scroll_y == ScrollbarPolicy::Stable) {
		clean_contg_width = std::max(0.0f, clean_contg_width - SCROLLBAR_GUTTER_WIDTH);
	} else if (scroll_y == ScrollbarPolicy::StableBothEdges) {
		clean_contg_width = std::max(0.0f, clean_contg_width - 2.0f * SCROLLBAR_GUTTER_WIDTH);
	} else {
		clean_contg_width = std::max(0.0f, clean_contg_width);
	}
	StaticBlockWidthSolver solver(
		clean_contg_width,
		try_resolve_to_px(st.margin_left, contg_width),
		try_resolve_to_px(st.width, contg_width),
		try_resolve_to_px(st.margin_right, contg_width));
	float avail_width = solver.measureWidth().value;
	solver.setLayoutWidth(avail_width);

	// Compute width, left and right margins
	BlockBox& b = absl::get<BlockBox>(o->box);
	b.content.width = solver.width();
	b.margin.left = solver.marginLeft();
	b.border.left = try_resolve_to_px(st.border_left_width, contg_width).value_or(0);
	b.padding.left = try_resolve_to_px(st.padding_left, contg_width).value_or(0);
	b.padding.right = try_resolve_to_px(st.padding_right, contg_width).value_or(0);
	b.border.right = try_resolve_to_px(st.border_right_width, contg_width).value_or(0);
	b.margin.right = solver.marginRight();

	// Compute height, top and bottom margins
	b.margin.top = try_resolve_to_px(st.margin_top, contg_width).value_or(0);
	b.border.top = try_resolve_to_px(st.border_top_width, contg_width).value_or(0);
	b.padding.top = try_resolve_to_px(st.padding_top, contg_width).value_or(0);

	b.padding.bottom = try_resolve_to_px(st.padding_bottom, contg_width).value_or(0);
	b.border.bottom = try_resolve_to_px(st.border_bottom_width, contg_width).value_or(0);
	b.margin.bottom = try_resolve_to_px(st.margin_bottom, contg_width).value_or(0);

	// Compute pos.x
	b.pos.x = bfc.contg_left_edge + b.margin.left;
}

void LayoutObject::arrangeBlockTop(LayoutObject* o, BlockFormatContext& bfc)
{
	CHECK(absl::holds_alternative<BlockBox>(o->box));

	BlockBox& box = absl::get<BlockBox>(o->box);
	float borpad_top = box.border.top + box.padding.top;

	if (o->flags & NEW_BFC_FLAG) {
		bfc.border_bottom_edge = bfc.margin_bottom_edge = bfc.margin_bottom_edge + borpad_top;
	} else {
		if (borpad_top > 0) {
			float coll_margin = collapse_margin(box.margin.top,
				(bfc.margin_bottom_edge - bfc.border_bottom_edge));
			box.pos.y = bfc.border_bottom_edge + coll_margin - box.margin.top;
			bfc.border_bottom_edge = bfc.border_bottom_edge + coll_margin + borpad_top;
			bfc.margin_bottom_edge = bfc.border_bottom_edge;
		} else {
			float coll_margin = collapse_margin(box.margin.top,
				(bfc.margin_bottom_edge - bfc.border_bottom_edge));
			box.pos.y = bfc.border_bottom_edge + coll_margin - box.margin.top;
			//bfc.border_bottom_edge += coll_margin + borpad_top;
			bfc.margin_bottom_edge = bfc.border_bottom_edge + coll_margin;
		}
	}
}

void LayoutObject::arrangeBlockChildren(LayoutObject* o,
	BlockFormatContext& bfc,
	const scene2d::DimensionF& viewport_size,
	ScrollbarPolicy scroll_y)
{
	CHECK(absl::holds_alternative<BlockBox>(o->box));

	BlockBox& box = absl::get<BlockBox>(o->box);
	float borpad_top = box.border.top + box.padding.top;
	float borpad_bottom = box.border.bottom + box.padding.bottom;

	if (o->flags & NEW_BFC_FLAG) {
		BlockFormatContext& inner_bfc = o->bfc.value();
		if (scroll_y == ScrollbarPolicy::Stable) {
			inner_bfc.contg_right_edge = std::max(bfc.contg_left_edge,
				bfc.contg_right_edge - SCROLLBAR_GUTTER_WIDTH);
		} else if (scroll_y == ScrollbarPolicy::StableBothEdges) {
			if (bfc.contg_right_edge - bfc.contg_left_edge >= 2.0f * SCROLLBAR_GUTTER_WIDTH) {
				inner_bfc.contg_left_edge = bfc.contg_left_edge + SCROLLBAR_GUTTER_WIDTH;
				inner_bfc.contg_right_edge = bfc.contg_right_edge - SCROLLBAR_GUTTER_WIDTH;
			} else {
				float mid = 0.5f * (bfc.contg_left_edge + bfc.contg_right_edge);
				inner_bfc.contg_left_edge = inner_bfc.contg_right_edge = mid;
			}
		} else {
			inner_bfc.contg_left_edge = bfc.contg_left_edge;
			inner_bfc.contg_right_edge = bfc.contg_right_edge;
		}
		inner_bfc.border_right_edge = inner_bfc.contg_left_edge;
		inner_bfc.border_bottom_edge = inner_bfc.margin_bottom_edge = bfc.margin_bottom_edge;
		inner_bfc.contg_height = bfc.contg_height;
		bfc = inner_bfc;
	}

	box.prefer_height = try_resolve_to_px(o->style->height, bfc.contg_height);

	float saved_bfc_margin_bottom = bfc.margin_bottom_edge;
	if (o->flags & HAS_BLOCK_CHILD_FLAG) {
		{
			base::scoped_setter _1(bfc.contg_height,
				box.prefer_height);
			base::scoped_setter _2(bfc.contg_left_edge, bfc.contg_left_edge + box.contentRect().left);
			base::scoped_setter _3(bfc.contg_right_edge, bfc.contg_left_edge + box.contentRect().width());
			LayoutObject* child = o->first_child;
			do {
				arrange(child, bfc, viewport_size, scroll_y);
				child = child->next_sibling;
			} while (child != o->first_child);
		}

		BlockBox& box = absl::get<BlockBox>(o->box);
		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom_edge = saved_bfc_margin_bottom + *box.prefer_height;
			bfc.margin_bottom_edge = bfc.border_bottom_edge;
		} else {
			// Compute 'auto' height
			if (borpad_bottom > 0) {
				box.content.height = std::max(0.0f,
					bfc.margin_bottom_edge - box.pos.y - box.margin.top - borpad_top);
			} else {
				box.content.height = std::max(0.0f,
					bfc.border_bottom_edge - box.pos.y - box.margin.top - borpad_top);
			}
		}
	} else if (o->flags & HAS_INLINE_CHILD_FLAG) {
		o->ifc.emplace(bfc, bfc.contg_left_edge, bfc.contg_right_edge - bfc.contg_left_edge);
		LOG(INFO)
			<< "begin IFC pos=" << scene2d::PointF(bfc.contg_left_edge, bfc.margin_bottom_edge)
			<< ", bfc_bottom=" << bfc.border_bottom_edge << ", " << bfc.margin_bottom_edge;

		{
			base::scoped_setter _1(bfc.contg_height, box.prefer_height);
			base::scoped_setter _2(bfc.contg_left_edge, bfc.contg_left_edge + box.contentRect().left);
			base::scoped_setter _3(bfc.contg_right_edge, bfc.contg_left_edge + box.contentRect().width());
			LayoutObject* child = o->first_child;
			do {
				arrange(child, *o->ifc);
				child = child->next_sibling;
			} while (child != o->first_child);
			o->ifc->layoutArrange(o->style->text_align);
		}

		BlockBox& box = absl::get<BlockBox>(o->box);
		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom_edge = bfc.margin_bottom_edge + *box.prefer_height;
			bfc.margin_bottom_edge = bfc.border_bottom_edge;
		} else {
			box.content.height = o->ifc->getLayoutHeight();
			if (box.content.height > 0) {
				bfc.border_bottom_edge = bfc.margin_bottom_edge + box.content.height;
				bfc.margin_bottom_edge = bfc.border_bottom_edge;
			} else {
				// check top and bottom margin collapse, below
			}
		}
		LOG(INFO)
			<< "end IFC size=" << box.content
			<< ", bfc_bottom=" << bfc.border_bottom_edge << ", " << bfc.margin_bottom_edge;
	} else {
		BlockBox& box = absl::get<BlockBox>(o->box);
		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom_edge = bfc.margin_bottom_edge + *box.prefer_height;
			bfc.margin_bottom_edge = bfc.border_bottom_edge;
		}
	}
}

void LayoutObject::arrangeBlockBottom(LayoutObject* o, BlockFormatContext& bfc)
{
	CHECK(absl::holds_alternative<BlockBox>(o->box));

	BlockBox& box = absl::get<BlockBox>(o->box);
	float borpad_bottom = box.border.bottom + box.padding.bottom;

	if (o->flags & NEW_BFC_FLAG) {
		BlockFormatContext& inner_bfc = o->bfc.value();
		bfc.border_bottom_edge = bfc.margin_bottom_edge = inner_bfc.margin_bottom_edge;
	} else {
		if (borpad_bottom > 0) {
			bfc.border_bottom_edge = bfc.margin_bottom_edge + borpad_bottom;
			bfc.margin_bottom_edge = bfc.border_bottom_edge + box.margin.bottom;
		} else {
			float coll_margin = collapse_margin(box.margin.bottom,
				(bfc.margin_bottom_edge - bfc.border_bottom_edge));
			bfc.margin_bottom_edge = bfc.border_bottom_edge + coll_margin;
		}
	}
}

class TextBoxFlow : public graph2d::TextFlowSourceInterface, public graph2d::TextFlowSinkInterface {
public:
	TextBoxFlow(TextBox& tb, InlineFormatContext& ifc)
		: text_box_(tb), ifc_(ifc)
	{}

	// 通过 TextFlowSourceInterface 继承
	style::LineBox* getCurrentLine(float fontHeight, float& left, float& width, bool& empty) override
	{
		LineBox* line = ifc_.getCurrentLine();
		left = line->left + line->offset_x;
		width = line->avail_width - line->offset_x;
		empty = !(line->offset_x > 0.0f);
		return line;
	}

	style::LineBox* getNextLine(float fontHeight, float& left, float& width) override
	{
		LineBox* line = ifc_.getNextLine();
		left = line->left + line->offset_x;
		width = line->avail_width - line->offset_x;
		return line;
	}

	// 通过 TextFlowSinkInterface 继承
	void prepare(size_t glyph_count) override
	{
		text_box_.glyph_run_boxes.glyph_runs.reserve(glyph_count);
		text_box_.glyph_run_boxes.inline_boxes.reserve(glyph_count);
	}

	void addGlyphRun(style::LineBox* line, const scene2d::PointF& pos, std::unique_ptr<graph2d::GlyphRunInterface> glyph_run) override
	{
		std::unique_ptr<InlineBox> inline_box = std::make_unique<InlineBox>();
		inline_box->type = InlineBoxType::WithGlyphRun;
		inline_box->payload = glyph_run.get();
		auto fm = text_box_.text_flow->flowMetrics();
		inline_box->baseline = fm.baseline;
		inline_box->size = glyph_run->boundingRect().size();

		inline_box->line_box = line;
		inline_box->line_box->addInlineBox(inline_box.get());
		inline_box->line_box_offset_x = inline_box->line_box->offset_x;
		inline_box->line_box->offset_x += inline_box->size.width;

		text_box_.glyph_run_boxes.inline_boxes.push_back(std::move(inline_box));
		text_box_.glyph_run_boxes.glyph_runs.push_back(std::move(glyph_run));
	}

private:
	TextBox& text_box_;
	InlineFormatContext& ifc_;
};

void LayoutObject::arrange(LayoutObject* o, InlineFormatContext& ifc)
{
	if (absl::holds_alternative<TextBox>(o->box)) {
		TextBox& tb = absl::get<TextBox>(o->box);
		TextBoxFlow tbf(tb, ifc);
		tb.glyph_run_boxes.reset();
		tb.text_flow->flowText(&tbf, &tbf);
	}
	if (o->flags & HAS_INLINE_CHILD_FLAG) {
		LayoutObject* child = o->first_child;
		do {
			arrange(child, ifc);
			child = child->next_sibling;
		} while (child != o->first_child);
	}
}

void LayoutObject::paint(LayoutObject* o, graph2d::PainterInterface* painter)
{
	const Style& st = *o->style;
	
	if (absl::holds_alternative<BlockBox>(o->box)) {
		const BlockBox& b = absl::get<BlockBox>(o->box);
		scene2d::RectF border_rect = b.borderRect();
		scene2d::RectF render_rect = scene2d::RectF::fromXYWH(
			b.pos.x + border_rect.left,
			b.pos.y + border_rect.top,
			border_rect.width(),
			border_rect.height());
		painter->drawBox(
			render_rect,
			st.border_top_width.pixelOrZero(),
			st.background_color,
			st.border_color);
	} else if (absl::holds_alternative<InlineBox>(o->box)) {

	} else if (absl::holds_alternative<TextBox>(o->box)) {
		const TextBox& tb = absl::get<TextBox>(o->box);
		size_t n = tb.glyph_run_boxes.glyph_runs.size();
		for (size_t i = 0; i < n; ++i) {
			InlineBox* ibox = tb.glyph_run_boxes.inline_boxes[i].get();
			graph2d::GlyphRunInterface* gr = tb.glyph_run_boxes.glyph_runs[i].get();
			scene2d::PointF baseline_origin = ibox->pos;
			baseline_origin.y += ibox->baseline;
			painter->drawGlyphRun(baseline_origin, gr, st.color);
		}
	}

	LayoutObject* child = o->first_child;
	if (child) do {
		paint(child, painter);
		child = child->next_sibling;
	} while (child != o->first_child);
}

LayoutTreeBuilder::LayoutTreeBuilder(scene2d::Node* node)
	: root_(node) {}
std::vector<FlowRoot> LayoutTreeBuilder::build()
{
	std::vector<FlowRoot> flow_roots;

	flow_roots.push_back({ &root_->layout_, nullptr });
	initFlowRoot(root_);
	scene2d::Node::eachChild(root_, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));

	while (!abs_pos_nodes_.empty()) {
		auto nodes = std::move(abs_pos_nodes_);
		for (auto node : nodes) {
			auto pnode = node->positionedAncestor();
			auto po = pnode ? &pnode->layout_ : nullptr;
			flow_roots.push_back({ &node->layout_, po });
			initFlowRoot(node);
			scene2d::Node::eachChild(node, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));
		}
	}

	return flow_roots;
}

void LayoutTreeBuilder::initFlowRoot(scene2d::Node* node)
{
	node->inline_box_ = style::InlineBox();
	node->block_box_ = style::BlockBox(&node->computed_style_);
	node->bfc_.emplace(node);
	node->ifc_ = absl::nullopt;
	current_ = &node->layout_;
	current_->reset();
	current_->flags |= LayoutObject::NEW_BFC_FLAG;
	current_->bfc.emplace(node);
	current_->box = BlockBox(&node->computed_style_);
}

void LayoutTreeBuilder::prepareChild(scene2d::Node* node)
{
	beginChild(&node->layout_);
	if (node->type() == scene2d::NodeType::NODE_TEXT) {
		addText(node);
	} else if (node->type() == scene2d::NodeType::NODE_ELEMENT) {
		if (node->absolutelyPositioned()) {
			abs_pos_nodes_.push_back(node);
			return;
		}

		// 'static' or 'relative' positioned
		const auto& st = node->computedStyle();
		if (st.display == style::DisplayType::Block) {
			parentAddBlockChild();
		} else if (st.display == style::DisplayType::Inline) {
			parentAddInlineChild();
		} else if (st.display == style::DisplayType::InlineBlock) {
			parentAddInlineChild();
		}

		// establish new BFC
		if (st.display == style::DisplayType::InlineBlock
			|| st.overflow_x != style::OverflowType::Visible
			|| st.overflow_y != style::OverflowType::Visible
			|| new_bfc_pending_) {

			new_bfc_pending_ = false;
			current_->flags |= LayoutObject::NEW_BFC_FLAG;
			current_->bfc = absl::make_optional<BlockFormatContext>(node);
		}
	}
	scene2d::Node::eachChild(node, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));

	endChild();
}

void LayoutTreeBuilder::addText(scene2d::Node* node)
{
	parentAddInlineChild();

	TextBox tb;
	tb.text_flow = node->text_flow_.get();
	current_->box.emplace<TextBox>(std::move(tb));
}

void LayoutTreeBuilder::beginChild(LayoutObject* o)
{
	//if (current_->type != BlockBoxType::Empty && current_->type != BlockBoxType::WithBlockChildren) {
	//	LOG(WARNING) << "begin Block in containing block " << current_->type << " not implemented.";
	//}
	o->reset();

	o->parent = current_;
	if (last_child_) {
		o->next_sibling = last_child_->next_sibling;
		o->prev_sibling = last_child_;
		last_child_->next_sibling = o;
		o->next_sibling->prev_sibling = o;
	} else {
		o->next_sibling = o->prev_sibling = o;
		o->parent->first_child = o;
	}
	last_child_ = o;
	stack_.push_back(std::make_tuple(current_, last_child_));

	current_ = o;
	last_child_ = nullptr;
}

void LayoutTreeBuilder::endChild()
{
	if (reparent_to_anon_block_pending_) {
		reparent_to_anon_block_pending_ = false;

		auto parent = current_->parent;
		auto prev = current_->prev_sibling;
		auto next = current_->next_sibling;
		auto anon = current_->anon_block.get();

		prev->next_sibling = next;
		next->prev_sibling = prev;

		current_->parent = anon;
		current_->next_sibling = current_->prev_sibling = current_;

		anon->first_child = current_;
		anon->parent = parent;
		if (parent) {
			if (parent->first_child == current_)
				parent->first_child = anon;
			if (prev != current_)
				prev->next_sibling = anon;
			if (next != current_)
				next->prev_sibling = anon;
		}
	}
	std::tie(current_, last_child_) = stack_.back();
	stack_.pop_back();
}

void LayoutTreeBuilder::parentAddBlockChild()
{
	if (current_->parent->flags & LayoutObject::HAS_INLINE_CHILD_FLAG) {
		current_->box = InlineBox();
		new_bfc_pending_ = true;
		return;
	}

	current_->parent->flags |= LayoutObject::HAS_BLOCK_CHILD_FLAG;
	current_->box = BlockBox(nullptr);
}

void LayoutTreeBuilder::parentAddInlineChild()
{
	if (current_->parent->flags & LayoutObject::HAS_BLOCK_CHILD_FLAG) {
		reparent_to_anon_block_pending_ = true;
		current_->anon_block = std::make_unique<LayoutObject>();
		current_->box = InlineBox();
	} else {
		current_->parent->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
		current_->box = InlineBox();
	}
}

}
