#include "LayoutObject.h"
#include "BoxConstraintSolver.h"
#include "scene2d/Node.h"
#include "graph2d/Painter.h"
#include "absl/functional/bind_front.h"
#include "base/log.h"
#include "graph2d/graph2d.h"
#include <algorithm>

namespace style {

static const float SCROLLBAR_GUTTER_WIDTH = 16.0f;

void LayoutObject::init(const Style* st, scene2d::Node* nd)
{
	style = st;
	node = nd;
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
	bfc.contg_left_edge = 0.0f;
	bfc.contg_right_edge = viewport_size.width;
	bfc.border_bottom_edge = bfc.margin_bottom_edge = 0.0f;
	bfc.contg_height.emplace(viewport_size.height);
	bfc.max_border_right_edge = bfc.max_border_bottom_edge = 0.0f;

	if (fl.positioned_parent) {
		if (absl::holds_alternative<BlockBox>(fl.positioned_parent->box)) {
			const auto& rect = absl::get<BlockBox>(fl.positioned_parent->box).paddingRect();
			bfc.contg_left_edge = rect.left;
			bfc.contg_right_edge = rect.right;
			bfc.border_bottom_edge = bfc.margin_bottom_edge = rect.top;
			bfc.contg_height.emplace(rect.height());
			bfc.max_border_right_edge = bfc.contg_left_edge;
			bfc.max_border_bottom_edge = bfc.border_bottom_edge;
		} else if (absl::holds_alternative<std::vector<InlineBox>>(fl.positioned_parent->box)) {
			const auto& ibs = absl::get<std::vector<InlineBox>>(fl.positioned_parent->box);
			if (!ibs.empty()) {
				const InlineBox& first = ibs.front();
				const InlineBox& last = ibs.back();
				bfc.contg_left_edge = first.pos.x;
				bfc.contg_right_edge = last.pos.x + last.size.width;
				bfc.border_bottom_edge = bfc.margin_bottom_edge = first.pos.y;
				bfc.contg_height.emplace(std::max(0.0f, last.pos.y + last.size.height - first.pos.y));
				bfc.max_border_right_edge = bfc.contg_left_edge;
				bfc.max_border_right_edge = bfc.border_bottom_edge;
			}
		}
	}

	arrangeBlock(fl.root, bfc, viewport_size);
}

void LayoutObject::paint(LayoutObject* o, graph2d::PainterInterface* painter)
{
	const Style& st = *o->style;

	static int depth = 0;
	base::scoped_setter _(depth, depth + 1);
	LOG(INFO) << std::string(depth, '-') << " paint " << *o;

	if (absl::holds_alternative<BlockBox>(o->box)) {
		const BlockBox& b = absl::get<BlockBox>(o->box);
		scene2d::RectF border_rect = b.borderRect();
		scene2d::RectF render_rect = scene2d::RectF::fromXYWH(
			b.pos.x + border_rect.left,
			b.pos.y + border_rect.top,
			border_rect.width(),
			border_rect.height());
		// LOG(INFO) << "paint box: " << render_rect;
		painter->drawBox(
			render_rect,
			st.border_top_width.pixelOrZero(),
			st.background_color,
			st.border_color);
	} else if (absl::holds_alternative<std::vector<InlineBox>>(o->box)) {

	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		const InlineBlockBox& ibb = absl::get<InlineBlockBox>(o->box);
		if (!ibb.inline_boxes.empty()) {
			const InlineBox& ib = ibb.inline_boxes.front();
			const BlockBox& b = ibb.block_box;
			scene2d::RectF border_rect = b.borderRect();
			scene2d::RectF render_rect = scene2d::RectF::fromXYWH(
				b.pos.x + border_rect.left,
				b.pos.y + border_rect.top,
				border_rect.width(),
				border_rect.height());
			// LOG(INFO) << "paint box: " << render_rect;
			painter->drawBox(
				render_rect,
				st.border_top_width.pixelOrZero(),
				st.background_color,
				st.border_color);
		}
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

	if (st.position == PositionType::Absolute) {
		painter->setTranslation(absl::get<BlockBox>(o->box).pos, true);
	}

	LayoutObject* child = o->first_child;
	if (child) do {
		paint(child, painter);
		child = child->next_sibling;
	} while (child != o->first_child);

	if (st.position == PositionType::Absolute) {
		painter->restore();
	}
}

LayoutObject* LayoutObject::pick(LayoutObject* o, scene2d::PointF pos, int flag_mask, scene2d::PointF* out_local_pos)
{
	const Style& st = *o->style;
	LayoutObject* pick_result = nullptr;

	if (o->node->text_ == "mini") {
		int kk = 1;
	}

	if (absl::holds_alternative<BlockBox>(o->box)) {
		const BlockBox& b = absl::get<BlockBox>(o->box);
		scene2d::RectF border_rect = b.borderRect();
		scene2d::RectF render_rect = scene2d::RectF::fromXYWH(
			b.pos.x + border_rect.left,
			b.pos.y + border_rect.top,
			border_rect.width(),
			border_rect.height());
		if (o->node && o->node->testFlags(flag_mask) && render_rect.contains(pos)) {
			if (out_local_pos)
				*out_local_pos = pos - render_rect.origin();
			pick_result = o;
		}
	} else if (absl::holds_alternative<std::vector<InlineBox>>(o->box)) {
		const auto& ibs = absl::get<std::vector<InlineBox>>(o->box);
		for (auto it = ibs.rbegin(); it != ibs.rend(); ++it) {
			const scene2d::RectF rect = it->boundingRect();
			if (o->node && o->node->testFlags(flag_mask) && rect.contains(pos)) {
				if (out_local_pos)
					*out_local_pos = pos - rect.origin();
				pick_result = o;
			}
		}
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		const BlockBox& b = absl::get<InlineBlockBox>(o->box).block_box;
		scene2d::RectF border_rect = b.borderRect();
		scene2d::RectF render_rect = scene2d::RectF::fromXYWH(
			b.pos.x + border_rect.left,
			b.pos.y + border_rect.top,
			border_rect.width(),
			border_rect.height());
		if (o->node && o->node->testFlags(flag_mask) && render_rect.contains(pos)) {
			if (out_local_pos)
				*out_local_pos = pos - render_rect.origin();
			pick_result = o;
		}
	} else if (absl::holds_alternative<TextBox>(o->box)) {
	}

	if (st.position == PositionType::Absolute) {
		pos -= absl::get<BlockBox>(o->box).pos;
	}

	LayoutObject* child = o->first_child ? o->first_child->prev_sibling : nullptr;
	if (child) do {
		LayoutObject* picked_child = pick(child, pos, flag_mask, out_local_pos);
		if (picked_child) {
			pick_result = picked_child;
			break;
		}
		child = child->prev_sibling;
	} while (child != o->first_child->prev_sibling);

	return pick_result;
}

scene2d::PointF LayoutObject::getOffset(LayoutObject* o)
{
	if (absl::holds_alternative<BlockBox>(o->box)) {
		const auto& rect = absl::get<BlockBox>(o->box).paddingRect();
		return rect.origin();
	} else if (absl::holds_alternative<std::vector<InlineBox>>(o->box)) {
		const auto& ibs = absl::get<std::vector<InlineBox>>(o->box);
		if (!ibs.empty()) {
			const InlineBox& first = ibs.front();
			const InlineBox& last = ibs.back();
			return first.pos;
		}
	}
	return scene2d::PointF();
}

void LayoutObject::measure(LayoutObject* o, float viewport_height)
{
	//static int depth = 0;
	//base::scoped_setter _(depth, depth + 1);
	//LOG(INFO) << std::string(depth, '-') << " measure " << o << " " << *o;

	LayoutObject* child = o->first_child;
	if (child) do {
		measure(child, viewport_height);
		child = child->next_sibling;
	} while (child != o->first_child);

	const style::Style& st = *o->style;
	float min_width = 0.0f, max_width = std::numeric_limits<float>::infinity();
	bool depends_children_width = false;
	if (st.display == DisplayType::Block || st.display == DisplayType::InlineBlock) {
		float width = try_resolve_to_px(st.width, absl::nullopt).value_or(0);

		if (st.width.isPixel() || st.width.isRaw()) {
			min_width = std::max(0.0f, width);
			max_width = std::max(0.0f, width);
		} else {
			depends_children_width = true;
		}
	} else if (st.display == DisplayType::Inline) {
		if (absl::holds_alternative<TextBox>(o->box)) {
			TextBox& tb = absl::get<TextBox>(o->box);
			// TODO: handle parent margin_border_padding_left(right)
			std::tie(min_width, max_width) = tb.text_flow->measureWidth();
		} else {
			depends_children_width = true;
		}
	}

	if (depends_children_width) {
		if (o->flags & LayoutObject::HAS_BLOCK_CHILD_FLAG) {
			LayoutObject* child = o->first_child;
			if (child) do {
				const style::Style& cst = *child->style;
				float margin_left = try_resolve_to_px(cst.margin_left, absl::nullopt).value_or(0);
				float border_left = try_resolve_to_px(cst.border_left_width, absl::nullopt).value_or(0);
				float padding_left = try_resolve_to_px(cst.padding_left, absl::nullopt).value_or(0);
				float padding_right = try_resolve_to_px(cst.padding_right, absl::nullopt).value_or(0);
				float border_right = try_resolve_to_px(cst.border_right_width, absl::nullopt).value_or(0);
				float margin_right = try_resolve_to_px(cst.margin_right, absl::nullopt).value_or(0);
				float mbp_width = margin_left + border_left + padding_left + padding_right + border_right + margin_right;

				min_width = std::max(min_width, mbp_width + child->min_width);
				if (max_width == std::numeric_limits<float>::infinity()) {
					max_width = mbp_width + child->max_width;
				} else {
					max_width = std::max(max_width, mbp_width + child->max_width);
				}

				child = child->next_sibling;
			} while (child != o->first_child);
		} else if (o->flags & LayoutObject::HAS_INLINE_CHILD_FLAG) {
			LayoutObject* child = o->first_child;
			if (child) do {
				float mbp_width = 0;
				const style::Style& cst = *child->style;
				if (cst.display == DisplayType::Block || cst.display == DisplayType::InlineBlock) {
					float margin_left = try_resolve_to_px(cst.margin_left, absl::nullopt).value_or(0);
					float border_left = try_resolve_to_px(cst.border_left_width, absl::nullopt).value_or(0);
					float padding_left = try_resolve_to_px(cst.padding_left, absl::nullopt).value_or(0);
					float padding_right = try_resolve_to_px(cst.padding_right, absl::nullopt).value_or(0);
					float border_right = try_resolve_to_px(cst.border_right_width, absl::nullopt).value_or(0);
					float margin_right = try_resolve_to_px(cst.margin_right, absl::nullopt).value_or(0);
					mbp_width = margin_left + border_left + padding_left + padding_right + border_right + margin_right;
				}

				min_width = std::max(min_width, mbp_width + child->min_width);
				if (max_width == std::numeric_limits<float>::infinity()) {
					max_width = mbp_width + child->max_width;
				} else {
					max_width += mbp_width + child->max_width;
				}

				child = child->next_sibling;
			} while (child != o->first_child);
		}
	}
	o->min_width = min_width;
	o->max_width = max_width;
}

void LayoutObject::arrangeBlock(LayoutObject* o, BlockFormatContext& bfc, const scene2d::DimensionF& viewport_size)
{
	const Style& st = *o->style;
	ScrollbarPolicy scroll_y = ScrollbarPolicy::Hidden;
	if (st.overflow_y == OverflowType::Scroll)
		scroll_y = ScrollbarPolicy::Stable;

	float saved_border_bottom_edge = bfc.border_bottom_edge;
	float saved_margin_bottom_edge = bfc.margin_bottom_edge;
	float saved_max_border_right_edge = bfc.max_border_right_edge;
	float saved_max_border_bottom_edge = bfc.max_border_bottom_edge;
	arrangeBlock(o, bfc, viewport_size, scroll_y);

	const BlockBox& b = absl::get<BlockBox>(o->box);
	if (st.overflow_y == OverflowType::Auto && bfc.max_border_bottom_edge > b.pos.y + b.paddingRect().bottom) {
		// handle overflow-y
		scroll_y = ScrollbarPolicy::Stable;
		bfc.border_bottom_edge = saved_border_bottom_edge;
		bfc.margin_bottom_edge = saved_margin_bottom_edge;
		bfc.max_border_right_edge = saved_max_border_right_edge;
		bfc.max_border_bottom_edge = saved_max_border_bottom_edge;
		arrangeBlock(o, bfc, viewport_size, scroll_y);
	}
	if (scroll_y != ScrollbarPolicy::Hidden) {
		scene2d::DimensionF inner_size = b.innerPaddingRect().size();
		if (!o->scroll_data.has_value()) {
			o->scroll_data.emplace();
			ScrollData& sd = o->scroll_data.value();
			sd.content_size.width = std::max(inner_size.width,
				bfc.max_border_right_edge - saved_max_border_right_edge);
			sd.content_size.height = std::max(inner_size.height,
				bfc.max_border_bottom_edge - saved_max_border_bottom_edge);
			sd.viewport_rect.right = inner_size.width;
			sd.viewport_rect.bottom = inner_size.height;
		} else {
			ScrollData& sd = o->scroll_data.value();
			sd.content_size.width = std::max(inner_size.width,
				bfc.max_border_right_edge - saved_max_border_right_edge);
			sd.content_size.height = std::max(inner_size.height,
				bfc.max_border_bottom_edge - saved_max_border_bottom_edge);
			sd.viewport_rect.left = std::max(0.0f, std::min(sd.viewport_rect.left,
				sd.content_size.width - inner_size.width));
			sd.viewport_rect.top = std::max(0.0f, std::min(sd.viewport_rect.top,
				sd.content_size.height - inner_size.height));
			sd.viewport_rect.right = sd.viewport_rect.left + inner_size.width;
			sd.viewport_rect.bottom = sd.viewport_rect.top + inner_size.height;
		}
		LOG(INFO)
			<< "scrollData contentSize " << o->scroll_data->content_size
			<< ", viewportRect " << o->scroll_data->viewport_rect;
	} else {
		o->scroll_data = absl::nullopt;
	}
	bfc.max_border_right_edge = std::max(bfc.max_border_right_edge, b.pos.x + b.borderRect().right);
	bfc.max_border_bottom_edge = std::max(bfc.max_border_bottom_edge, bfc.border_bottom_edge);
}

void LayoutObject::arrangeBlock(LayoutObject* o,
	BlockFormatContext& bfc,
	const scene2d::DimensionF& viewport_size,
	ScrollbarPolicy scroll_y)
{
	const Style& st = *o->style;
	if (st.display == DisplayType::Block) {
		arrangeBlockX(o, bfc, viewport_size, scroll_y);
		if (o->flags & NEW_BFC_FLAG) {
			BlockFormatContext& inner_bfc = o->bfc.value();
			if (st.position == PositionType::Absolute) {
				inner_bfc.contg_left_edge = 0;
				inner_bfc.contg_right_edge = bfc.contg_right_edge - bfc.contg_left_edge;
				inner_bfc.contg_height = bfc.contg_height;
				inner_bfc.max_border_right_edge = 0;
				inner_bfc.border_bottom_edge = 0;
				inner_bfc.margin_bottom_edge = 0;
			} else {
				inner_bfc.contg_left_edge = bfc.contg_left_edge;
				inner_bfc.contg_right_edge = bfc.contg_right_edge;
				inner_bfc.contg_height = bfc.contg_height;
				inner_bfc.max_border_right_edge = bfc.contg_left_edge;
				inner_bfc.border_bottom_edge = bfc.margin_bottom_edge;
				inner_bfc.margin_bottom_edge = bfc.margin_bottom_edge;
			}
			arrangeBlockTop(o, inner_bfc);
			arrangeBlockChildren(o, inner_bfc, viewport_size);
			arrangeBlockBottom(o, inner_bfc);
			if (st.position == PositionType::Absolute) {
				CHECK(bfc.contg_height.has_value());
				BlockBox& box = absl::get<BlockBox>(o->box);
				float contg_blk_height = bfc.contg_height.value_or(0);
				AbsoluteBlockPositionSolver height_solver(contg_blk_height,
					try_resolve_to_px(st.top, contg_blk_height),
					try_resolve_to_px(st.height, contg_blk_height),
					try_resolve_to_px(st.bottom, contg_blk_height));
				height_solver.setLayoutHeight(box.marginRect().size().height);
				box.pos.y = height_solver.top();
			}
			bfc.border_bottom_edge = bfc.margin_bottom_edge = inner_bfc.margin_bottom_edge;
			bfc.max_border_right_edge = std::max(bfc.max_border_right_edge, inner_bfc.max_border_right_edge);
			bfc.max_border_bottom_edge = std::max(bfc.max_border_bottom_edge, inner_bfc.max_border_bottom_edge);
		} else {
			arrangeBlockTop(o, bfc);
			arrangeBlockChildren(o, bfc, viewport_size);
			arrangeBlockBottom(o, bfc);
		}
	} else {
		LOG(WARNING) << "LayoutObject::arrangeBlock " << st.display << " not implemented.";
	}
}

void LayoutObject::arrangeBlockX(LayoutObject* o,
	BlockFormatContext& bfc,
	const scene2d::DimensionF& viewport_size,
	ScrollbarPolicy scroll_y)
{
	const Style& st = *o->style;
	CHECK(st.display == DisplayType::Block);
	BlockBox& b = absl::get<BlockBox>(o->box);

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

	if (st.position == PositionType::Absolute) {
		style::AbsoluteBlockWidthSolver solver(
			clean_contg_width,
			try_resolve_to_px(st.left, contg_width),
			try_resolve_to_px(st.margin_left, contg_width),
			try_resolve_to_px(st.width, contg_width),
			try_resolve_to_px(st.margin_right, contg_width),
			try_resolve_to_px(st.right, contg_width));

		// Compute width, left and right margins
		style::WidthConstraint mw = solver.measureWidth();
		if (mw.type == style::WidthConstraintType::Fixed) {
			b.content.width = mw.value;
		} else if (mw.type == style::WidthConstraintType::MinContent) {
			b.content.width = o->min_width;
		} else if (mw.type == style::WidthConstraintType::MaxContent) {
			b.content.width = o->max_width;
		} else if (mw.type == style::WidthConstraintType::FitContent) {
			b.content.width = std::min(o->max_width, std::max(o->min_width, mw.value));
		} else {
			LOG(ERROR) << "BUG: unexpected width constraint type.";
			b.content.width = 0.0f;
		}
		solver.setLayoutWidth(b.content.width);

		b.margin.left = solver.marginLeft();
		b.border.left = try_resolve_to_px(st.border_left_width, contg_width).value_or(0);
		b.padding.left = try_resolve_to_px(st.padding_left, contg_width).value_or(0);
		b.padding.right = try_resolve_to_px(st.padding_right, contg_width).value_or(0);
		b.border.right = try_resolve_to_px(st.border_right_width, contg_width).value_or(0);
		b.margin.right = solver.marginRight();

		// Compute top and bottom margins
		b.margin.top = try_resolve_to_px(st.margin_top, contg_width).value_or(0);
		b.border.top = try_resolve_to_px(st.border_top_width, contg_width).value_or(0);
		b.padding.top = try_resolve_to_px(st.padding_top, contg_width).value_or(0);

		b.padding.bottom = try_resolve_to_px(st.padding_bottom, contg_width).value_or(0);
		b.border.bottom = try_resolve_to_px(st.border_bottom_width, contg_width).value_or(0);
		b.margin.bottom = try_resolve_to_px(st.margin_bottom, contg_width).value_or(0);

		b.pos.x = solver.left();
	} else {
		StaticBlockWidthSolver solver(
			clean_contg_width,
			try_resolve_to_px(st.margin_left, contg_width),
			try_resolve_to_px(st.width, contg_width),
			try_resolve_to_px(st.margin_right, contg_width));
		float avail_width = solver.measureWidth().value;
		solver.setLayoutWidth(avail_width);

		// Compute width, left and right margins
		b.content.width = solver.width();
		b.margin.left = solver.marginLeft();
		b.border.left = try_resolve_to_px(st.border_left_width, contg_width).value_or(0);
		b.padding.left = try_resolve_to_px(st.padding_left, contg_width).value_or(0);
		b.padding.right = try_resolve_to_px(st.padding_right, contg_width).value_or(0);
		b.border.right = try_resolve_to_px(st.border_right_width, contg_width).value_or(0);
		b.margin.right = solver.marginRight();

		if (scroll_y == ScrollbarPolicy::Stable) {
			b.inner_padding.right = SCROLLBAR_GUTTER_WIDTH;
		} else if (scroll_y == ScrollbarPolicy::StableBothEdges) {
			b.inner_padding.left = SCROLLBAR_GUTTER_WIDTH;
			b.inner_padding.right = SCROLLBAR_GUTTER_WIDTH;
		}
		b.content.width = std::max(0.0f, b.content.width - b.inner_padding.left - b.inner_padding.right);

		// Compute height, top and bottom margins
		b.margin.top = try_resolve_to_px(st.margin_top, contg_width).value_or(0);
		b.border.top = try_resolve_to_px(st.border_top_width, contg_width).value_or(0);
		b.padding.top = try_resolve_to_px(st.padding_top, contg_width).value_or(0);

		b.padding.bottom = try_resolve_to_px(st.padding_bottom, contg_width).value_or(0);
		b.border.bottom = try_resolve_to_px(st.border_bottom_width, contg_width).value_or(0);
		b.margin.bottom = try_resolve_to_px(st.margin_bottom, contg_width).value_or(0);

		// Compute pos.x
		b.pos.x = bfc.contg_left_edge;

		// Update max border_right_edge
		bfc.max_border_right_edge = std::max(bfc.max_border_right_edge, bfc.contg_left_edge + b.borderRect().right);
	}
}

void LayoutObject::arrangeBlockTop(LayoutObject* o, BlockFormatContext& bfc)
{
	const Style& st = *o->style;
	CHECK(absl::holds_alternative<BlockBox>(o->box));

	BlockBox& box = absl::get<BlockBox>(o->box);

	float borpad_top = box.border.top + box.padding.top;

	if (o->flags & NEW_BFC_FLAG) {
		box.pos.y = bfc.margin_bottom_edge;
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
	const scene2d::DimensionF& viewport_size)
{
	CHECK(absl::holds_alternative<BlockBox>(o->box));

	const Style& st = *o->style;
	BlockBox& box = absl::get<BlockBox>(o->box);
	float borpad_top = box.border.top + box.padding.top;
	float borpad_bottom = box.border.bottom + box.padding.bottom;

	if (st.position == PositionType::Absolute) {
		if (bfc.contg_height.has_value() && !box.prefer_height.has_value()
			&& (st.top.isPixel() || st.top.isRaw())
			&& (st.bottom.isPixel() || st.bottom.isRaw())) {
			float h = bfc.contg_height.value()
				- try_resolve_to_px(st.top, bfc.contg_height).value_or(0)
				- box.margin.top - box.border.top - box.padding.top - box.inner_padding.top
				- box.inner_padding.bottom - box.padding.bottom - box.border.bottom - box.margin.bottom
				- try_resolve_to_px(st.bottom, bfc.contg_height).value_or(0);
			box.prefer_height.emplace(std::max(0.0f, h));
		}
	} else {
		box.prefer_height = try_resolve_to_px(st.height, bfc.contg_height);
	}

	float saved_bfc_margin_bottom = bfc.margin_bottom_edge;
	if (o->flags & HAS_BLOCK_CHILD_FLAG) {
		{
			base::scoped_setter _1(bfc.contg_height, box.prefer_height);
			base::scoped_setter _2(bfc.contg_left_edge, bfc.contg_left_edge + box.contentRect().left);
			base::scoped_setter _3(bfc.contg_right_edge, bfc.contg_left_edge + box.contentRect().width());
			LayoutObject* child = o->first_child;
			do {
				arrangeBlock(child, bfc, viewport_size);
				child = child->next_sibling;
			} while (child != o->first_child);
		}

		BlockBox& box = absl::get<BlockBox>(o->box);
		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom_edge = bfc.margin_bottom_edge + *box.prefer_height;
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
		auto rect = box.contentRect();
		scene2d::PointF pos;
		if (o->style->position == PositionType::Static || o->style->position == PositionType::Relative) {
			pos = box.pos;
		}
		o->ifc.emplace(bfc, pos.x + rect.left, rect.width(), pos.y + rect.top);
		LOG(INFO)
			<< "begin IFC pos=" << scene2d::PointF(bfc.contg_left_edge, bfc.margin_bottom_edge)
			<< ", bfc_bottom=" << bfc.border_bottom_edge << ", " << bfc.margin_bottom_edge;

		{
			base::scoped_setter _1(bfc.contg_height, box.prefer_height);
			base::scoped_setter _2(bfc.contg_left_edge, bfc.contg_left_edge + box.pos.x + box.contentRect().left);
			base::scoped_setter _3(bfc.contg_right_edge, bfc.contg_left_edge + box.contentRect().width());
			LayoutObject* child = o->first_child;
			do {
				prepare(child, *o->ifc, viewport_size);
				child = child->next_sibling;
			} while (child != o->first_child);
			o->ifc->arrange(o->style->text_align);
			child = o->first_child;
			do {
				arrange(child, *o->ifc, viewport_size);
				child = child->next_sibling;
			} while (child != o->first_child);
		}

		//bfc.max_border_right_edge = std::max(bfc.max_border_right_edge,
		//	o->ifc->getMaxLineBoxRight());
		bfc.max_border_bottom_edge = std::max(bfc.max_border_bottom_edge,
			bfc.margin_bottom_edge + o->ifc->getLayoutHeight());

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
	const Style& st = *o->style;
	CHECK(absl::holds_alternative<BlockBox>(o->box));

	BlockBox& box = absl::get<BlockBox>(o->box);

	float borpad_bottom = box.border.bottom + box.padding.bottom;

	if (borpad_bottom > 0) {
		bfc.border_bottom_edge = bfc.margin_bottom_edge + borpad_bottom;
		bfc.margin_bottom_edge = bfc.border_bottom_edge + box.margin.bottom;
	} else {
		float coll_margin = collapse_margin(box.margin.bottom,
			(bfc.margin_bottom_edge - bfc.border_bottom_edge));
		bfc.margin_bottom_edge = bfc.border_bottom_edge + coll_margin;
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

		line->line_height = std::max(line->line_height, fm.line_height);

		text_box_.glyph_run_boxes.inline_boxes.push_back(std::move(inline_box));
		text_box_.glyph_run_boxes.glyph_runs.push_back(std::move(glyph_run));
	}

private:
	TextBox& text_box_;
	InlineFormatContext& ifc_;
};

void LayoutObject::prepare(LayoutObject* o, InlineFormatContext& ifc, const scene2d::DimensionF& viewport_size)
{
	const auto& st = *o->style;

	if (absl::holds_alternative<TextBox>(o->box)) {
		TextBox& tb = absl::get<TextBox>(o->box);
		TextBoxFlow tbf(tb, ifc);
		tb.glyph_run_boxes.reset();
		tb.text_flow->flowText(&tbf, &tbf);
	} else if (st.display == DisplayType::InlineBlock) {
		o->box.emplace<InlineBlockBox>();
		arrangeInlineBlock(o, ifc, viewport_size);
	} else if (o->flags & HAS_INLINE_CHILD_FLAG) {
		LayoutObject* child = o->first_child;
		if (child) do {
			prepare(child, ifc, viewport_size);
			child = child->next_sibling;
		} while (child != o->first_child);
	}
}

// arrangeBlock bottom up
void LayoutObject::arrange(LayoutObject* o, InlineFormatContext& ifc, const scene2d::DimensionF& viewport_size)
{
	const auto& st = *o->style;

	if (st.display == DisplayType::Inline) {
		if (o->flags & HAS_INLINE_CHILD_FLAG) {
			LayoutObject* child = o->first_child;
			std::vector<InlineBox*> inline_boxes;
			if (child) do {
				arrange(child, ifc, viewport_size);

				if (absl::holds_alternative<TextBox>(child->box)) {
					const auto& child_inline_boxes = absl::get<TextBox>(child->box).glyph_run_boxes.inline_boxes;
					for (auto& ib : child_inline_boxes) {
						inline_boxes.push_back(ib.get());
					}
				} else if (absl::holds_alternative<std::vector<InlineBox>>(child->box)) {
					auto& child_inline_boxes = absl::get<std::vector<InlineBox>>(child->box);
					for (auto& ib : child_inline_boxes) {
						inline_boxes.push_back(&ib);
					}
				} else if (absl::holds_alternative<InlineBlockBox>(child->box)) {
					auto& child_inline_boxes = absl::get<InlineBlockBox>(child->box).inline_boxes;
					for (auto& ib : child_inline_boxes) {
						inline_boxes.push_back(&ib);
					}
				}

				child = child->next_sibling;
			} while (child != o->first_child);

			std::vector<InlineBox> merged_boxes;
			for (InlineBox* child : inline_boxes) {
				if (merged_boxes.empty() || merged_boxes.back().line_box != child->line_box) {
					auto fm = graph2d::getFontMetrics(st.font_family.keyword_val.c_str(),
						st.font_size.pixelOrZero());
					float line_height = st.line_height.pixelOrZero();
					InlineBox ib;
					ib.pos = child->pos;
					ib.baseline = fm.baseline;
					ib.size.width = child->size.width;
					ib.size.height = fm.line_height;
					ib.line_box = child->line_box;
					ib.line_box->line_height = std::max(ib.line_box->line_height, line_height);
					merged_boxes.push_back(ib);
				} else {
					InlineBox& ib = merged_boxes.back();
					ib.size.width = std::max(child->pos.x + child->size.width,
						ib.pos.x + ib.size.width) - ib.pos.x;
				}
			}

			o->box.emplace<std::vector<InlineBox>>(std::move(merged_boxes));
		}
	} else if (st.display == DisplayType::InlineBlock) {
		InlineBlockBox& ibb = absl::get<InlineBlockBox>(o->box);
		if (!ibb.inline_boxes.empty()) {
			const InlineBox& ib = ibb.inline_boxes.front();
			ibb.block_box.pos = ib.pos;
			LayoutObject* child = o->first_child;
			if (child) do {
				translate(child, ib.pos);
				child = child->next_sibling;
			} while (child != o->first_child);
		}
	}
}

void LayoutObject::translate(LayoutObject* o, scene2d::PointF offset)
{
	if (absl::holds_alternative<TextBox>(o->box)) {
		auto& tb = absl::get<TextBox>(o->box);
		for (auto& ib : tb.glyph_run_boxes.inline_boxes) {
			ib->pos += offset;
		}
	} else if (absl::holds_alternative<std::vector<InlineBox>>(o->box)) {
		auto& ibs = absl::get<std::vector<InlineBox>>(o->box);
		for (auto& ib : ibs) {
			ib.pos += offset;
		}
	} else if (absl::holds_alternative<BlockBox>(o->box)) {
		auto& b = absl::get<BlockBox>(o->box);
		b.pos += offset;
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		auto& ibb = absl::get<InlineBlockBox>(o->box);
		CHECK(!ibb.inline_boxes.empty());
		if (!ibb.inline_boxes.empty()) {
			auto& ib = ibb.inline_boxes.front();
			ib.pos += offset;
		}
		ibb.block_box.pos += offset;
	}

	LayoutObject* child = o->first_child;
	if (child) do {
		translate(child, offset);
		child = child->next_sibling;
	} while (child != o->first_child);
}

void LayoutObject::arrangeInlineBlock(LayoutObject* o, InlineFormatContext& ifc, const scene2d::DimensionF& viewport_size)
{
	CHECK(o->style->display == DisplayType::InlineBlock)
		<< "LayoutObject::arrangeInlineBlock: expect inline-block element";
	CHECK(o->bfc.has_value())
		<< "BUG: LayoutObject::arrangeInlineBlock: expect inner BFC";

	const Style& st = *o->style;
	BlockFormatContext& bfc = ifc.bfc();

	float contg_width = bfc.contg_right_edge - bfc.contg_left_edge;
	float mbp_width = try_resolve_to_px(st.margin_left, contg_width).value_or(0)
		+ try_resolve_to_px(st.border_left_width, contg_width).value_or(0)
		+ try_resolve_to_px(st.padding_left, contg_width).value_or(0)
		+ try_resolve_to_px(st.padding_right, contg_width).value_or(0)
		+ try_resolve_to_px(st.border_right_width, contg_width).value_or(0)
		+ try_resolve_to_px(st.margin_right, contg_width).value_or(0);
	float min_width = mbp_width + o->min_width;
	float max_width = mbp_width + o->max_width;
	LineBox* line = ifc.getLineBox(max_width);
	float inline_pos_x = line->left + line->offset_x;
	float fit_width = std::max(min_width, std::min(max_width, line->avail_width - line->offset_x));
	line->offset_x += fit_width;

	ScrollbarPolicy scroll_y = ScrollbarPolicy::Hidden;
	if (st.overflow_y == OverflowType::Scroll) {
		scroll_y = ScrollbarPolicy::Stable;
	} else if (st.overflow_y == OverflowType::Auto) {
		if (min_width > fit_width) {
			scroll_y = ScrollbarPolicy::Stable;
		}
	}

	arrangeInlineBlockX(o, bfc, viewport_size, scroll_y);
	arrangeInlineBlockChildren(o, bfc.contg_height, viewport_size);

	InlineBlockBox& ibb = absl::get<InlineBlockBox>(o->box);
	ibb.inline_boxes.resize(1);
	InlineBox& ib = ibb.inline_boxes.front();
	ib.line_box = line;
	ib.line_box->addInlineBox(&ib);
	ib.pos.x = inline_pos_x;
	ib.size = ibb.block_box.marginRect().size();
	if (scroll_y == ScrollbarPolicy::Hidden) {
		absl::optional<float> baseline = find_first_baseline(o);
		ib.baseline = baseline.value_or(ib.size.height);
	} else {
		ib.baseline = ib.size.height;
	}
	line->line_height = std::max(line->line_height, ib.size.height);
}

void LayoutObject::arrangeInlineBlockX(LayoutObject* o,
	BlockFormatContext& bfc,
	const scene2d::DimensionF& viewport_size,
	ScrollbarPolicy scroll_y)
{
	const Style& st = *o->style;
	CHECK(st.display == DisplayType::InlineBlock);

	float contg_width = bfc.contg_right_edge - bfc.contg_left_edge;

	// Compute width, left and right margins
	BlockBox& b = absl::get<InlineBlockBox>(o->box).block_box;
	b.content.width = try_resolve_to_px(st.width, contg_width).value_or(0);
	b.margin.left = try_resolve_to_px(st.margin_left, contg_width).value_or(0);
	b.border.left = try_resolve_to_px(st.border_left_width, contg_width).value_or(0);
	b.padding.left = try_resolve_to_px(st.padding_left, contg_width).value_or(0);
	b.padding.right = try_resolve_to_px(st.padding_right, contg_width).value_or(0);
	b.border.right = try_resolve_to_px(st.border_right_width, contg_width).value_or(0);
	b.margin.right = try_resolve_to_px(st.margin_right, contg_width).value_or(0);

	if (st.width.isAuto()) {
		b.content.width = std::max(o->min_width, std::min(o->max_width, contg_width));
	} else {
		b.content.width = try_resolve_to_px(st.width, contg_width).value_or(0);
	}

	if (scroll_y == ScrollbarPolicy::Stable) {
		b.inner_padding.right = SCROLLBAR_GUTTER_WIDTH;
	} else if (scroll_y == ScrollbarPolicy::StableBothEdges) {
		b.inner_padding.left = SCROLLBAR_GUTTER_WIDTH;
		b.inner_padding.right = SCROLLBAR_GUTTER_WIDTH;
	}
	b.content.width = std::max(0.0f, b.content.width - b.inner_padding.left - b.inner_padding.right);

	// Compute height, top and bottom margins
	b.margin.top = try_resolve_to_px(st.margin_top, contg_width).value_or(0);
	b.border.top = try_resolve_to_px(st.border_top_width, contg_width).value_or(0);
	b.padding.top = try_resolve_to_px(st.padding_top, contg_width).value_or(0);

	b.padding.bottom = try_resolve_to_px(st.padding_bottom, contg_width).value_or(0);
	b.border.bottom = try_resolve_to_px(st.border_bottom_width, contg_width).value_or(0);
	b.margin.bottom = try_resolve_to_px(st.margin_bottom, contg_width).value_or(0);

	// No need to compute pos.x
	// b.pos.x = bfc.contg_left_edge;

	// Update max border_right_edge
	bfc.max_border_right_edge = std::max(bfc.max_border_right_edge, bfc.contg_left_edge + b.borderRect().right);
}

void LayoutObject::arrangeInlineBlockChildren(LayoutObject* o,
	absl::optional<float> contg_height, const scene2d::DimensionF& viewport_size)
{
	CHECK(absl::holds_alternative<InlineBlockBox>(o->box));

	const Style& st = *o->style;
	BlockBox& box = absl::get<InlineBlockBox>(o->box).block_box;
	float borpad_top = box.border.top + box.padding.top;
	float borpad_bottom = box.border.bottom + box.padding.bottom;

	BlockFormatContext& bfc = o->bfc.value();
	bfc.contg_left_edge = box.contentRect().left;
	bfc.contg_right_edge = box.contentRect().right;
	bfc.max_border_right_edge = bfc.contg_left_edge;
	bfc.border_bottom_edge = bfc.margin_bottom_edge = box.margin.top + borpad_top;
	bfc.contg_height = contg_height;
	box.prefer_height = try_resolve_to_px(st.height, bfc.contg_height);

	float saved_bfc_margin_bottom = bfc.margin_bottom_edge;
	if (o->flags & HAS_BLOCK_CHILD_FLAG) {
		{
			LayoutObject* child = o->first_child;
			do {
				arrangeBlock(child, bfc, viewport_size);
				child = child->next_sibling;
			} while (child != o->first_child);
		}

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
		auto rect = box.contentRect();
		o->ifc.emplace(bfc, bfc.contg_left_edge, rect.width(), rect.top);
		LOG(INFO)
			<< "begin IFC pos=" << scene2d::PointF(bfc.contg_left_edge, bfc.margin_bottom_edge)
			<< ", bfc_bottom=" << bfc.border_bottom_edge << ", " << bfc.margin_bottom_edge;

		{
			LayoutObject* child = o->first_child;
			do {
				prepare(child, *o->ifc, viewport_size);
				child = child->next_sibling;
			} while (child != o->first_child);
			o->ifc->arrange(st.text_align);
			child = o->first_child;
			do {
				arrange(child, *o->ifc, viewport_size);
				child = child->next_sibling;
			} while (child != o->first_child);
		}

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
		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom_edge = bfc.margin_bottom_edge + *box.prefer_height;
			bfc.margin_bottom_edge = bfc.border_bottom_edge;
		}
	}

	bfc.border_bottom_edge = bfc.margin_bottom_edge + borpad_bottom;
	bfc.margin_bottom_edge = bfc.border_bottom_edge + box.margin.bottom;
}

absl::optional<float> LayoutObject::find_first_baseline(LayoutObject* o, float accum_y)
{
	if (o->ifc.has_value()) {
		for (const std::unique_ptr<LineBox>& line : o->ifc->lineBoxes()) {
			for (const InlineBox* ib : line->inline_boxes) {
				return ib->pos.y + ib->baseline + accum_y;
			}
		}
	}

	if (o->style->display == DisplayType::InlineBlock) {
		const auto& ibb = absl::get<InlineBlockBox>(o->box);
		if (!ibb.inline_boxes.empty())
			accum_y += ibb.inline_boxes.front().pos.y;
	}

	LayoutObject* child = o->first_child;
	if (child) do {
		auto baseline = find_first_baseline(child, accum_y);
		if (baseline.has_value())
			return baseline;
		child = child->next_sibling;
	} while (child != o->first_child);
	return absl::nullopt;
}

LayoutTreeBuilder::LayoutTreeBuilder(scene2d::Node* node)
	: root_(node) {}
std::vector<FlowRoot> LayoutTreeBuilder::build()
{
	std::vector<FlowRoot> flow_roots;

	flow_roots.push_back({ &root_->layout_, nullptr });
	initFlowRoot(root_);
	flow_root_ = &flow_roots.back();
	scene2d::Node::eachChild(root_, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));

	while (!abs_pos_nodes_.empty()) {
		auto nodes = std::move(abs_pos_nodes_);
		for (auto node : nodes) {
			auto pnode = node->positionedAncestor();
			auto po = pnode ? &pnode->layout_ : nullptr;
			flow_roots.push_back({ &node->layout_, po });
			flow_root_ = &flow_roots.back();
			initFlowRoot(node);
			scene2d::Node::eachChild(node, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));
		}
	}
	flow_root_ = nullptr;

	return flow_roots;
}

void LayoutTreeBuilder::initFlowRoot(scene2d::Node* node)
{
	node->inline_box_ = InlineBox();
	node->block_box_ = BlockBox();
	node->bfc_.emplace(node);
	node->ifc_ = absl::nullopt;
	current_ = &node->layout_;
	current_->reset();
	current_->flags |= LayoutObject::NEW_BFC_FLAG;
	current_->bfc.emplace(node);
	current_->box = BlockBox();
	last_child_ = nullptr;
}

void LayoutTreeBuilder::prepareChild(scene2d::Node* node)
{
	if (node->type() == scene2d::NodeType::NODE_TEXT) {
		addText(node);
	} else if (node->type() == scene2d::NodeType::NODE_ELEMENT) {
		if (node->positioned()) {
			flow_root_->relatives.push_back(&node->layout_);
			if (node->absolutelyPositioned()) {
				abs_pos_nodes_.push_back(node);
				return;
			}
		}

		beginChild(&node->layout_);

		// 'static' or 'relative' positioned
		const auto& st = node->computedStyle();
		if (st.display == style::DisplayType::Block) {
			if (current_->parent->flags & LayoutObject::HAS_INLINE_CHILD_FLAG) {
				LOG(ERROR) << "LayoutTreeBuilder::prepareChild(): unexpected block child found, BUG!";
				abort();
			}
			current_->parent->flags |= LayoutObject::HAS_BLOCK_CHILD_FLAG;
			current_->box.emplace<BlockBox>();
		} else if (st.display == style::DisplayType::Inline) {
			if (current_->node->type() == scene2d::NodeType::NODE_TEXT
				&& (current_->parent->style->display == DisplayType::Block
					|| current_->parent->style->display == DisplayType::InlineBlock)) {
				reparent_to_anon_block_pending_ = true;
				current_->anon_span = std::make_unique<LayoutObject>();
				current_->anon_span->init(current_->style, current_->node);
				current_->anon_span->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG | LayoutObject::ANON_SPAN_FLAG;
			} else {
				current_->parent->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
			}
			current_->box.emplace<std::vector<InlineBox>>();
		} else if (st.display == style::DisplayType::InlineBlock) {
			current_->parent->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
			current_->box.emplace<InlineBlockBox>();
		}

		// establish new BFC
		if (st.display == style::DisplayType::InlineBlock
			|| st.overflow_x != style::OverflowType::Visible
			|| st.overflow_y != style::OverflowType::Visible
			|| new_bfc_pending_) {

			new_bfc_pending_ = false;
			current_->flags |= LayoutObject::NEW_BFC_FLAG;
			current_->bfc.emplace(node);
		}
		scene2d::Node::eachChild(node, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));
		endChild();
	}
}

