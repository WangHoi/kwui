#include "LayoutObject.h"
#include "BoxConstraintSolver.h"
#include "scene2d/Node.h"
#include "graph2d/Painter.h"
#include "absl/functional/bind_front.h"
#include "base/log.h"
#include "graph2d/graph2d.h"
#include <algorithm>

namespace style {

static const float SCROLLBAR_GUTTER_WIDTH = 8.0f;

void LayoutObject::init(const Style* st, scene2d::Node* nd)
{
	style = st;
	node = nd;
	next_sibling = prev_sibling = nullptr;
	first_child = last_child = nullptr;
}

void LayoutObject::reset()
{
	flags = 0;
	bfc = absl::nullopt;
	ifc = absl::nullopt;
	aux_boxes.clear();
	min_width = 0.0f;
	max_width = std::numeric_limits<float>::infinity();
	prefer_height = absl::nullopt;
	scroll_object = absl::nullopt;
	parent = nullptr;
	first_child = nullptr;
	last_child = nullptr;
	next_sibling = prev_sibling = nullptr;
	positioned_children.clear();
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
	bfc.contg_width = viewport_size.width;
	bfc.border_bottom_edge = bfc.margin_bottom_edge = 0.0f;
	bfc.contg_height.emplace(viewport_size.height);

	if (fl.positioned_parent) {
		auto containing_rect = LayoutObject::containingRectForPositionedChildren(fl.positioned_parent);
		if (containing_rect.has_value()) {
			bfc.contg_width = containing_rect.value().width();
			bfc.contg_height.emplace(containing_rect.value().height());
		} else {
			LOG(INFO) << "reflow: containing block rect for positioned parent not found.";
		}
	}

	arrangeBlock(fl.root, bfc, viewport_size);
}

void LayoutObject::paint(LayoutObject* o, graph2d::PainterInterface* painter)
{
	const Style& st = *o->style;

	//static int depth = 0;
	//base::scoped_setter _(depth, depth + 1);
	//LOG(INFO) << std::string(depth, '-') << " paint " << *o;

	absl::optional<scene2d::RectF> content_rect;

	if (absl::holds_alternative<BlockBox>(o->box)) {
		const BlockBox& b = absl::get<BlockBox>(o->box);
		content_rect.emplace(b.contentRect().translated(b.pos));
		CornerRadiusF border_radius;
		border_radius.top_left = st.border_top_left_radius.pixelOrZero();
		border_radius.top_right = st.border_top_right_radius.pixelOrZero();
		border_radius.bottom_right = st.border_bottom_right_radius.pixelOrZero();
		border_radius.bottom_left = st.border_bottom_left_radius.pixelOrZero();
		// LOG(INFO) << "paint box: " << render_rect;
		painter->drawBox(
			b.paddingRect().translated(b.pos),
			b.border,
			border_radius,
			st.background_color,
			st.border_color,
			st.background_image.get());
	} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(o->box)) {
		/*
		const auto& ibs = absl::get<std::vector<InlineBox>>(o->box);
		if (!ibs.empty()) {
			const auto& first = ibs.front();
			const auto& last = ibs.back();
			content_rect.emplace(scene2d::RectF::fromLTRB(
				std::min(first.pos.x, last.pos.x),
				std::min(first.pos.y, last.pos.y),
				std::max(first.pos.x + first.size.width, last.pos.x + last.size.width),
				std::max(first.pos.y + first.size.height, last.pos.y + last.size.height)));
		}
		*/
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		const InlineBlockBox& ibb = absl::get<InlineBlockBox>(o->box);
		const InlineBox& ib = *ibb.inline_box;
		const BlockBox& b = ibb.block_box;
		content_rect.emplace(b.contentRect().translated(b.pos));
		CornerRadiusF border_radius;
		border_radius.top_left = st.border_top_left_radius.pixelOrZero();
		border_radius.top_right = st.border_top_right_radius.pixelOrZero();
		border_radius.bottom_right = st.border_bottom_right_radius.pixelOrZero();
		border_radius.bottom_left = st.border_bottom_left_radius.pixelOrZero();
		//LOG(INFO) << "paint box: " << render_rect;
		painter->drawBox(
			b.paddingRect().translated(b.pos),
			b.border,
			border_radius,
			st.background_color,
			st.border_color,
			st.background_image.get());
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

	scene2d::PointF scroll_offset;
	if (o->scroll_object.has_value()) {
		ScrollObject& so = o->scroll_object.value();

		scroll_offset = -so.scroll_offset;
		scene2d::PointF ipad_origin;
		absl::optional<scene2d::RectF> v_scrollbar_rect, h_scrollbar_rect;
		if (absl::holds_alternative<BlockBox>(o->box)) {
			const BlockBox& b = absl::get<BlockBox>(o->box);
			scene2d::RectF ipad_rect = b.clientRect();
			ipad_origin = b.pos + ipad_rect.origin();
			if (b.scrollbar_gutter.right > 0) {
				v_scrollbar_rect.emplace(scene2d::RectF::fromXYWH(
					ipad_origin.x + so.viewport_size.width,
					ipad_origin.y,
					b.scrollbar_gutter.right,
					so.viewport_size.height));
			}
			if (b.scrollbar_gutter.bottom > 0) {
				h_scrollbar_rect.emplace(scene2d::RectF::fromXYWH(
					ipad_origin.x,
					ipad_origin.y + so.viewport_size.height,
					so.viewport_size.width,
					b.scrollbar_gutter.bottom));
			}
		} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
			const InlineBlockBox& ibb = absl::get<InlineBlockBox>(o->box);
			scene2d::RectF ipad_rect = ibb.block_box.clientRect();
			ipad_origin = ibb.block_box.pos + ipad_rect.origin();
			if (ibb.block_box.scrollbar_gutter.right > 0) {
				v_scrollbar_rect.emplace(scene2d::RectF::fromXYWH(
					ipad_origin.x + so.viewport_size.width,
					ipad_origin.y,
					ibb.block_box.scrollbar_gutter.right,
					so.viewport_size.height));
			}
			if (ibb.block_box.scrollbar_gutter.bottom > 0) {
				h_scrollbar_rect.emplace(scene2d::RectF::fromXYWH(
					ipad_origin.x,
					ipad_origin.y + so.viewport_size.height,
					so.viewport_size.width,
					ibb.block_box.scrollbar_gutter.bottom));
			}
		}

		if (v_scrollbar_rect.has_value()) {
			ScrollObject::paintVScrollbar(&so, painter, v_scrollbar_rect.value());
		}
		if (h_scrollbar_rect.has_value()) {
			ScrollObject::paintHScrollbar(&so, painter, h_scrollbar_rect.value());
		}

		painter->pushClipRect(ipad_origin, so.viewport_size);
	}

	painter->save();
	painter->setTranslation(content_rect.value_or(scene2d::RectF()).origin() + scroll_offset, true);

	if (o->node && o->node->control_ && content_rect.has_value()) {
		painter->drawControl(scene2d::RectF::fromOriginSize(scene2d::PointF(), content_rect.value().size()), o->node->control_.get());
	}

	LayoutObject* child = o->first_child;
	while (child) {
		paint(child, painter);
		child = child->next_sibling;
	}

	painter->restore();

	for (LayoutObject* child : o->positioned_children) {
		if (child->style->position == PositionType::Relative) {
			continue;
			//painter->setTranslation(content_rect.value_or(scene2d::RectF()).origin() + scroll_offset, true);
		}
		painter->setTranslation(containingRectForPositionedChildren(o).value_or(scene2d::RectF()).origin() + scroll_offset, true);
		paint(child, painter);
		painter->restore();
	}

	if (o->scroll_object.has_value()) {
		painter->popClipRect();
	}
}

