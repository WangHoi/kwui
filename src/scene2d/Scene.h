#pragma once

#include "quickjs.h"
#include <string>

#include "base/base.h"

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
	void updateComponent(JSValue comp_state);

private:
	Node* root_;
	base::WeakObject<Scene> *weakptr_;
};
}
