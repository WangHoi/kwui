#include "ComponentState.h"
#include "ContextId.h"
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
	//LOG(INFO) << "ComponentState: finalizer opaque " << ptr;
	if (ptr) {
		ptr->finalize(rt);
		delete ptr;
	}
	//LOG(INFO) << "__ComponentState finalizer called";
}

// 	constructor(renderFn, props, children)
static JSValue component_state_ctor(JSContext* ctx,
	JSValueConst new_target,
	int argc, JSValueConst* argv)
{
	JSValue obj = JS_UNDEFINED;
	JSValue proto;
	ComponentState* opaque;

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
	opaque = new ComponentState(obj);
	//LOG(INFO) << "ComponentState: setOpaque " << opaque;
	JS_SetOpaque(obj, opaque);
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
		me->curr_effect_ = 0;
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

	me->slots_.resize(me->curr_slot_);
	
	JS_SetPropertyStr(ctx, global, "__comp_state", JS_UNDEFINED);
	JS_FreeValue(ctx, global);
	return ret;
}

static JSClassDef g_component_state_class = {
	"__ComponentState",
	component_state_finalizer, // finalizer
	&ComponentState::gcMark,
};

static const JSCFunctionListEntry g_component_state_funcs[2] = {
	js_cfunc_def("render", 0, &component_state_render),
	js_cgetset_def("dialogId", &ComponentState::dialogId),
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
	if (comp_state) {
		if (!node) {
			comp_state->unmount(ctx, this_val);
		}
		comp_state->node_ = node;
	}
}

JSValue ComponentState::useHook(JSContext* ctx, JSValue this_val, JSValue init_fn, JSValue update_fn, JSValue cleanup_fn)
{
	//JSValue user_update_fn = JS_NewCFunctionData(ctx, &ComponentState::useHookUpdater, 1, (int)curr_slot_, 1, &this_val);
	JSValue data = JS_NewObject(ctx);
	JS_SetOpaque(data, this);;
	JSValue user_update_fn = JS_NewCFunctionData(ctx, &ComponentState::useHookUpdater, 1, (int)curr_slot_, 1, &data);
	JS_FreeValue(ctx, data);
	absl::Cleanup _ = [&]() {
		JS_FreeValue(ctx, user_update_fn);
		};
	if (curr_slot_ < slots_.size()) {
		JSValue argv[2] = { slots_[curr_slot_].state, user_update_fn };
		++curr_slot_;
		return JS_NewFastArray(ctx, 2, argv);
	} else if (curr_slot_ > slots_.size()) {
		++curr_slot_;
		return JS_UNDEFINED;
	} else {
		++curr_slot_;
		slots_.emplace_back(Slot{
			JS_UNDEFINED,
			JS_DupValue(ctx, update_fn),
			JS_DupValue(ctx, cleanup_fn),
			});
		// LOG(INFO) << "ComponentState: slots append opaque " << this;
		JSValue init_state = JS_Call(ctx, init_fn, this_val, 1, &user_update_fn);
		if (JS_IsException(init_state)) {
			js_std_dump_error(ctx);
		} else {
			slots_.back().state = init_state;
		}
		JSValue argv[2] = { init_state, user_update_fn };
		return JS_NewFastArray(ctx, 2, argv);
	}
}

JSValue ComponentState::provideContext(JSContext* ctx, JSValue this_val, JSValue id, JSValue user_ctx)
{
	auto cid = (ContextId*)JS_GetOpaque(id, ContextId::JS_CLASS_ID);
	contexts_[cid->id()] = Value(ctx, user_ctx);
	return JS_UNDEFINED;
}

JSValue ComponentState::useContext(JSContext* ctx, JSValue this_val, JSValue id)
{
	auto cid = (ContextId*)JS_GetOpaque(id, ContextId::JS_CLASS_ID);
	if (!cid) {
		return JS_UNDEFINED;
	}
	auto p = this;
	while (p) {
		auto it = p->contexts_.find(cid->id());
		if (it != p->contexts_.end()) {
			return JS_DupValue(ctx, it->second.jsValue());
		}
		p = p->parent();
	}
	return JS_UNDEFINED;
}

