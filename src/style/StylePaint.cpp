#include "StylePaint.h"
#include "base/log.h"

namespace style {

void BlockPaintContext::push(scene2d::Node* node, const scene2d::PointF& pos)
{
	node_stack_.push_back({ node, base_pos_ });
	base_pos_ += pos;
}

void BlockPaintContext::pop()
{
	CHECK(!node_stack_.empty());
	if (!node_stack_.empty()) {
		base_pos_ = node_stack_.back().saved_base_pos;
		node_stack_.pop_back();
	}
}

void BlockPaintContext::pushAbsolute(scene2d::Node* node, const scene2d::PointF& pos)
{
	positioned_node_stack_.push_back({ node, positioned_base_pos_ });
	positioned_base_pos_ += pos;
	node_stack_.push_back({ node, base_pos_ });
	base_pos_ = positioned_base_pos_;
}

void BlockPaintContext::popAbsolute()
{
	CHECK(!positioned_node_stack_.empty() && !node_stack_.empty());
	if (!positioned_node_stack_.empty()) {
		positioned_base_pos_ -= positioned_node_stack_.back().saved_base_pos;
		positioned_node_stack_.pop_back();
	}
	if (!node_stack_.empty()) {
		base_pos_ = node_stack_.back().saved_base_pos;
		node_stack_.pop_back();
	}
}

}