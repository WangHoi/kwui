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
	Node* createTextActor(const std::string &text);
	Node* createElementActor(base::string_atom tag);

	void setup(JSValue actorData, JSValue styleData);
private:
	Node* root_;
};
}
