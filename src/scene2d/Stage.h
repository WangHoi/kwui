#pragma once

#include "quickjs.h"
#include <string>

#include "base/base.h"
#include "Actor.h"

namespace scene2d {

class Stage : public base::Object {
public:
	Stage();
	~Stage();
	Actor* createTextActor(const std::string &text);
	Actor* createElementActor(base::string_atom tag);

	void setup(JSValue actorData, JSValue styleData);
private:
	Actor* root_;
};
}
