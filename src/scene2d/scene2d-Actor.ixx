export module scene2d:Actor;

import std.memory;

namespace scene2d {

export enum ActorType {
	ACTOR_TEXT = 1,
	ACTOR_ELEMENT = 2,
};

export class Actor : public std::enable_shared_from_this<Actor> {
public:
	void addChild() {}
};

}