void LayoutTreeBuilder::addText(scene2d::Node* node)
{
	TextBox tb;
	tb.text_flow = node->text_flow_.get();
	node->layout_.box.emplace<TextBox>(std::move(tb));

	if (current_->style->display == DisplayType::Block
			|| current_->style->display == DisplayType::InlineBlock) {
		current_->anon_span = std::make_unique<LayoutObject>();
		auto anon = current_->anon_span.get();
		anon->init(&node->computed_style_, node);
		anon->flags = LayoutObject::HAS_INLINE_CHILD_FLAG | LayoutObject::ANON_SPAN_FLAG;
		anon->box.emplace<std::vector<InlineBox>>();

		CHECK(!(current_->flags & LayoutObject::HAS_BLOCK_CHILD_FLAG));
		current_->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
		current_->first_child = anon;
		last_child_ = anon;
		anon->parent = current_;

		anon->first_child = &node->layout_;
		node->layout_.parent = anon;
	} else {
		CHECK(!(current_->flags & LayoutObject::HAS_BLOCK_CHILD_FLAG));
		current_->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
		current_->first_child = &node->layout_;
		node->layout_.parent = current_;
	}
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
	stack_.push_back(std::make_tuple(current_, last_child_, reparent_to_anon_block_pending_));

	current_ = o;
	last_child_ = nullptr;
	reparent_to_anon_block_pending_ = false;
}

void LayoutTreeBuilder::endChild()
{
	std::tie(current_, last_child_, reparent_to_anon_block_pending_) = stack_.back();
	stack_.pop_back();
}

void LayoutTreeBuilder::parentAddBlockChild()
{
	if (current_->parent->flags & LayoutObject::HAS_INLINE_CHILD_FLAG) {
		LOG(ERROR) << "LayoutTreeBuilder::parentAddBlockChild(): invalid state";
		new_bfc_pending_ = true;
		return;
	}

	current_->parent->flags |= LayoutObject::HAS_BLOCK_CHILD_FLAG;
	current_->box = BlockBox();
}

void LayoutTreeBuilder::parentAddInlineChild()
{
	if (current_->node->type() == scene2d::NodeType::NODE_TEXT
		&& (current_->parent->style->display == DisplayType::Block 
			|| current_->parent->style->display == DisplayType::InlineBlock)) {
		reparent_to_anon_block_pending_ = true;
		current_->anon_span = std::make_unique<LayoutObject>();
		current_->anon_span->init(current_->style, current_->node);
		current_->anon_span->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG | LayoutObject::ANON_SPAN_FLAG;
	} else {
		current_->parent->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
	}
}

}
