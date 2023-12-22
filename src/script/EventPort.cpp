#include "EventPort.h"

namespace script {

static JSClassID js_event_port_clsid = 0;
static JSClassDef js_event_port_clsdef;
static const char* js_event_port_clsname = "__EventPort";

void js_add_event_port(JSContext* j)
{
	JSValue global = JS_GetGlobalObject(j);

	JS_NewClassID(&js_event_port_clsid);
	JS_NewClass(JS_GetRuntime(j), js_event_port_clsid, &js_event_port_clsdef);
	JSValue jobj = JS_NewObjectClass(j, js_event_port_clsid);

	JS_SetPropertyStr(j, global, js_event_port_clsname, jobj);

	JS_FreeValue(j, global);
}

}
