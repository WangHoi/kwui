#include "ComponentState.h"
#include "scene2d/Node.h"
#include "scene2d/Scene.h"
#include "absl/base/macros.h"
#include "absl/log/log.h"
#include "absl/cleanup/cleanup.h"

namespace script {

JSClassID ComponentState::JS_CLASS_ID = 0;

static void component_state_finalizer(JSRuntime* rt, JSValue val)
{
	auto ptr = (ComponentState*)JS_GetOpaque(val, ComponentState::JS_CLASS_ID);
	if (ptr) {
		ptr->cleanup(rt);
		delete ptr;
	}
	LOG(INFO) << "__ComponentState finalizer called";
}

// 	constructor(renderFn, props, children)
static JSValue component_state_ctor(JSContext* ctx,
	JSValueConst new_target,
	int argc, JSValueConst* argv)
{
	JSValue obj = JS_UNDEFINED;
	JSValue proto;

	if (!JS_IsFunction(ctx, argv[0]))
		goto fail;
	if (!JS_IsObject(argv[1]))
		goto fail;
	if (!JS_IsArray(ctx, argv[2]))
		goto fail;
	/* using new_target to get the prototype is necessary when the
	   class is extended. */
	proto = JS_GetPropertyStr(ctx, new_target, "prototype");
	if (JS_IsException(proto))
		goto fail;
	obj = JS_NewObjectProtoClass(ctx, proto, ComponentState::JS_CLASS_ID);
	JS_FreeValue(ctx, proto);
	if (JS_IsException(obj))
		goto fail;
	JS_SetPropertyStr(ctx, obj, "renderFn", JS_DupValue(ctx, argv[0]));
	JS_SetPropertyStr(ctx, obj, "props", JS_DupValue(ctx, argv[1]));
	JS_SetPropertyStr(ctx, obj, "children", JS_DupValue(ctx, argv[2]));
	JS_SetOpaque(obj, new ComponentState);
	return obj;
fail:
	JS_FreeValue(ctx, obj);
	return JS_EXCEPTION;
}

// TODO: move into class ComponentState
JSValue component_state_render(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
	auto me = (ComponentState*)JS_GetOpaque(this_val, ComponentState::JS_CLASS_ID);
	if (me) {
		me->curr_slot_ = 0;
	}

	JSValue global = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global, "__comp_state", JS_DupValue(ctx, this_val));

	JSValue render_fn, args[2];
	render_fn = JS_GetPropertyStr(ctx, this_val, "renderFn");
	args[0] = JS_GetPropertyStr(ctx, this_val, "props");
	args[1] = JS_GetPropertyStr(ctx, this_val, "children");
	absl::Cleanup _ = [&]() {
		JS_FreeValue(ctx, render_fn);
		JS_FreeValue(ctx, args[0]);
		JS_FreeValue(ctx, args[1]);
		};
	JSValue ret = JS_Call(ctx, render_fn, this_val, 2, args);
	
	JS_SetPropertyStr(ctx, global, "__comp_state", JS_UNDEFINED);
	JS_FreeValue(ctx, global);
	return ret;
}

static JSClassDef g_component_state_class = {
	"__ComponentState",
	component_state_finalizer, // finalizer
	&ComponentState::gcMark,
};

static const JSCFunctionListEntry g_component_state_funcs[1] = {
	js_cfunc_def("render", 0, &component_state_render),
};

void ComponentState::registerClass(JSContext* ctx)
{
	JSValue proto, klass;

	/* create the Point class */
	JS_NewClassID(&JS_CLASS_ID);
	JS_NewClass(JS_GetRuntime(ctx), JS_CLASS_ID, &g_component_state_class);

	proto = JS_NewObject(ctx);
	JS_SetPropertyFunctionList(ctx, proto, g_component_state_funcs, ABSL_ARRAYSIZE(g_component_state_funcs));

	klass = JS_NewCFunction2(ctx,
		component_state_ctor,
		g_component_state_class.class_name,
		3,
		JS_CFUNC_constructor, 0);
	/* set proto.constructor and ctor.prototype */
	JS_SetConstructor(ctx, klass, proto);
	JS_SetClassProto(ctx, JS_CLASS_ID, proto);

	JSValue global = JS_GetGlobalObject(ctx);
	JS_SetPropertyStr(ctx, global, g_component_state_class.class_name, klass);
	JS_FreeValue(ctx, global);
}