LayoutObject* LayoutObject::pick(LayoutObject* o, scene2d::PointF pos, int flag_mask, scene2d::PointF* out_local_pos)
{
	const Style& st = *o->style;
	LayoutObject* pick_result = nullptr;

	auto saved_pos = pos;
	if (absl::holds_alternative<BlockBox>(o->box)) {
		const BlockBox& b = absl::get<BlockBox>(o->box);
		scene2d::RectF border_rect = b.borderRect().translated(b.pos);
		scene2d::PointF local_pos = pos - b.pos - b.contentRect().origin();
		if (border_rect.contains(pos) && o->node && o->node->hitTest(local_pos, flag_mask)) {
			// LOG(INFO) << "hit " << o->node->tag_ << ", pos " << pos << ", border_rect " << border_rect << ", content_rect " << b.contentRect();
			if (out_local_pos)
				*out_local_pos = local_pos;
			pick_result = o;
		}
		pos = local_pos;
	} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(o->box)) {
		const auto& ibs = absl::get<std::vector<std::unique_ptr<InlineBox>>>(o->box);
		for (auto it = ibs.rbegin(); it != ibs.rend(); ++it) {
			const auto& ib = *it;
			const scene2d::RectF local_rect = scene2d::RectF::fromOriginSize(scene2d::PointF(), ib->size);
			scene2d::PointF local_pos = pos - ib->pos;
			if (local_rect.contains(local_pos) && o->node && o->node->hitTest(local_pos, flag_mask)) {
				if (out_local_pos)
					*out_local_pos = local_pos;
				pick_result = o;
			}
		}
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		const BlockBox& b = absl::get<InlineBlockBox>(o->box).block_box;
		scene2d::RectF border_rect = b.borderRect().translated(b.pos);
		scene2d::PointF local_pos = pos - b.pos - b.contentRect().origin();
		if (border_rect.contains(pos) && o->node && o->node->hitTest(local_pos, flag_mask)) {
			if (out_local_pos)
				*out_local_pos = local_pos;
			pick_result = o;
		}
		pos = local_pos;
	} else if (absl::holds_alternative<TextBox>(o->box)) {
	}

	if (o->scroll_object.has_value()) {
		pos += o->scroll_object.value().scroll_offset;
	}

	LayoutObject* child = o->last_child ? o->last_child : nullptr;
	while (child) {
		LayoutObject* picked_child = pick(child, pos, flag_mask, out_local_pos);
		if (picked_child) {
			pick_result = picked_child;
			break;
		}
		child = child->prev_sibling;
	}

	for (auto it = o->positioned_children.rbegin(); it != o->positioned_children.rend(); ++it) {
		auto p = (*it)->style->position == PositionType::Relative
			? pos
			: saved_pos - containingRectForPositionedChildren(o).value_or(scene2d::RectF()).origin();
		LayoutObject* picked_child = pick(*it, p, flag_mask, out_local_pos);
		if (picked_child) {
			pick_result = picked_child;
			break;
		}
	}

	return pick_result;
}

scene2d::PointF LayoutObject::getOffset(LayoutObject* o)
{
	if (absl::holds_alternative<BlockBox>(o->box)) {
		const auto& rect = absl::get<BlockBox>(o->box).paddingRect();
		return rect.origin();
	} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(o->box)) {
		const auto& ibs = absl::get<std::vector<std::unique_ptr<InlineBox>>>(o->box);
		if (!ibs.empty()) {
			const InlineBox& first = *ibs.front();
			const InlineBox& last = *ibs.back();
			return first.pos;
		}
	}
	return scene2d::PointF();
}

scene2d::PointF LayoutObject::pos(LayoutObject* o)
{
	if (absl::holds_alternative<BlockBox>(o->box)) {
		return absl::get<BlockBox>(o->box).pos;
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		return absl::get<InlineBlockBox>(o->box).block_box.pos;
	}
	return scene2d::PointF();
}

void LayoutObject::setPos(LayoutObject* o, const scene2d::PointF& pos)
{
	if (absl::holds_alternative<BlockBox>(o->box)) {
		absl::get<BlockBox>(o->box).pos = pos;
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		absl::get<InlineBlockBox>(o->box).block_box.pos = pos;
	}
}

absl::optional<scene2d::RectF> LayoutObject::getChildrenBoundingRect(LayoutObject* o)
{
	absl::optional<scene2d::RectF> rect;

	if (o->flags & HAS_BLOCK_CHILD_FLAG) {
		auto child = o->first_child;
		while (child) {
			auto p = LayoutObject::contentRect(o).origin() + LayoutObject::pos(child);
			scene2d::RectF child_border_rect = LayoutObject::borderRect(child)
				.translate(p);
			if (rect.has_value()) {
				rect.value().unite(child_border_rect);
			} else {
				rect = child_border_rect;
			}
			if (child->style->overflow_x == OverflowType::Visible
				&& child->style->overflow_y == OverflowType::Visible) {

				scene2d::RectF child_content_rect = LayoutObject::contentRect(child);
				auto child_cbrect = LayoutObject::getChildrenBoundingRect(child);
				if (child_cbrect.has_value()) {
					child_cbrect.value().translate(child_content_rect.origin());
					if (rect.has_value()) {
						rect.value().unite(child_cbrect.value());
					} else {
						rect = child_cbrect.value();
					}
				}
			}
			child = child->next_sibling;
		}
	} else if (o->flags & HAS_INLINE_CHILD_FLAG) {
		if (o->ifc.has_value()) {
			rect = scene2d::RectF::fromOriginSize(scene2d::PointF(),
				scene2d::DimensionF(o->ifc->getLayoutWidth(), o->ifc->getLayoutHeight()));
			rect.value().translate(LayoutObject::contentRect(o).origin());
		}
	}

	absl::optional<scene2d::RectF> crect = LayoutObject::containingRectForPositionedChildren(o);
	if (crect.has_value()) {
		for (LayoutObject* child : o->positioned_children) {
			auto p = crect.value().origin() + LayoutObject::pos(child);
			scene2d::RectF child_border_rect = LayoutObject::borderRect(child)
				.translate(p);
			if (rect.has_value()) {
				rect.value().unite(child_border_rect);
			} else {
				rect = child_border_rect;
			}
			if (child->style->overflow_x == OverflowType::Visible
				&& child->style->overflow_y == OverflowType::Visible) {

				scene2d::RectF child_content_rect = LayoutObject::contentRect(child);
				auto child_cbrect = LayoutObject::getChildrenBoundingRect(child);
				if (child_cbrect.has_value()) {
					child_cbrect.value().translate(child_content_rect.origin());
					if (rect.has_value()) {
						rect.value().unite(child_cbrect.value());
					} else {
						rect = child_cbrect.value();
					}
				}
			}
		}
	}

	return rect;
}

scene2d::RectF LayoutObject::borderRect(LayoutObject* o)
{
	if (absl::holds_alternative<BlockBox>(o->box)) {
		return absl::get<BlockBox>(o->box).borderRect();
	} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(o->box)) {
		const auto& ibs = absl::get<std::vector<std::unique_ptr<InlineBox>>>(o->box);
		if (ibs.empty())
			return scene2d::RectF();
		absl::optional<scene2d::RectF> rect;
		for (auto it = ibs.begin(); it != ibs.end(); ++it) {
			const auto& ib = *it;
			scene2d::RectF ib_rect = scene2d::RectF::fromOriginSize(ib->pos, ib->size);
			if (!rect.has_value()) {
				rect.emplace(ib_rect);
			} else {
				rect.value().unite(ib_rect);
			}
		}
		return rect.has_value() ? rect.value() : scene2d::RectF();
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		return absl::get<InlineBlockBox>(o->box).block_box.borderRect();
	} else {
		return scene2d::RectF();
	}
}

