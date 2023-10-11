#pragma once
#include "scene2d/geom_types.h"
#include <vector>

namespace scene2d {
class Node;
}

namespace style {

class BlockPaintContext {
public:
	inline const scene2d::PointF& basePos() const
	{
		return base_pos_;
	}
	inline const scene2d::PointF& positionedBasePos() const
	{
		return positioned_base_pos_;
	}
	// push 'BFC' or relative offset
	void push(scene2d::Node* node, const scene2d::PointF& pos);
	void pop();
	// push 'absolute'
	void pushAbsolute(scene2d::Node* node, const scene2d::PointF& pos);
	void popAbsolute();

private:
	struct Item {
		scene2d::Node* parent;
		scene2d::PointF saved_base_pos;
	};
	std::vector<Item> node_stack_;
	std::vector<Item> positioned_node_stack_;
	scene2d::PointF base_pos_;
	scene2d::PointF positioned_base_pos_;
};

}
