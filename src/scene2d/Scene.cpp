#include "Scene.h"
#include "Node.h"

namespace scene2d {

Scene::Scene()
{
	root_ = createElementActor(base::string_intern("kml"));
	root_->retain();
}

Scene::~Scene()
{
	if (root_) {
		root_->release();
		root_ = nullptr;
	}
}

Node* Scene::createTextActor(const std::string& text)
{
	auto actor = new Node(ActorType::ACTOR_TEXT, text);
	actor->retain();
	return actor;
}

Node* Scene::createElementActor(base::string_atom tag)
{
	auto actor = new Node(ActorType::ACTOR_ELEMENT, tag);
	actor->retain();
	return actor;
}

void Scene::setup(JSValue actorData, JSValue styleData)
{

}

}