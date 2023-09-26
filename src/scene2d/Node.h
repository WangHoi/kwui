#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>

#include "base/base.h"
#include "style/Style.h"
#include "script/script.h"

namespace scene2d {

enum class ActorType {
	ACTOR_TEXT = 1,
	ACTOR_ELEMENT = 2,
};

class Scene;
class Node : public base::Object {
public:
	Node(ActorType type)
		: next_sibling_(this)
		, prev_sibling_(this)
		, type_(type)
	{}
	Node(ActorType type, const std::string& text)
		: Node(type)
	{
		text_ = text;
	}
	Node(ActorType type, base::string_atom tag)
		: Node(type)
	{
		tag_ = tag;
	}

protected:
#pragma region Tree nodes
	Scene* stage_ = nullptr;

	Node* parent_ = nullptr;
	Node* next_sibling_;	// double linked
	Node* prev_sibling_;
	Node* first_child_ = nullptr;

	ActorType type_;

	// Text
	std::string text_;

	// Element
	base::string_atom tag_;

	// Component
	JSValue comp_state_ = JS_UNINITIALIZED;

#pragma endregion

#pragma region Style and layout
	style::StyleSpec specStyle_;
	style::Style computedStyle_;
#pragma endregion
	
	friend class Scene;
};

}
