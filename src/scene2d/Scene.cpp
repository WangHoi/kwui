#include "Scene.h"
#include "Node.h"

namespace scene2d {

Scene::Scene()
{
	root_ = createElementNode(base::string_intern("kml"));
	root_->retain();
	weakptr_ = new base::WeakObject<Scene>(this);
	weakptr_->retain();
}

Scene::~Scene()
{
	weakptr_->clear();
	weakptr_->release();
	if (root_) {
		root_->release();
		root_ = nullptr;
	}
}

Node* Scene::createTextNode(const std::string& text)
{
	auto actor = new Node(ActorType::ACTOR_TEXT, text);
	actor->retain();
	return actor;
}

Node* Scene::createElementNode(base::string_atom tag)
{
	auto actor = new Node(ActorType::ACTOR_ELEMENT, tag);
	actor->retain();
	return actor;
}

void Scene::updateComponent(JSValue comp_state)
{

}

}