scene2d::RectF LayoutObject::paddingRect(LayoutObject* o)
{
	if (absl::holds_alternative<BlockBox>(o->box)) {
		return absl::get<BlockBox>(o->box).paddingRect();
	} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(o->box)) {
		const auto& ibs = absl::get<std::vector<std::unique_ptr<InlineBox>>>(o->box);
		if (ibs.empty())
			return scene2d::RectF();
		absl::optional<scene2d::RectF> rect;
		for (auto it = ibs.begin(); it != ibs.end(); ++it) {
			const auto& ib = *it;
			scene2d::RectF ib_rect = scene2d::RectF::fromOriginSize(ib->pos, ib->size);
			if (!rect.has_value()) {
				rect.emplace(ib_rect);
			} else {
				rect.value().unite(ib_rect);
			}
		}
		return rect.has_value() ? rect.value() : scene2d::RectF();
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		return absl::get<InlineBlockBox>(o->box).block_box.paddingRect();
	} else {
		return scene2d::RectF();
	}
}

scene2d::RectF LayoutObject::contentRect(LayoutObject* o)
{
	if (absl::holds_alternative<BlockBox>(o->box)) {
		return absl::get<BlockBox>(o->box).contentRect();
	} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(o->box)) {
		const auto& ibs = absl::get<std::vector<std::unique_ptr<InlineBox>>>(o->box);
		if (ibs.empty())
			return scene2d::RectF();
		absl::optional<scene2d::RectF> rect;
		for (auto it = ibs.begin(); it != ibs.end(); ++it) {
			const auto& ib = *it;
			scene2d::RectF ib_rect = scene2d::RectF::fromOriginSize(ib->pos, ib->size);
			if (!rect.has_value()) {
				rect.emplace(ib_rect);
			} else {
				rect.value().unite(ib_rect);
			}
		}
		return rect.has_value() ? rect.value() : scene2d::RectF();
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		return absl::get<InlineBlockBox>(o->box).block_box.contentRect();
	} else {
		return scene2d::RectF();
	}
}

void LayoutObject::measure(LayoutObject* o, float viewport_height)
{
	//static int depth = 0;
	//base::scoped_setter _(depth, depth + 1);
	//LOG(INFO) << std::string(depth, '-') << " measure " << o << " " << *o;

	LayoutObject* child = o->first_child;
	while (child) {
		measure(child, viewport_height);
		child = child->next_sibling;
	}

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
			while (child) {
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
			}
		} else if (o->flags & LayoutObject::HAS_INLINE_CHILD_FLAG) {
			LayoutObject* child = o->first_child;
			while (child) {
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
			}
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
	arrangeBlock(o, bfc, viewport_size, scroll_y);

	BlockBox& b = absl::get<BlockBox>(o->box);
	scene2d::RectF padding_rect = LayoutObject::paddingRect(o);
	absl::optional<scene2d::RectF> children_bounding_rect = LayoutObject::getChildrenBoundingRect(o);
	if (st.overflow_y == OverflowType::Auto
		&& children_bounding_rect.has_value()
		&& children_bounding_rect.value().bottom > padding_rect.bottom) {
		// handle overflow-y
		scroll_y = ScrollbarPolicy::Stable;
		bfc.border_bottom_edge = saved_border_bottom_edge;
		bfc.margin_bottom_edge = saved_margin_bottom_edge;
		arrangeBlock(o, bfc, viewport_size, scroll_y);
	}

	// Check overflow-x
	ScrollbarPolicy scroll_x = ScrollbarPolicy::Hidden;
	if (st.overflow_x == OverflowType::Scroll) {
		scroll_x = ScrollbarPolicy::Stable;
	}
	if (st.overflow_x == OverflowType::Auto
		&& children_bounding_rect.has_value()
		&& children_bounding_rect.value().right > padding_rect.bottom) {
		scroll_x = ScrollbarPolicy::Stable;
	}
	if (scroll_x == ScrollbarPolicy::Stable) {
		b.scrollbar_gutter.bottom = SCROLLBAR_GUTTER_WIDTH;
	} else if (scroll_x == ScrollbarPolicy::StableBothEdges) {
		b.scrollbar_gutter.top = b.scrollbar_gutter.bottom = SCROLLBAR_GUTTER_WIDTH;
	}
	b.content.height = std::max(0.0f, b.content.height - b.scrollbar_gutter.top - b.scrollbar_gutter.bottom);

	// Update ScrollObject
	if (st.overflow_y != OverflowType::Visible || st.overflow_x != OverflowType::Visible) {
		const BlockFormatContext& inner_bfc = o->bfc.value();
		scene2d::RectF inner_rect = b.clientRect();
		float padding_left_edge = b.pos.x + b.paddingRect().left;
		float padding_top_edge = b.pos.y + b.paddingRect().top;
		scene2d::RectF content_rect = LayoutObject::getChildrenBoundingRect(o).value_or(scene2d::RectF());
		if (!o->scroll_object.has_value()) {
			o->scroll_object.emplace();
		}
		ScrollObject& sd = o->scroll_object.value();
		sd.content_size.width = std::max(inner_rect.size().width, content_rect.right - b.paddingRect().left);
		sd.content_size.height = std::max(inner_rect.size().height, content_rect.bottom - b.paddingRect().top);
		sd.viewport_size = inner_rect.size();
		//LOG(INFO)
		//	<< "scrollData contentSize " << o->scroll_object->content_size
		//	<< ", viewportRect " << o->scroll_object->viewport_rect;
	} else {
		o->scroll_object = absl::nullopt;
	}
}

void LayoutObject::arrangeBlock(LayoutObject* o,
	BlockFormatContext& bfc,
	const scene2d::DimensionF& viewport_size,
	ScrollbarPolicy scroll_y)
{
	const Style& st = *o->style;
	BlockBox& box = absl::get<BlockBox>(o->box);
	if (st.display == DisplayType::Block) {
		arrangeBlockX(o, bfc, viewport_size, scroll_y);
		if (o->flags & NEW_BFC_FLAG) {
			BlockFormatContext& inner_bfc = o->bfc.value();
			arrangeBfcTop(o, bfc, box);
			if (st.position == PositionType::Absolute) {
				inner_bfc.contg_width = bfc.contg_width;
				inner_bfc.contg_height = bfc.contg_height;
				inner_bfc.border_bottom_edge = 0;
				inner_bfc.margin_bottom_edge = 0;
			} else {
				inner_bfc.contg_width = bfc.contg_width;
				inner_bfc.contg_height = box.prefer_height;
				inner_bfc.border_bottom_edge = bfc.margin_bottom_edge;
				inner_bfc.margin_bottom_edge = bfc.margin_bottom_edge;
			}
			arrangeBfcChildren(o, inner_bfc, box, viewport_size);
			if (st.overflow_y == OverflowType::Visible) {
				bfc.border_bottom_edge = bfc.margin_bottom_edge = inner_bfc.margin_bottom_edge;
			} else {
				float box_content_bottom_edge = box.pos.y + box.contentRect().bottom;
				bfc.border_bottom_edge = bfc.margin_bottom_edge = std::max(bfc.margin_bottom_edge, box_content_bottom_edge);
			}

			arrangeBfcBottom(o, bfc, box);
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
		} else {
			arrangeBfcTop(o, bfc, box);
			arrangeBfcChildren(o, bfc, box, viewport_size);
			arrangeBfcBottom(o, bfc, box);
		}

		LayoutObject::arrangePositionedChildren(o, viewport_size);
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

	float contg_width = bfc.contg_width;
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
			b.scrollbar_gutter.right = SCROLLBAR_GUTTER_WIDTH;
		} else if (scroll_y == ScrollbarPolicy::StableBothEdges) {
			b.scrollbar_gutter.left = SCROLLBAR_GUTTER_WIDTH;
			b.scrollbar_gutter.right = SCROLLBAR_GUTTER_WIDTH;
		}
		b.content.width = std::max(0.0f, b.content.width - b.scrollbar_gutter.left - b.scrollbar_gutter.right);

		// Compute height, top and bottom margins
		b.margin.top = try_resolve_to_px(st.margin_top, contg_width).value_or(0);
		b.border.top = try_resolve_to_px(st.border_top_width, contg_width).value_or(0);
		b.padding.top = try_resolve_to_px(st.padding_top, contg_width).value_or(0);

		b.padding.bottom = try_resolve_to_px(st.padding_bottom, contg_width).value_or(0);
		b.border.bottom = try_resolve_to_px(st.border_bottom_width, contg_width).value_or(0);
		b.margin.bottom = try_resolve_to_px(st.margin_bottom, contg_width).value_or(0);

		// Compute pos.x
		b.pos.x = 0.0f;
	}
}

