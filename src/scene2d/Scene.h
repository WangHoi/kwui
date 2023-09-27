#pragma once

#include <string>
#include <memory>
#include "base/base.h"
#include "quickjs.h"

namespace script {
class Context;
}

namespace scene2d {

class Node;
class Scene : public base::Object {
public:
	Scene();
	~Scene();
	inline base::WeakObject<Scene> *weakObject() const
	{
		return weakptr_;
	}
	Node* createTextNode(const std::string &text);
	Node* createElementNode(base::string_atom tag);
	Node* createComponentNode(JSValue comp_data);
	void updateComponent(JSValue comp_state);

private:
	Node* createComponentNodeWithState(JSValue comp_data);

	std::unique_ptr<script::Context> ctx_;
	Node* root_;
	base::WeakObject<Scene> *weakptr_;
};
}
