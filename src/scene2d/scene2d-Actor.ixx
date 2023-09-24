module;
#include <string>
#include <memory>
#include <vector>
#include <set>
export module scene2d:Actor;

import base;
import style;

namespace scene2d {

export enum class ActorType {
	ACTOR_TEXT = 1,
	ACTOR_ELEMENT = 2,
};

class Stage;
export class Actor : public base::Object {
public:
	Actor(ActorType type)
		: next_sibling_(this)
		, prev_sibling_(this)
		, type_(type)
	{}
	Actor(ActorType type, const std::string& text)
		: Actor(type)
	{
		text_ = text;
	}
	Actor(ActorType type, base::string_atom tag)
		: Actor(type)
	{
		tag_ = tag;
	}

protected:
#pragma region AOM
	Stage* stage_ = nullptr;

	Actor* parent_ = nullptr;
	Actor* next_sibling_;	// double linked
	Actor* prev_sibling_;
	Actor* first_child_ = nullptr;

	ActorType type_;

	// Text
	std::string text_;

	// Element
	base::string_atom tag_;
#pragma endregion

#pragma region Style and layout
	style::StyleSpec specStyle_;
	style::Style computedStyle_;
#pragma endregion
	
	friend class Stage;
};

}
