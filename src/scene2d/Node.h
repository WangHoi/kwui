#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>

#include "base/base.h"
#include "style/Style.h"
#include "script/script.h"

namespace scene2d {

enum class NodeType {
	NODE_TEXT = 1,
	NODE_ELEMENT = 2,
	NODE_COMPONENT = 3,
};

class Scene;
class Node : public base::Object {
public:
	Node(NodeType type);
	Node(NodeType type, const std::string& text);
	Node(NodeType type, base::string_atom tag);
	Node(NodeType type, JSValue comp_state);
	~Node();
	
	void appendChild(Node *child);

	inline base::WeakObject<Node> *weakObject() const
	{
		return weakptr_;
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
	base::WeakObject<Node> *weakptr_ = nullptr;

#pragma endregion

#pragma region Style and layout
	style::StyleSpec specStyle_;
	style::Style computedStyle_;
#pragma endregion
	
	friend class Scene;
};

}