JSValue ComponentState::newObject(JSContext* ctx, int argc, JSValueConst* argv)
{
	JSValue global = JS_GetGlobalObject(ctx);
	JSValue klass = JS_GetPropertyStr(ctx, global, g_component_state_class.class_name);
	JSValue obj = JS_CallConstructor(ctx, klass, argc, argv);
	JS_FreeValue(ctx, klass);
	JS_FreeValue(ctx, global);
	return obj;
}

void ComponentState::setNode(JSContext* ctx, JSValue this_val, scene2d::Node* node)
{
	auto comp_state = (ComponentState*)JS_GetOpaque2(ctx, this_val, JS_CLASS_ID);
	if (comp_state)
		comp_state->node_ = node;
}

JSValue ComponentState::useHook(JSContext* ctx, JSValue this_val, JSValue initial_state, JSValue mutater)
{
	JSValue updater = JS_NewCFunctionData(ctx, &ComponentState::useHookUpdater, 1, (int)curr_slot_, 1, &this_val);
	absl::Cleanup _ = [&]() {
		JS_FreeValue(ctx, updater);
		};
	if (curr_slot_ < slots_.size()) {
		JSValue argv[2] = { slots_[curr_slot_].state, updater };
		return JS_NewFastArray(ctx, 2, argv);
	} else if (curr_slot_ > slots_.size()) {
		return JS_UNDEFINED;
	} else {
		++curr_slot_;
		slots_.emplace_back(Slot{
			JS_DupValue(ctx, initial_state),
			JS_DupValue(ctx, mutater),
			});
		JSValue argv[2] = { slots_[curr_slot_ - 1].state, updater };
		return JS_NewFastArray(ctx, 2, argv);
	}
}

void ComponentState::cleanup(JSRuntime* rt)
{
	for (auto& slot : slots_) {
		JS_FreeValueRT(rt, slot.state);
		JS_FreeValueRT(rt, slot.mutater);
	}
	slots_.clear();
	curr_slot_ = 0;
}

JSValue ComponentState::useHookUpdater(JSContext* ctx, JSValueConst /*this_val*/, int argc, JSValueConst* argv, int magic, JSValue* func_data)
{
	LOG(INFO) << "useHookUpdater slot " << magic;
	auto me = (ComponentState*)JS_GetOpaque(func_data[0], ComponentState::JS_CLASS_ID);
	if (me && me->node_) {
		CHECK(magic >= 0 && magic < (int)me->slots_.size());
		auto scene = me->node_->scene();
		if (scene) {
			bool need_render = false;
			
			if (JS_IsFunction(ctx, me->slots_[magic].mutater)) {
				JSValue mutater_args[2] = { me->slots_[magic].state, argv[0] };
				JSValue ret = JS_Call(ctx, me->slots_[magic].mutater, func_data[0], 2, mutater_args);
				if (JS_IsArray(ctx, ret)) {
					int64_t len = 0;
					JS_GetPropertyLength(ctx, &len, ret);
					if (len == 2) {
						JSValue new_state = JS_GetPropertyUint32(ctx, ret, 0);
						JSValue should_render = JS_GetPropertyUint32(ctx, ret, 1);
						JS_FreeValue(ctx, me->slots_[magic].state);
						me->slots_[magic].state = new_state;
						need_render = JS_ToBool(ctx, should_render);
						JS_FreeValue(ctx, should_render);
					}
				}
				JS_FreeValue(ctx, ret);
			}

			if (need_render) {
				JSValue render_func = JS_GetPropertyStr(ctx, func_data[0], "render");
				JSValue comp_data = JS_Call(ctx, render_func, func_data[0], 0, nullptr);
				absl::Cleanup _ = [&]() {
					JS_FreeValue(ctx, render_func);
					JS_FreeValue(ctx, comp_data);
					};
				scene->updateComponentNode(me->node_, comp_data);
			}
		}
	}
	return JS_UNDEFINED;
}

void ComponentState::gcMark(JSRuntime* rt, JSValueConst val, JS_MarkFunc* mark_func)
{
	auto me = (ComponentState*)JS_GetOpaque(val, ComponentState::JS_CLASS_ID);
	if (me) {
		for (auto& slot : me->slots_) {
			JS_MarkValue(rt, slot.state, mark_func);
			JS_MarkValue(rt, slot.mutater, mark_func);
		}
	}
}

}