JSValue ComponentState::useEffect(JSContext* ctx, JSValue this_val, JSValue setup_fn, JSValue deps, JSValue compare_fn)
{
	if (!node_) {
		LOG(ERROR) << "__useEffect: no Node.";
		return JS_UNDEFINED;
	}
	if (!node_->scene()) {
		LOG(ERROR) << "__useEffect: no Scene.";
		return JS_UNDEFINED;
	}
	DCHECK(JS_IsFunction(ctx, compare_fn)) << "__useEffect: compareFn must be function.";
	if (curr_effect_ >= effects_.size()) {
		++curr_effect_;
		effects_.emplace_back(Effect{
			Effect::STATE_INIT,
			0,
			Value(ctx, deps),
			Value(ctx, setup_fn),
			Value(),
			});
		effects_.back().tid = node_->scene()->addPostRenderTask(makeUseEffectTask(ctx, curr_effect_ - 1));
		effects_.back().state = Effect::STATE_SCHEDULED;
	} else {
		++curr_effect_;
		auto& eff = effects_[curr_effect_ - 1];
		bool changed = true;
		if (JS_IsArray(ctx, deps)) {
			JSValue args[2] = { deps, eff.deps.jsValue() };
			JSValue ret = JS_Call(ctx, compare_fn, JS_UNDEFINED, ABSL_ARRAYSIZE(args), args);
			if (JS_IsException(ret)) {
				LOG(ERROR) << "__useEffect: compareFn() failed:";
				js_std_dump_error(ctx);
				changed = false;
			} else {
				changed = (JS_ToBool(ctx, ret) == 0);
			}
			JS_FreeValue(ctx, ret);
		}
		if (changed) {
			if (eff.state == Effect::STATE_SCHEDULED) {
				// Cancel scheduled task
				node_->scene()->removePostRenderTask(eff.tid);
				eff.tid = 0;
			}
			eff.state = Effect::STATE_INIT;
			eff.deps = Value(ctx, deps);
			eff.setup_fn = Value(ctx, setup_fn);

			eff.tid = node_->scene()->addPostRenderTask(makeUseEffectTask(ctx, curr_effect_ - 1));
			eff.state = Effect::STATE_SCHEDULED;
		}
	}
	return JS_UNDEFINED;
}

void ComponentState::finalize(JSRuntime* rt)
{
	for (auto& slot : slots_) {
		JS_FreeValueRT(rt, slot.state);
		JS_FreeValueRT(rt, slot.cleanup_fn);
		JS_FreeValueRT(rt, slot.update_fn);
	}
	slots_.clear();
	effects_.clear();
	curr_slot_ = 0;
	curr_effect_ = 0;
	this_obj_ = JS_UNDEFINED;
}

ComponentState::ComponentState(JSValue this_obj)
	: this_obj_(this_obj)
{
	weak_proxy_ = new base::WeakObjectProxy<ComponentState>(this);
	weak_proxy_->retain();
}

ComponentState::~ComponentState()
{
	weak_proxy_->clear();
	weak_proxy_->release();
}

JSValue ComponentState::useHookUpdater(JSContext* ctx, JSValueConst /*this_val*/, int argc, JSValueConst* argv, int magic, JSValue* func_data)
{
	//LOG(INFO) << "useHookUpdater slot " << magic;
	auto me = (ComponentState*)JS_GetOpaque(func_data[0], 1/*JS_CLASS_OBJECT*/);
	auto this_obj = me->this_obj_;
	if (me && me->node_) {
		CHECK(magic >= 0 && magic < (int)me->slots_.size());
		auto scene = me->node_->scene();
		if (scene) {
			bool need_render = false;
			
			if (JS_IsFunction(ctx, me->slots_[magic].update_fn)) {
				JSValue mutater_args[2] = { me->slots_[magic].state, argv[0] };
				JSValue ret = JS_Call(ctx, me->slots_[magic].update_fn, this_obj, 2, mutater_args);
				if (JS_IsArray(ctx, ret)) {
					int64_t len = 0;
					JS_GetPropertyLength(ctx, &len, ret);
					if (len == 2) {
						JSValue new_state = JS_GetPropertyUint32(ctx, ret, 0);
						JSValue should_render = JS_GetPropertyUint32(ctx, ret, 1);
						JSValue old_state = me->slots_[magic].state;
						me->slots_[magic].state = new_state;
						JS_FreeValue(ctx, old_state);

						auto nj = JS_JSONStringify(ctx, new_state, JS_UNDEFINED, JS_UNDEFINED);
						auto njs = Context::parse<std::string>(ctx, nj);
						//LOG(INFO) << "after update, new state " << njs;
						JS_FreeValue(ctx, nj);

						need_render = JS_ToBool(ctx, should_render);
						JS_FreeValue(ctx, should_render);
					}
				}
				JS_FreeValue(ctx, ret);
			}

			if (need_render) {
				JSValue render_func = JS_GetPropertyStr(ctx, this_obj, "render");
				JSValue comp_data = JS_Call(ctx, render_func, this_obj, 0, nullptr);
				absl::Cleanup _ = [&]() {
					JS_FreeValue(ctx, render_func);
					JS_FreeValue(ctx, comp_data);
					};
				scene->updateComponentNodeChildren(me->node_, comp_data);
			}
		}
	}
	return JS_UNDEFINED;
}

