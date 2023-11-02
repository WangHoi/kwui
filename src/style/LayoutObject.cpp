#include "LayoutObject.h"
#include "scene2d/Node.h"
#include "absl/functional/bind_front.h"
#include "base/log.h"

namespace style {

void LayoutObject::reset()
{
	LOG(WARNING) << "TODO: implement LayoutObject::reset()";
	flags = 0;
}

void LayoutObject::reflow(LayoutObject* o, const scene2d::DimensionF& viewport_size)
{
	CHECK(o->flags & LayoutObject::NEW_BFC_FLAG)
		<< "LayoutObject::reflow: root expect NEW_BFC_FLAG";
	CHECK(absl::holds_alternative<BlockBox>(o->box))
		<< "LayoutObject::reflow: root must be block-level";
	
	BlockFormatContext& bfc = o->bfc.value();
	const Style& st = *o->style;
	BlockBox& b = absl::get<BlockBox>(o->box);

	// 1. Measure min(max)-content width

	// 2. Horizontal arrage

	// 3. Vertical arrage

	if (st.position == PositionType::Static) {

	} else {
		LOG(WARNING) << "TODO: handle positioned";
	}
}

LayoutTreeBuilder::LayoutTreeBuilder(scene2d::Node* node)
	: root_(node) {}
std::vector<LayoutObject*> LayoutTreeBuilder::build()
{
	std::vector<LayoutObject*> flow_roots;
	
	flow_roots.push_back(&root_->layout_);
	initFlowRoot(root_);
	scene2d::Node::eachChild(root_, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));
	
	while (!abs_pos_nodes_.empty()) {
		auto nodes = std::move(abs_pos_nodes_);
		for (auto node : nodes) {
			flow_roots.push_back(&node->layout_);
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