void LayoutObject::arrangeBfcTop(LayoutObject* o, BlockFormatContext& bfc, BlockBox& box)
{
	const Style& st = *o->style;

	float borpad_top = box.border.top + box.padding.top;

	if (o->flags & NEW_BFC_FLAG) {
		box.pos.y = bfc.margin_bottom_edge;
		bfc.border_bottom_edge = bfc.margin_bottom_edge = bfc.margin_bottom_edge + box.margin.top + borpad_top;
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

	// Box prefer height
	box.prefer_height = try_resolve_to_px(st.height, bfc.contg_height);
	if (st.position == PositionType::Absolute) {
		if (bfc.contg_height.has_value() && !box.prefer_height.has_value()
			&& (st.top.isPixel() || st.top.isRaw() || st.top.unit == ValueUnit::Percent)
			&& (st.bottom.isPixel() || st.bottom.isRaw() || st.bottom.unit == ValueUnit::Percent)) {
			float h = bfc.contg_height.value()
				- try_resolve_to_px(st.top, bfc.contg_height).value_or(0)
				- box.margin.top - box.border.top - box.padding.top - box.scrollbar_gutter.top
				- box.scrollbar_gutter.bottom - box.padding.bottom - box.border.bottom - box.margin.bottom
				- try_resolve_to_px(st.bottom, bfc.contg_height).value_or(0);
			box.prefer_height.emplace(std::max(0.0f, h));
		}
	}
}

void LayoutObject::arrangeBfcChildren(LayoutObject* o,
	BlockFormatContext& bfc,
	BlockBox& box,
	const scene2d::DimensionF& viewport_size)
{
	const Style& st = *o->style;
	float borpad_top = box.border.top + box.padding.top;
	float borpad_bottom = box.border.bottom + box.padding.bottom;

	float saved_bfc_margin_bottom = bfc.margin_bottom_edge;
	if (o->flags & HAS_BLOCK_CHILD_FLAG) {
		float saved_border_bottom_edge = bfc.border_bottom_edge;
		float saved_margin_bottom_edge = bfc.margin_bottom_edge;

		base::scoped_setter _1(bfc.contg_height, box.prefer_height);
		//base::scoped_setter _2(bfc.contg_left_edge, bfc.contg_left_edge + box.contentRect().left);
		base::scoped_setter _3(bfc.contg_width, box.contentRect().width());
		float margin = bfc.margin_bottom_edge - bfc.border_bottom_edge;
		bfc.margin_bottom_edge = 0.0f;
		bfc.border_bottom_edge = -margin;
		LayoutObject* child = o->first_child;
		while (child) {
			arrangeBlock(child, bfc, viewport_size);
			child = child->next_sibling;
		}

		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom_edge = saved_border_bottom_edge + std::max(bfc.border_bottom_edge, *box.prefer_height);
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
			bfc.border_bottom_edge += saved_border_bottom_edge + margin;
			bfc.margin_bottom_edge += saved_margin_bottom_edge;
		}
	} else if (o->flags & HAS_INLINE_CHILD_FLAG) {
		auto content_rect = box.contentRect();
		scene2d::PointF pos;
		if (o->style->position == PositionType::Static || o->style->position == PositionType::Relative) {
			pos = box.pos;
		}
		o->ifc.emplace(o, bfc, content_rect.width());
		{
			base::scoped_setter _1(bfc.contg_height, box.prefer_height);
			// base::scoped_setter _2(bfc.contg_left_edge, bfc.contg_left_edge + box.pos.x + box.contentRect().left);
			base::scoped_setter _3(bfc.contg_width, box.contentRect().width());
			LayoutObject* child = o->first_child;
			while (child) {
				prepare(child, *o->ifc, viewport_size);
				child = child->next_sibling;
			}
			o->ifc->arrange(o->style->text_align);
			child = o->first_child;
			while (child) {
				arrange(child, *o->ifc, viewport_size);
				child = child->next_sibling;
			}
		}

		//bfc.max_border_right_edge = std::max(bfc.max_border_right_edge,
		//	o->ifc->getMaxLineBoxRight());

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
	} else {
		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom_edge = bfc.margin_bottom_edge + *box.prefer_height;
			bfc.margin_bottom_edge = bfc.border_bottom_edge;
		}
	}
}

void LayoutObject::arrangeBfcBottom(LayoutObject* o, BlockFormatContext& bfc, BlockBox& box)
{
	const Style& st = *o->style;

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
	TextBoxFlow(LayoutObject* o, TextBox& tb, InlineFormatContext& ifc)
		: layout_object_(o), text_box_(tb), ifc_(ifc)
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
		auto fm = text_box_.text_flow->fontMetrics();
		inline_box->baseline = fm.baseline();
		inline_box->size = glyph_run->boundingRect().size();

		inline_box->line_box = line;
		inline_box->line_box->addInlineBox(layout_object_, inline_box.get());
		inline_box->line_box_offset_x = inline_box->line_box->offset_x;
		inline_box->line_box->offset_x += inline_box->size.width;

		line->line_height = std::max(line->line_height, fm.lineHeight());

		text_box_.glyph_run_boxes.inline_boxes.push_back(std::move(inline_box));
		text_box_.glyph_run_boxes.glyph_runs.push_back(std::move(glyph_run));
	}

private:
	LayoutObject* layout_object_ = nullptr;
	TextBox& text_box_;
	InlineFormatContext& ifc_;
};

