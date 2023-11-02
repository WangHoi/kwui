#include "LayoutObject.h"
#include "scene2d/Node.h"
#include "absl/functional/bind_front.h"
#include "base/log.h"
#include <algorithm>

namespace style {

void LayoutObject::reset()
{
	LOG(WARNING) << "TODO: implement LayoutObject::reset()";
	flags = 0;
	min_width = 0.0f;
	max_width = std::numeric_limits<float>::infinity();
	prefer_height = absl::nullopt;
}

void LayoutObject::reflow(FlowRoot fl, const scene2d::DimensionF& viewport_size)
{
	CHECK(fl.root->flags & LayoutObject::NEW_BFC_FLAG)
		<< "LayoutObject::reflow: root expect NEW_BFC_FLAG";
	CHECK(absl::holds_alternative<BlockBox>(fl.root->box))
		<< "LayoutObject::reflow: root must be block-level";
	
	const Style& st = *fl.root->style;
	BlockBox& b = absl::get<BlockBox>(fl.root->box);

	// 1. Measure min(max)-content width
	measure(fl.root, viewport_size.height);

	// 2. Arrange
	BlockFormatContext bfc(nullptr);
	bfc.contg_left = 0.0f;
	bfc.contg_right = viewport_size.width;
	arrange(fl.root, bfc);

	if (st.position == PositionType::Static) {

	} else {
		LOG(WARNING) << "TODO: handle positioned";
	}
}

void LayoutObject::measure(LayoutObject* o, float viewport_height)
{
	const style::Style& st = *o->style;
	absl::optional<float> parent_height = o->parent
		? o->parent->prefer_height : absl::make_optional(viewport_height);
	o->prefer_height = try_resolve_to_px(st.height, parent_height);

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

void LayoutObject::arrange(LayoutObject* o, BlockFormatContext& bfc)
{
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
	// TODO: add reset method
	node->layout_.reset();
	node->layout_.flags |= LayoutObject::NEW_BFC_FLAG;
	node->layout_.bfc.emplace(node);

	node->inline_box_ = style::InlineBox();
	node->block_box_ = style::BlockBox(&node->computed_style_);
	node->bfc_.emplace(node);
	node->ifc_ = absl::nullopt;
	current_ = &node->layout_;
	current_->reset();
	last_child_ = nullptr;
	stack_.clear();
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
	}
	last_child_ = o;
	//if (current_->type == BlockBoxType::Empty) {
	//	current_->type = BlockBoxType::WithBlockChildren;
	//	current_->payload = o;
	//}
	stack_.push_back(std::make_tuple(current_, last_child_));

	current_ = o;
	last_child_ = nullptr;
}

void LayoutTreeBuilder::endChild()
{
	if (reparent_to_anon_block_pending_) {
		reparent_to_anon_block_pending_ = false;
		LOG(WARNING) << "TODO: handle anon_block";
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
