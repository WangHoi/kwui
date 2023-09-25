#include "Stage.h"

namespace scene2d {


Stage::Stage()
{
	root_ = createElementActor(base::string_intern("kml"));
	root_->retain();
}

Stage::~Stage()
{
	if (root_) {
		root_->release();
		root_ = nullptr;
	}
}

Actor* Stage::createTextActor(const std::string& text)
{
	auto actor = new Actor(ActorType::ACTOR_TEXT, text);
	actor->retain();
	return actor;
}

Actor* Stage::createElementActor(base::string_atom tag)
{
	auto actor = new Actor(ActorType::ACTOR_ELEMENT, tag);
	actor->retain();
	return actor;
}

void Stage::setup(JSValue actorData, JSValue styleData)
{

}

}