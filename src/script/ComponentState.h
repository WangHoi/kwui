#pragma once
#include "quickjs.h"

namespace script {
class ComponentState {
public:
	// register class to global object
	static void registerClass(JSContext* ctx);
	static JSValue newObject(JSContext* ctx, int argc, JSValueConst* argv);
};

}