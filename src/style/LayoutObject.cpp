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

LayoutTreeBuilder::LayoutTreeBuilder(scene2d::Node* node)
	: root_(node) {}
std::vector<LayoutObject*> LayoutTreeBuilder::build()
{
	std::vector<LayoutObject*> flow_roots;
	
	initFlowRoot(root_);
	
	scene2d::Node::eachChild(root_, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));
	
	return flow_roots;
}

void LayoutTreeBuilder::initFlowRoot(scene2d::Node* node)
{
	// TODO: add reset method
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
			|| st.overflow_y != style::OverflowType::Visible) {
			current_->flags |= LayoutObject::NEW_BFC_FLAG;
			current_->bfc = absl::make_optional<BlockFormatContext>(node);
		}
	}
	scene2d::Node::eachChild(node, absl::bind_front(&LayoutTreeBuilder::prepareChild, this));

	endChild();
}

void LayoutTreeBuilder::addText(scene2d::Node* node)
{
	/*
	if (current_->type == BlockBoxType::Empty) {
		current_->type = BlockBoxType::WithInlineChildren;
		current_->payload = node->parent();
	}
	*/
}

void LayoutTreeBuilder::beginChild(LayoutObject* o)
{
	//if (current_->type != BlockBoxType::Empty && current_->type != BlockBoxType::WithBlockChildren) {
	//	LOG(WARNING) << "begin Block in containing block " << current_->type << " not implemented.";
	//}
	o->reset();

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
	// TODO: handle anon_block
	std::tie(current_, last_child_) = stack_.back();
	stack_.pop_back();
}

void LayoutTreeBuilder::parentAddBlockChild()
{
	if (current_->parent->flags & LayoutObject::HAS_INLINE_CHILD_FLAG) {
		LOG(WARNING) << "add block-level child to inline parent, fallback to inline-block";
		current_->box = InlineBox();
		return;
	}

	current_->parent->flags |= LayoutObject::HAS_BLOCK_CHILD_FLAG;
	current_->box = BlockBox(nullptr);
}

void LayoutTreeBuilder::parentAddInlineChild()
{
	if (current_->parent->flags & LayoutObject::HAS_BLOCK_CHILD_FLAG) {
		current_->anon_block = std::make_unique<LayoutObject>();
		current_->box = InlineBox();
	} else {
		current_->parent->flags |= LayoutObject::HAS_INLINE_CHILD_FLAG;
		current_->box = InlineBox();
	}
}

}