void LayoutObject::prepare(LayoutObject* o, InlineFormatContext& ifc, const scene2d::DimensionF& viewport_size)
{
	const auto& st = *o->style;

	if (absl::holds_alternative<TextBox>(o->box)) {
		TextBox& tb = absl::get<TextBox>(o->box);
		TextBoxFlow tbf(o, tb, ifc);
		tb.glyph_run_boxes.reset();
		tb.text_flow->flowText(&tbf, &tbf);
	} else if (st.display == DisplayType::InlineBlock) {
		o->box.emplace<InlineBlockBox>();
		arrangeInlineBlock(o, ifc, viewport_size);
	} else if (o->flags & HAS_INLINE_CHILD_FLAG) {
		LayoutObject* child = o->first_child;
		while (child) {
			prepare(child, ifc, viewport_size);
			child = child->next_sibling;
		}
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
			while (child) {
				arrange(child, ifc, viewport_size);

				if (absl::holds_alternative<TextBox>(child->box)) {
					const auto& child_inline_boxes = absl::get<TextBox>(child->box).glyph_run_boxes.inline_boxes;
					for (auto& ib : child_inline_boxes) {
						inline_boxes.push_back(ib.get());
					}
				} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(child->box)) {
					auto& child_inline_boxes = absl::get<std::vector<std::unique_ptr<InlineBox>>>(child->box);
					for (auto& ib : child_inline_boxes) {
						inline_boxes.push_back(ib.get());
					}
				} else if (absl::holds_alternative<InlineBlockBox>(child->box)) {
					auto& child_inline_box = absl::get<InlineBlockBox>(child->box).inline_box;
					inline_boxes.push_back(child_inline_box.get());
				}

				child = child->next_sibling;
			}

			std::vector<std::tuple<LineBox*, InlineBox*, InlineBox*>> merged_tuple;
			std::vector<std::unique_ptr<InlineBox>> merged_boxes;
			for (InlineBox* child : inline_boxes) {
				if (merged_boxes.empty() || merged_boxes.back()->line_box != child->line_box) {
					auto fm = graph2d::getFontMetrics(st.font_family.keyword_val.c_str(),
						st.font_size.pixelOrZero());
					float line_height = st.line_height.pixelOrZero();
					std::unique_ptr<InlineBox> ib = std::make_unique<InlineBox>();
					ib->pos = child->pos;
					ib->baseline = fm.baseline();
					ib->size.width = child->size.width;
					ib->size.height = fm.lineHeight();
					ib->line_box = child->line_box;
					ib->line_box->line_height = std::max(ib->line_box->line_height, line_height);
					merged_tuple.emplace_back(child->line_box, ib.get(), ib.get());
					merged_boxes.push_back(std::move(ib));
				} else {
					std::unique_ptr<InlineBox>& ib = merged_boxes.back();
					ib->size.width = std::max(child->pos.x + child->size.width,
						ib->pos.x + ib->size.width) - ib->pos.x;
					auto& tuple = merged_tuple.back();
					absl::get<2>(tuple) = ib.get();
				}
			}

			o->box.emplace<std::vector<std::unique_ptr<InlineBox>>>(std::move(merged_boxes));
			for (size_t i = 0; i < merged_boxes.size(); ++i) {
				auto ib = merged_boxes[i].get();
				auto [line, first_ib, last_ib] = merged_tuple[i];
				line->mergeInlineBox(o, ib, first_ib, last_ib);
			}

			LayoutObject::arrangePositionedChildren(o, viewport_size);
		}
	} else if (st.display == DisplayType::InlineBlock) {
		InlineBlockBox& ibb = absl::get<InlineBlockBox>(o->box);
		ibb.block_box.pos = ibb.inline_box->pos;
		/*
		LOG(INFO) << "translate " << ib.pos;
		LayoutObject* child = o->first_child;
		while (child) {
			translate(child, ib.pos);
			child = child->next_sibling;
		}
		*/
	}
}

void LayoutObject::translate(LayoutObject* o, scene2d::PointF offset)
{
	if (absl::holds_alternative<TextBox>(o->box)) {
		auto& tb = absl::get<TextBox>(o->box);
		for (auto& ib : tb.glyph_run_boxes.inline_boxes) {
			ib->pos += offset;
		}
	} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(o->box)) {
		auto& ibs = absl::get<std::vector<std::unique_ptr<InlineBox>>>(o->box);
		for (auto& ib : ibs) {
			ib->pos += offset;
		}
	} else if (absl::holds_alternative<BlockBox>(o->box)) {
		auto& b = absl::get<BlockBox>(o->box);
		b.pos += offset;
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		auto& ibb = absl::get<InlineBlockBox>(o->box);
		ibb.inline_box->pos += offset;
		ibb.block_box.pos += offset;
	}

	LayoutObject* child = o->first_child;
	while (child) {
		translate(child, offset);
		child = child->next_sibling;
	}
}

void LayoutObject::arrangeInlineBlock(LayoutObject* o, InlineFormatContext& ifc, const scene2d::DimensionF& viewport_size)
{
	CHECK(o->style->display == DisplayType::InlineBlock)
		<< "LayoutObject::arrangeInlineBlock: expect inline-block element";
	CHECK(o->bfc.has_value())
		<< "BUG: LayoutObject::arrangeInlineBlock: expect inner BFC";

	const Style& st = *o->style;
	BlockFormatContext& bfc = ifc.bfc();

	float contg_width = bfc.contg_width;
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

	BlockBox& b = absl::get<InlineBlockBox>(o->box).block_box;
	arrangeInlineBlockX(o, bfc, viewport_size, scroll_y);

	BlockFormatContext& inner_bfc = o->bfc.value();
	inner_bfc.contg_width = bfc.contg_width;
	inner_bfc.contg_height = bfc.contg_height;
	inner_bfc.border_bottom_edge = 0;
	inner_bfc.margin_bottom_edge = 0;
	arrangeBfcTop(o, inner_bfc, b);
	arrangeBfcChildren(o, inner_bfc, b, viewport_size);
	arrangeBfcBottom(o, inner_bfc, b);

	/*
	if (st.overflow_y == OverflowType::Visible) {
		bfc.border_bottom_edge = bfc.margin_bottom_edge = inner_bfc.margin_bottom_edge;
		bfc.max_border_bottom_edge = std::max(bfc.max_border_bottom_edge, inner_bfc.max_border_bottom_edge);
	} else {
		float box_content_bottom_edge = b.pos.y + b.contentRect().bottom;
		bfc.border_bottom_edge = bfc.margin_bottom_edge = std::max(bfc.margin_bottom_edge, box_content_bottom_edge);
		bfc.max_border_bottom_edge = std::max(bfc.max_border_bottom_edge, box_content_bottom_edge);
	}
	if (st.overflow_x == OverflowType::Visible) {
		bfc.max_border_right_edge = std::max(bfc.max_border_right_edge, inner_bfc.max_border_right_edge);
	} else {
		float box_margin_right_edge = b.pos.x + b.marginRect().right;
		bfc.max_border_right_edge = std::max(bfc.max_border_right_edge, box_margin_right_edge);
	}

	// Check overflow-x
	ScrollbarPolicy scroll_x = ScrollbarPolicy::Hidden;
	if (st.overflow_x == OverflowType::Scroll) {
		scroll_x = ScrollbarPolicy::Stable;
	}
	if (st.overflow_x == OverflowType::Auto && bfc.max_border_right_edge > b.pos.x + b.paddingRect().bottom) {
		scroll_x = ScrollbarPolicy::Stable;
	}
	if (scroll_x == ScrollbarPolicy::Stable) {
		b.inner_padding.bottom = SCROLLBAR_GUTTER_WIDTH;
	} else if (scroll_x == ScrollbarPolicy::StableBothEdges) {
		b.inner_padding.top = b.inner_padding.bottom = SCROLLBAR_GUTTER_WIDTH;
	}
	b.content.height = std::max(0.0f, b.content.height - b.inner_padding.top - b.inner_padding.bottom);

	// Update ScrollObject
	if (st.overflow_y != OverflowType::Visible || st.overflow_x != OverflowType::Visible) {
		const BlockFormatContext& inner_bfc = o->bfc.value();
		scene2d::RectF inner_rect = b.clientRect();
		float padding_left_edge = b.pos.x + b.paddingRect().left;
		float padding_top_edge = b.pos.y + b.paddingRect().top;
		float border_bottom_edge = std::max(inner_bfc.border_bottom_edge, inner_bfc.max_border_bottom_edge);
		if (!o->scroll_object.has_value()) {
			o->scroll_object.emplace();
			ScrollObject& sd = o->scroll_object.value();
			sd.content_size.width = std::max(inner_rect.size().width, inner_bfc.max_border_right_edge - padding_left_edge);
			sd.content_size.height = std::max(inner_rect.size().height, border_bottom_edge - padding_top_edge);
			sd.viewport_rect.right = inner_rect.size().width;
			sd.viewport_rect.bottom = inner_rect.size().height;
		} else {
			ScrollObject& sd = o->scroll_object.value();
			sd.content_size.width = std::max(inner_rect.size().width, inner_bfc.max_border_right_edge - padding_left_edge);
			sd.content_size.height = std::max(inner_rect.size().height, border_bottom_edge - padding_top_edge);
			sd.viewport_rect.left = std::max(0.0f, std::min(sd.viewport_rect.left,
				sd.content_size.width - inner_rect.size().width));
			sd.viewport_rect.top = std::max(0.0f, std::min(sd.viewport_rect.top,
				sd.content_size.height - inner_rect.size().height));
			sd.viewport_rect.right = sd.viewport_rect.left + inner_rect.size().width;
			sd.viewport_rect.bottom = sd.viewport_rect.top + inner_rect.size().height;
		}
		LOG(INFO)
			<< "scrollData contentSize " << o->scroll_object->content_size
			<< ", viewportRect " << o->scroll_object->viewport_rect;
	} else {
		o->scroll_object = absl::nullopt;
	}
	// bfc.max_border_right_edge = std::max(bfc.max_border_right_edge, box.pos.x + box.borderRect().right);
	// bfc.max_border_bottom_edge = std::max(bfc.max_border_bottom_edge, bfc.border_bottom_edge);
	*/

	InlineBlockBox& ibb = absl::get<InlineBlockBox>(o->box);
	InlineBox& ib = *ibb.inline_box;
	ib.line_box = line;
	ib.line_box->addInlineBox(o, &ib);
	ib.pos.x = inline_pos_x;
	ib.size = ibb.block_box.marginRect().size();
	if (st.overflow_y == OverflowType::Visible) {
		absl::optional<float> baseline = findFirstBaseline(o);
		ib.baseline = ibb.block_box.contentRect().top + baseline.value_or(ib.size.height);
		//LOG(INFO) << "findFirstBaseline for " << o->node->tag_ << ": " << ib.baseline;
	} else {
		ib.baseline = ib.size.height;
	}
	line->line_height = std::max(line->line_height, ib.size.height);

	LayoutObject::arrangePositionedChildren(o, viewport_size);
}

