#include "ContextId.h"
#include "absl/random/random.h"
#include <atomic>

namespace script {

std::atomic_size_t g_next_thread_id = 1;

JSClassID ContextId::JS_CLASS_ID = 0;
JSClassDef ContextId::JS_CLASS_DEF = {
	"__ContextId",
	&ContextId::finalize,
};

void ContextId::registerClass(JSContext* ctx)
{
	JSRuntime* rt = JS_GetRuntime(ctx);
	JS_NewClassID(&JS_CLASS_ID);
	if (!JS_IsRegisteredClass(rt, JS_CLASS_ID))
		JS_NewClass(rt, JS_CLASS_ID, &JS_CLASS_DEF);
}

JSValue ContextId::create(JSContext* ctx)
{
	JSValue val = JS_NewObjectClass(ctx, JS_CLASS_ID);
	JS_SetOpaque(val, new ContextId);
	return val;
}

ContextId::ContextId()
{
	id_ = g_next_thread_id.fetch_add(1);
}

void ContextId::finalize(JSRuntime* rt, JSValue val)
{
	auto me = (ContextId*)JS_GetOpaque(val, JS_CLASS_ID);
	delete me;
}

}