void ComponentState::unmount(JSContext* ctx, JSValueConst this_val)
{
	for (auto& slot : slots_) {
		if (JS_IsFunction(ctx, slot.cleanup_fn)) {
			JSValue ret = JS_Call(ctx, slot.cleanup_fn, this_val, 1, &slot.state);
			if (JS_IsException(ret)) {
				js_std_dump_error(ctx);
			}
			JS_FreeValue(ctx, ret);
		}
	}
	for (auto& eff : effects_) {
		if (eff.state == Effect::STATE_SCHEDULED) {
			node_->scene()->removePostRenderTask(eff.tid);
			eff.tid = 0;
		}

		if (eff.cleanup_fn.isFunction()) {
			JSValue ret = JS_Call(ctx, eff.cleanup_fn.jsValue(), this_val, 0, nullptr);
			if (JS_IsException(ret)) {
				js_std_dump_error(ctx);
			}
			JS_FreeValue(ctx, ret);
		}

		eff.state = Effect::STATE_INIT;
		eff.tid = 0;
		eff.deps = Value();
		eff.setup_fn = Value();
		eff.cleanup_fn = Value();
	}
}

ComponentState* ComponentState::parent() const
{
	if (!node_)
		return nullptr;
	auto p = node_->parent();
	while (p) {
		if (p->type() == scene2d::NodeType::NODE_COMPONENT) {
			JSValue state = p->componentState();
			return (ComponentState*)JS_GetOpaque(state, JS_CLASS_ID);
		}
		p = p->parent();
	}
	return nullptr;
}

std::function<void()> ComponentState::makeUseEffectTask(JSContext* ctx, size_t index) const
{
	auto link = weaken();
	return [link, ctx, index]() -> void {
		auto me = link.get();
		if (!me || !me->node_)
			return;
		me->runEffect(ctx, index);
		};
}

void ComponentState::runEffect(JSContext* ctx, size_t index)
{
	if (index >= effects_.size())
		return;
	auto& eff = effects_[index];

	if (eff.cleanup_fn.isFunction()) {
		JSValue ret = JS_Call(ctx, eff.cleanup_fn.jsValue(), this_obj_, 0, nullptr);
		if (JS_IsException(ret)) {
			LOG(ERROR) << "__useEffect: run cleanupFn() failed.";
			js_std_dump_error(ctx);
		}
		JS_FreeValue(ctx, ret);
	}

	JSValue ret = JS_Call(ctx, eff.setup_fn.jsValue(), this_obj_, 0, nullptr);
	if (JS_IsException(ret))
		js_std_dump_error(ctx);
	eff.state = Effect::STATE_EXECUTED;
	eff.tid = 0;
	if (JS_IsFunction(ctx, ret)) {
		eff.cleanup_fn = Value(ctx, ret);
	}
	JS_FreeValue(ctx, ret);
}

void ComponentState::gcMark(JSRuntime* rt, JSValueConst val, JS_MarkFunc* mark_func)
{
	auto me = (ComponentState*)JS_GetOpaque(val, ComponentState::JS_CLASS_ID);
	if (me) {
		for (auto& slot : me->slots_) {
			JS_MarkValue(rt, slot.state, mark_func);
			JS_MarkValue(rt, slot.update_fn, mark_func);
		}
		for (auto& p : me->contexts_) {
			JS_MarkValue(rt, p.second.jsValue(), mark_func);
		}
		for (auto& e : me->effects_) {
			JS_MarkValue(rt, e.deps.jsValue(), mark_func);
			JS_MarkValue(rt, e.setup_fn.jsValue(), mark_func);
			JS_MarkValue(rt, e.cleanup_fn.jsValue(), mark_func);
		}
	}
}

JSValue ComponentState::dialogId(JSContext* ctx, JSValueConst this_val)
{
	auto me = (ComponentState*)JS_GetOpaque(this_val, ComponentState::JS_CLASS_ID);
	if (me->node_ && me->node_->scene()) {
		return JS_NewString(ctx, me->node_->scene()->eventContextId().c_str());
	} else {
		return JS_UNDEFINED;
	}
}

}