void LayoutObject::arrangeInlineBlockX(LayoutObject* o,
	BlockFormatContext& bfc,
	const scene2d::DimensionF& viewport_size,
	ScrollbarPolicy scroll_y)
{
	const Style& st = *o->style;
	CHECK(st.display == DisplayType::InlineBlock);

	float contg_width = bfc.contg_width;

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
		b.scrollbar_gutter.right = SCROLLBAR_GUTTER_WIDTH;
	} else if (scroll_y == ScrollbarPolicy::StableBothEdges) {
		b.scrollbar_gutter.left = SCROLLBAR_GUTTER_WIDTH;
		b.scrollbar_gutter.right = SCROLLBAR_GUTTER_WIDTH;
	}
	b.content.width = std::max(0.0f, b.content.width - b.scrollbar_gutter.left - b.scrollbar_gutter.right);

	// Compute height, top and bottom margins
	b.margin.top = try_resolve_to_px(st.margin_top, contg_width).value_or(0);
	b.border.top = try_resolve_to_px(st.border_top_width, contg_width).value_or(0);
	b.padding.top = try_resolve_to_px(st.padding_top, contg_width).value_or(0);

	b.padding.bottom = try_resolve_to_px(st.padding_bottom, contg_width).value_or(0);
	b.border.bottom = try_resolve_to_px(st.border_bottom_width, contg_width).value_or(0);
	b.margin.bottom = try_resolve_to_px(st.margin_bottom, contg_width).value_or(0);

	// No need to compute pos.x
	// b.pos.x = bfc.contg_left_edge;
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
	bfc.contg_width = box.contentRect().width();
	bfc.border_bottom_edge = bfc.margin_bottom_edge = box.margin.top + borpad_top;
	box.prefer_height = try_resolve_to_px(st.height, contg_height);
	bfc.contg_height = box.prefer_height;

	float saved_bfc_margin_bottom = bfc.margin_bottom_edge;
	if (o->flags & HAS_BLOCK_CHILD_FLAG) {
		{
			LayoutObject* child = o->first_child;
			while (child) {
				arrangeBlock(child, bfc, viewport_size);
				child = child->next_sibling;
			}
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
		o->ifc.emplace(o, bfc, box.contentRect().width());
		LOG(INFO)
			<< "begin IFC contg_width=" << bfc.contg_width
			<< ", bfc_bottom: border=" << bfc.border_bottom_edge << ", margin=" << bfc.margin_bottom_edge;

		{
			LayoutObject* child = o->first_child;
			while (child) {
				prepare(child, *o->ifc, viewport_size);
				child = child->next_sibling;
			}
			o->ifc->arrangeX(st.text_align);
			child = o->first_child;
			while (child) {
				arrange(child, *o->ifc, viewport_size);
				child = child->next_sibling;
			}
			o->ifc->arrangeY();
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

absl::optional<float> LayoutObject::findFirstBaseline(LayoutObject* o, float accum_y)
{
	if (o->ifc.has_value()) {
		for (const std::unique_ptr<LineBox>& line : o->ifc->lineBoxes()) {
			for (const InlineFragment& frag : line->inline_frags) {
				return frag.box->baseline + accum_y;
			}
		}
	}
	/*
	if (o->style->display == DisplayType::InlineBlock) {
		const auto& ibb = absl::get<InlineBlockBox>(o->box);
		if (!ibb.inline_boxes.empty())
			accum_y += ibb.inline_boxes.front().pos.y;
	}

	LayoutObject* child = o->first_child;
	while (child) {
		auto baseline = findFirstBaseline(child, accum_y);
		if (baseline.has_value())
			return baseline;
		child = child->next_sibling;
	}
	*/
	return absl::nullopt;
}

absl::optional<scene2d::RectF> LayoutObject::containingRectForPositionedChildren(LayoutObject* o)
{
	if (absl::holds_alternative<BlockBox>(o->box)) {
		return absl::get<BlockBox>(o->box).clientRect();
	} else if (absl::holds_alternative<std::vector<std::unique_ptr<InlineBox>>>(o->box)) {
		const auto& ibs = absl::get<std::vector<std::unique_ptr<InlineBox>>>(o->box);
		if (!ibs.empty()) {
			const InlineBox& first = *ibs.front();
			const InlineBox& last = *ibs.back();
			return scene2d::RectF::fromLTRB(
				std::min(first.pos.x, last.pos.x),
				std::min(first.pos.y, last.pos.y),
				std::max(first.pos.x + first.size.width, last.pos.x + last.size.width),
				std::max(first.pos.y + first.size.height, last.pos.y + last.size.height));
		}
	} else if (absl::holds_alternative<InlineBlockBox>(o->box)) {
		const auto& ibb = absl::get<InlineBlockBox>(o->box);
		return scene2d::RectF::fromOriginSize(ibb.inline_box->pos, ibb.inline_box->size);
	}
	return absl::nullopt;
}

void LayoutObject::removeFromParent()
{
	if (!parent)
		return;
	if (prev_sibling) {
		prev_sibling->next_sibling = next_sibling;
	} else {
		parent->first_child = next_sibling;
	}
	if (next_sibling) {
		next_sibling->prev_sibling = prev_sibling;
	} else {
		parent->last_child = prev_sibling;
	}
	prev_sibling = next_sibling = nullptr;
	parent = nullptr;
}

void LayoutObject::append(LayoutObject* child)
{
	DCHECK(!child->parent) << "LayoutObject::append child with exist parent.";
	DCHECK(!child->prev_sibling) << "LayoutObject::append child with exist prev-sibling.";
	DCHECK(!child->next_sibling) << "LayoutObject::append child with exist next-sibling.";

	child->parent = this;
	if (last_child) {
		last_child->next_sibling = child;
		child->prev_sibling = last_child;
		last_child = child;
	} else {
		first_child = last_child = child;
	}
}

void LayoutObject::insertBeforeMe(LayoutObject* o)
{
	DCHECK(parent) << "LayoutObject::insertBeforeMe self parent not exists.";
	DCHECK(!o->parent) << "LayoutObject::insertBeforeMe with exist parent.";
	DCHECK(!o->prev_sibling) << "LayoutObject::insertBeforeMe with exist prev-sibling.";
	DCHECK(!o->next_sibling) << "LayoutObject::insertBeforeMe with exist next-sibling.";

	o->parent = parent;
	if (prev_sibling) {
		prev_sibling->next_sibling = o;
		o->prev_sibling = prev_sibling;
	} else {
		parent->first_child = o;
	}
	prev_sibling = o;
	o->next_sibling = this;
}

void LayoutObject::insertAfterMe(LayoutObject* o)
{
	DCHECK(parent) << "LayoutObject::insertAfterMe self parent not exists.";
	DCHECK(!o->parent) << "LayoutObject::insertAfterMe with exist parent.";
	DCHECK(!o->prev_sibling) << "LayoutObject::insertAfterMe with exist prev-sibling.";
	DCHECK(!o->next_sibling) << "LayoutObject::insertAfterMe with exist next-sibling.";

	o->parent = parent;
	if (next_sibling) {
		next_sibling->prev_sibling = o;
		o->next_sibling = next_sibling;
	} else {
		parent->last_child = o;
	}
	next_sibling = o;
	o->prev_sibling = this;
}

void LayoutObject::dumpTree(int indent, std::ostringstream& stream)
{
	if (!node)
		return;
	std::string indent_str(indent, ' ');
	if (node->type() == scene2d::NodeType::NODE_ELEMENT) {
		if (flags & ANON_BLOCK_FLAG) {
			stream << indent_str << absl::StreamFormat("<anon_%s flag=%02x>\n", node->tag_.c_str(), flags);
		} else {
			stream << indent_str << absl::StreamFormat("<%s flag=%02x>\n", node->tag_.c_str(), flags);
		}
		for (auto child = first_child; child; child = child->next_sibling) {
			child->dumpTree(indent + 1, stream);
		}
		if (flags & ANON_BLOCK_FLAG) {
			stream << indent_str << absl::StreamFormat("</anon_%s>\n", node->tag_.c_str());
		} else {
			stream << indent_str << absl::StreamFormat("</%s>\n", node->tag_.c_str());
		}
	} else if (node->type() == scene2d::NodeType::NODE_TEXT) {
		if (flags & ANON_SPAN_FLAG) {
			stream << indent_str << absl::StreamFormat("<anon>\"%s\"</anon>\n", node->text_.c_str());
		} else {
			stream << indent_str << absl::StreamFormat("\"%s\"\n", node->text_.c_str());
		}
	} else {
		for (auto child = first_child; child; child = child->next_sibling) {
			child->dumpTree(indent, stream);
		}
	}
}

void LayoutObject::dumpTree()
{
	std::ostringstream stream;
	dumpTree(0, stream);
	LOG(INFO) << "DumpTree:\n" << stream.str();
}

void LayoutObject::arrangePositionedChildren(LayoutObject* o, const scene2d::DimensionF& viewport_size)
{
	for (LayoutObject* po : o->positioned_children) {
		if (po->style->position == PositionType::Absolute
			|| po->style->position == PositionType::Fixed) {
			reflow(FlowRoot{ po, o }, viewport_size);
		} else {
			CHECK(po->style->position == PositionType::Relative);
			auto& st = *po->style;
			scene2d::RectF crect = LayoutObject::contentRect(o);
			EdgeOffsetF off;
			if (st.left.isAuto() && st.right.isAuto()) {
				;
			} else if (st.right.isAuto()) {
				off.left = try_resolve_to_px(st.left, crect.width()).value_or(0.0f);
				off.right = -off.left;
			} else if (st.left.isAuto()) {
				off.right = try_resolve_to_px(st.right, crect.width()).value_or(0.0f);
				off.left = -off.right;
			} else {
				off.left = try_resolve_to_px(st.left, crect.width()).value_or(0.0f);
				off.right = -off.left;
			}
			if (st.top.isAuto() && st.bottom.isAuto()) {
				;
			} else if (st.bottom.isAuto()) {
				off.top = try_resolve_to_px(st.top, crect.width()).value_or(0.0f);
				off.bottom = -off.top;
			} else if (st.top.isAuto()) {
				off.bottom = try_resolve_to_px(st.bottom, crect.width()).value_or(0.0f);
				off.top = -off.bottom;
			} else {
				off.top = try_resolve_to_px(st.top, crect.width()).value_or(0.0f);
				off.bottom = -off.top;
			}
			LayoutObject::setPos(po, LayoutObject::pos(po) + scene2d::PointF(off.left, off.top));
		}
	}
}

LayoutTreeBuilder::LayoutTreeBuilder(scene2d::Node* node)
	: root_(node) {}
std::vector<FlowRoot> LayoutTreeBuilder::build()
{
	std::vector<FlowRoot> flow_roots;

	flow_roots.push_back({ &root_->layout_, nullptr });
	initFlowRoot(root_);
	flow_root_ = &flow_roots.back();
	scene2d::Node::eachLayoutChild(root_, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));
	//flow_root_->root->dumpTree();

	while (!abs_pos_nodes_.empty()) {
		auto nodes = std::move(abs_pos_nodes_);
		for (auto node : nodes) {
			auto pnode = node->positionedAncestor();
			auto po = pnode ? &pnode->layout_ : nullptr;
			flow_roots.push_back({ &node->layout_, po });
			flow_root_ = &flow_roots.back();
			initFlowRoot(node);
			scene2d::Node::eachLayoutChild(node, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));
		}
	}
	flow_root_ = nullptr;

	for (auto it = flow_roots.begin(); it != flow_roots.end();) {
		if (it->positioned_parent)
			it = flow_roots.erase(it);
		else
			++it;
	}
	return flow_roots;
}

void LayoutTreeBuilder::initFlowRoot(scene2d::Node* node)
{
	contg_ = &node->layout_;
	contg_->reset();
	contg_->flags |= LayoutObject::NEW_BFC_FLAG;
	contg_->bfc.emplace(node);
	contg_->box.emplace<BlockBox>();
}

static LayoutObject* make_phantom_span(LayoutObject* current)
{
	auto phantom_span = std::make_unique<LayoutObject>();
	phantom_span->init(current->style, current->node);
	phantom_span->flags = LayoutObject::PHANTOM_SPAN_FLAG;
	phantom_span->box.emplace<std::vector<std::unique_ptr<InlineBox>>>();
	current->aux_boxes.emplace_back(std::move(phantom_span));
	return current->aux_boxes.back().get();
}

static void split_up(LayoutObject* blk_contg_root, const std::deque<LayoutObject*>& inline_parent, LayoutObject* current)
{
	DCHECK(!inline_parent.empty()) << "LayoutObject split_up: empty inline parents.";
	/*
	LOG(INFO) << "split_up: block root: <" << blk_contg_root->node->elementTag() << ">";
	for (LayoutObject* ip : inline_parent) {
		LOG(INFO) << "\tinline parent: <" << ip->node->elementTag() << ">";
	}
	LOG(INFO) << "\tcurrent: <" << current->node->elementTag() << ">";
	*/
	// bottom-up span split
	std::deque<LayoutObject*> phantom_boxes;
	LayoutObject* o = current;
	LayoutObject* prev_span = nullptr;
	for (auto it = inline_parent.rbegin(); it != inline_parent.rend(); ++it) {
		LayoutObject* to_split = *it;
		LayoutObject* span = make_phantom_span(to_split);
		if (prev_span) {
			span->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
			span->append(prev_span);
		}
		while (o->next_sibling) {
			LayoutObject* next = o->next_sibling;
			next->removeFromParent();
			span->append(next);
		}
		phantom_boxes.push_front(span);
		prev_span = span;
		o = to_split;
	}

	// insert current block
	current->removeFromParent();
	inline_parent.front()->insertAfterMe(current);

	// insert first phantom span
	current->insertAfterMe(phantom_boxes.front());
}

void LayoutTreeBuilder::prepareChild(scene2d::Node* node)
{
	if (node->type() == scene2d::NodeType::NODE_TEXT) {
		addText(node);
	} else if (node->type() == scene2d::NodeType::NODE_ELEMENT) {
		if (node->positioned()) {
			scene2d::Node* pn = node->absolutelyPositioned()
				? node->positionedAncestor()
				: node->parent();
			if (pn) {
				pn->layout_.positioned_children.push_back(&node->layout_);
			}
			if (node->absolutelyPositioned()) {
				abs_pos_nodes_.push_back(node);
				return;
			}
		}

		//////////////////////////////////////
		//  'static' or 'relative' positioned
		const auto& st = node->computedStyle();
		if (st.display == style::DisplayType::Block) {
			if (contg_->style->display == style::DisplayType::Block || contg_->style->display == style::DisplayType::InlineBlock) {
				contg_->flags |= LayoutObject::HAS_BLOCK_CHILD_FLAG;

				beginChild(&node->layout_);
				contg_->box.emplace<BlockBox>();
			} else if (contg_->style->display == style::DisplayType::Inline) {
				// update bubble block map
				auto p = contg_->parent;
				while (p && p->style->display == style::DisplayType::Inline)
					p = p->parent;
				if (p)
					bbmap_[p].push_back(&node->layout_);

				beginChild(&node->layout_);
				contg_->box.emplace<BlockBox>();
			}
		} else if (st.display == style::DisplayType::Inline) {
			contg_->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
			beginChild(&node->layout_);
			contg_->box.emplace<std::vector<std::unique_ptr<InlineBox>>>();
		} else if (st.display == style::DisplayType::InlineBlock) {
			contg_->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
			beginChild(&node->layout_);
			contg_->box.emplace<InlineBlockBox>();
		}

		// establish new BFC
		if (st.display == style::DisplayType::InlineBlock
			|| st.overflow_x != style::OverflowType::Visible
			|| st.overflow_y != style::OverflowType::Visible) {

			contg_->flags |= LayoutObject::NEW_BFC_FLAG;
			contg_->bfc.emplace(node);
		}
		scene2d::Node::eachLayoutChild(node, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));
		endChild();
		if (contg_ && contg_->flags & LayoutObject::ANON_SPAN_FLAG)
			endChild();
	}
}

