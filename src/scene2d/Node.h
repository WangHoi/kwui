#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>

#include "base/base.h"
#include "style/Style.h"
#include "script/script.h"
#include "geom_types.h"

namespace windows {
class Dialog;
}

namespace scene2d {

enum class NodeType {
	NODE_TEXT = 1,
	NODE_ELEMENT = 2,
	NODE_COMPONENT = 3,
};

enum NodeFlag {
	NODE_FLAG_FOCUSABLE = 1,
	NODE_FLAG_HOVERABLE = 2,
	NODE_FLAG_CLICKABLE = 4,
};

class Scene;
class Node : public base::Object {
public:
	Node(NodeType type);
	Node(NodeType type, const std::string& text);
	Node(NodeType type, base::string_atom tag);
	Node(NodeType type, JSValue comp_state);
	~Node();

	void appendChild(Node* child);
	inline const std::vector<Node*> children() const
	{
		return children_;
	}

	inline base::WeakObjectProxy<Node>* weakObject() const
	{
		return weakptr_;
	}

	inline const PointF& origin() const
	{
		return origin_;
	}

	inline const DimensionF& size() const
	{
		return size_;
	}

	inline bool visible() const
	{
		return visible_;
	}

	bool testFlags(int flags) const;

	inline bool hitTestNode(const PointF& p) {
		return origin_.x <= p.x && p.x < origin_.x + size_.width
			&& origin_.y <= p.y && p.y < origin_.y + size_.height;
	}


protected:
#pragma region Tree nodes
	Scene* stage_ = nullptr;

	Node* parent_ = nullptr;
	int child_index = -1;
	std::vector<Node*> children_;

	NodeType type_;

	// Text
	std::string text_;

	// Element
	base::string_atom tag_;

	// Component
	JSValue comp_state_ = JS_UNINITIALIZED;
	base::WeakObjectProxy<Node>* weakptr_ = nullptr;

#pragma endregion

#pragma region Style and layout
	style::StyleSpec specStyle_;
	style::Style computedStyle_;

	PointF origin_;
	DimensionF size_;
	bool visible_ = true;
#pragma endregion

	friend class Scene;
	friend class windows::Dialog;
};

}
