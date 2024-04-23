#pragma once
#include "scene2d/Control.h"

namespace script {

class DialogInterface : public scene2d::EventContext {
public:
	virtual ~DialogInterface() {}

	virtual scene2d::Scene* GetScene() const = 0;
};

}