void LayoutTreeBuilder::addText(scene2d::Node* node)
{
	TextBox tb;
	tb.text_flow = node->text_flow_.get();
	
	node->layout_.reset();
	node->layout_.box.emplace<TextBox>(std::move(tb));

	if (contg_->style->display == DisplayType::Block
			|| contg_->style->display == DisplayType::InlineBlock) {
		node->layout_.aux_boxes.emplace_back(std::make_unique<LayoutObject>());
		auto anon = node->layout_.aux_boxes.back().get();
		anon->init(&node->computed_style_, node);
		anon->flags = LayoutObject::HAS_INLINE_CHILD_FLAG | LayoutObject::ANON_SPAN_FLAG;
		anon->box.emplace<std::vector<std::unique_ptr<InlineBox>>>();

		contg_->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
		contg_->append(anon);
		anon->append(&node->layout_);
	} else {
		contg_->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
		contg_->append(&node->layout_);
	}
}

void LayoutTreeBuilder::beginChild(LayoutObject* o)
{
	//if (current_->type != BlockBoxType::Empty && current_->type != BlockBoxType::WithBlockChildren) {
	//	LOG(WARNING) << "begin Block in containing block " << current_->type << " not implemented.";
	//}
	o->reset();

	contg_->append(o);
	stack_.push_back(contg_);

	contg_ = o;
}

