module;
#include <string>
#include <memory>
#include <vector>
export module scene2d:Actor;

import base;

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

protected:
	Stage* stage_ = nullptr;

	Actor* parent_ = nullptr;
	Actor* next_sibling_;
	Actor* prev_sibling_;
	base::object_refptr<Actor> children_;

	ActorType type_;

	std::string text_;
	
	base::string_atom tag_;
};

}
