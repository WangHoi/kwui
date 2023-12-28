#pragma once
#include "api/kwui/ScriptValue.h"
#include "script.h"
#include <vector>

namespace script {

class EventPort {
public:
	static void postFromNative(const std::string& event, const kwui::ScriptValue& value);
	static void addListenerFromNative(const std::string& event, kwui::ScriptFunction* func, void* udata);
	static bool removeListenerFromNative(const std::string& event, kwui::ScriptFunction* func, void* udata);

	static JSValue postFromScript(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
	static JSValue addListenerFromScript(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);
	static JSValue removeListenerFromScript(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv);

	static void setupAppObject(JSContext* ctx, JSValue app);

private:
	static void doPost(const std::string& event, const kwui::ScriptValue& value);
};

}