static LayoutObject* make_anon_block(LayoutObject* current, const std::pair<LayoutObject*, LayoutObject*>& inline_pair)
{
	auto anon_block = std::make_unique<LayoutObject>();
	anon_block->anon_style = std::make_unique<Style>();
	anon_block->anon_style->display = DisplayType::Block;
	anon_block->anon_style->position = PositionType::Static;
	anon_block->anon_style->resolveDefault(current->style);
	anon_block->init(anon_block->anon_style.get(), current->node);
	anon_block->flags = (LayoutObject::HAS_INLINE_CHILD_FLAG | LayoutObject::ANON_BLOCK_FLAG);
	anon_block->box.emplace<BlockBox>();
	auto nc = inline_pair.first;
	while (nc) {
		auto nc_next = nc->next_sibling;

		nc->removeFromParent();
		anon_block->append(nc);

		if (nc == inline_pair.second)
			break;
		nc = nc_next;
	}
	current->aux_boxes.emplace_back(std::move(anon_block));
	return current->aux_boxes.back().get();
}

void LayoutTreeBuilder::endChild()
{
	// is block container and contains both inline and block children
	if (contg_->style->display == style::DisplayType::Block || contg_->style->display == style::DisplayType::InlineBlock) {
		auto it = bbmap_.find(contg_);
		if (it != bbmap_.end()) {
			for (auto bit = it->second.rbegin(); bit != it->second.rend(); ++bit)
				bubbleUp(*bit);
			bbmap_.erase(it);
		}

		if ((contg_->flags & LayoutObject::HAS_BLOCK_CHILD_FLAG) != 0
			&& (contg_->flags & LayoutObject::HAS_INLINE_CHILD_FLAG) != 0) {
			contg_->flags &= ~LayoutObject::HAS_INLINE_CHILD_FLAG;
			// generate anonymous block box
			absl::optional<std::pair<LayoutObject*, LayoutObject*>> inline_pair;
			for (auto child = contg_->first_child; child; child = child->next_sibling) {
				if (child->style->display == DisplayType::Inline || child->style->display == DisplayType::InlineBlock) {
					if (inline_pair.has_value()) {
						inline_pair.value().second = child;
					} else {
						inline_pair.emplace(child, child);
					}
				} else { // is block container
					if (inline_pair.has_value()) {
						auto anon_block = make_anon_block(contg_, inline_pair.value());
						child->insertBeforeMe(anon_block);
						inline_pair = absl::nullopt;
					}
				}
			}
			if (inline_pair.has_value()) {
				auto anon_block = make_anon_block(contg_, inline_pair.value());
				contg_->append(anon_block);
				inline_pair = absl::nullopt;
			}
		}
	}

	contg_ = stack_.back();
	stack_.pop_back();
}

void LayoutTreeBuilder::bubbleUp(LayoutObject* blk)
{
	// tracking [block_parent, inline_parent1, ..., inline_parent2, block_current];
	LayoutObject* block_contg_parent = nullptr;
	std::deque<LayoutObject*> inline_parents;
	LayoutObject* o = blk->parent;
	while (o) {
		if (o->style->display == style::DisplayType::Block || o->style->display == style::DisplayType::InlineBlock) {
			block_contg_parent = o;
			break;
		} else if (o->style->display == style::DisplayType::Inline) {
			inline_parents.push_front(o);
		}
		o = o->parent;
	}

	if (!block_contg_parent) {
		LOG(ERROR) << "LayoutTreeBuilder::bubbleUp: block container root not found.";
		return;
	}

	// split up
	split_up(block_contg_parent, inline_parents, blk);

	// special beginChild()
	block_contg_parent->flags |= LayoutObject::HAS_BLOCK_CHILD_FLAG;
}

}
