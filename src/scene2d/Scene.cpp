#include "Scene.h"
#include "Node.h"
#include "script/script.h"
#include "style/style.h"
#include "style/Layout.h"
#include "style/StylePaint.h"
#include "style/LayoutObject.h"
#include "absl/functional/bind_front.h"
#include "absl/cleanup/cleanup.h"
#include "base/log.h"
#include "graph2d/Painter.h"
#include "absl/time/clock.h"
namespace scene2d {

static std::vector<std::unique_ptr<style::StyleRule>> makeStyleRules(JSContext* ctx, JSValue stylesheet)
{
	std::vector<std::unique_ptr<style::StyleRule>> style_rules;

	if (JS_IsObject(stylesheet)) {
		script::Context::eachObjectField(ctx, stylesheet, [&](const char* name, JSValue value) {
			auto selectors_res = style::Selector::parseGroup(name);
			auto style_spec = script::parse<style::StyleSpec>(ctx, value);
			if (selectors_res.ok()) {
				for (auto&& selector : *selectors_res) {
					auto rule = std::make_unique<style::StyleRule>(std::move(selector), style_spec);
					style_rules.push_back(std::move(rule));
				}
			} else {
				LOG(WARNING) << "parse selector '" << name << "' failed";
			}
			});
	} else if (JS_IsString(stylesheet)) {
		const char* s = JS_ToCString(ctx, stylesheet);
		if (s) {
			auto css_res = style::parse_css(s);
			if (css_res.ok()) {
				auto& stylesheet = css_res.value();
				std::stable_sort(stylesheet.begin(), stylesheet.end(),
					[](const std::unique_ptr<style::StyleRule>& a, const std::unique_ptr<style::StyleRule>& b) -> bool {
						return a->specificity() < b->specificity();
					});
				for (auto&& rule : stylesheet) {
					style_rules.push_back(std::move(rule));
				}
			} else {
				LOG(ERROR) << "parse stylesheet [" << s << "] failed";
			}
		}
		JS_FreeCString(ctx, s);
	}

	return style_rules;
}

struct SceneStyleResolveContext {
	struct Scope {
		absl::Span<std::unique_ptr<style::StyleRule>> rules;
		Node* root;
	};
	absl::Span<std::unique_ptr<style::StyleRule>> global;
	std::vector<Scope> scopes;
	std::vector<NodeStyleResolveContext*> pelem_ctxs; // parent element node contexts

	inline Node* currentScopedRoot() const {
		return scopes.empty() ? nullptr : scopes.back().root;
	}
	inline absl::Span<Node*> currentPrecedents() const {
		return pelem_ctxs.empty() ? absl::Span<Node*>() : absl::MakeSpan(pelem_ctxs.back()->prev_siblings);
	}

private:
	SceneStyleResolveContext(const SceneStyleResolveContext&) = delete;
	SceneStyleResolveContext& operator=(const SceneStyleResolveContext&) = delete;
};

Scene::Scene(EventContext& event_ctx)
	: event_ctx_(event_ctx)
{
	script_ctx_ = std::make_unique<script::Context>();
	root_ = createElementNode(base::string_intern("kml"));
	root_->retain();
	weakptr_ = new base::WeakObjectProxy<Scene>(this);
	weakptr_->retain();
	requestAnimationFrame(root_);
}

Scene::~Scene()
{
	weakptr_->clear();
	weakptr_->release();
	if (root_) {
		root_->release();
		root_ = nullptr;
	}
}

Node* Scene::createTextNode(const std::string& text)
{
	auto actor = new Node(this, NodeType::NODE_TEXT, text);
	return actor;
}

Node* Scene::createElementNode(base::string_atom tag)
{
	auto actor = new Node(this, NodeType::NODE_ELEMENT, tag);
	return actor;
}

Node* Scene::createComponentNode(Node* parent, JSValue comp_data)
{
	JSContext* jctx = script_ctx_->get();
	if (JS_IsString(comp_data)) {
		std::string text = script_ctx_->parse<std::string>(comp_data);
		//LOG(INFO) << "createTextNode: " << text;
		auto node = createTextNode(text);
		if (parent)
			parent->appendChild(node);
		return node;
	} else if (JS_IsArray(jctx, comp_data)) {
		//LOG(INFO) << "createElementNode: " << "fragment";
		Node* node = createElementNode(base::string_intern("fragment"));
		if (parent)
			parent->appendChild(node);

		int64_t length = 0;
		JS_GetPropertyLength(jctx, &length, comp_data);
		for (uint32_t i = 0; i < (uint32_t)length; ++i) {
			JSValue child_comp_data = JS_GetPropertyUint32(jctx, comp_data, i);
			Node* child = createComponentNode(node, child_comp_data);
			JS_FreeValue(jctx, child_comp_data);
		}
		return node;
	} else if (JS_IsObject(comp_data)) {
		JSValue render = JS_GetPropertyStr(jctx, comp_data, "render");
		absl::Cleanup _ = [&]() {
			JS_FreeValue(jctx, render);
			};
		if (JS_IsFunction(jctx, render)) {
			//LOG(INFO) << "createComponentNode";
			Node* node = new Node(this, NodeType::NODE_COMPONENT, comp_data);
			if (parent)
				parent->appendChild(node);

			JSValue child_comp_data = JS_Call(jctx, render, comp_data, 0, nullptr);
			createComponentNode(node, child_comp_data);
			JS_FreeValue(jctx, child_comp_data);
			return node;
		} else {
			JSValue tag = JS_GetPropertyStr(jctx, comp_data, "tag");
			JSValue atts = JS_GetPropertyStr(jctx, comp_data, "atts");
			JSValue kids = JS_GetPropertyStr(jctx, comp_data, "kids");
			absl::Cleanup _ = [&]() {
				JS_FreeValue(jctx, tag);
				JS_FreeValue(jctx, atts);
				JS_FreeValue(jctx, kids);
				};
			if (JS_IsString(tag) && JS_IsObject(atts) && JS_IsArray(jctx, kids)) {
				std::string tagName = script_ctx_->parse<std::string>(tag);
				//LOG(INFO) << "createElementNode: " << tagName;
				if (tagName == "style") {
					int64_t length = 0;
					JS_GetPropertyLength(jctx, &length, kids);
					if (length == 1) {
						JSValue jstyle = JS_GetPropertyUint32(jctx, kids, 0);
						setStyleSheet(parent, jstyle);
						JS_FreeValue(jctx, jstyle);
					} else {
						LOG(WARNING) << "parse <style> failed: invalid children count " << length;
					}
					return nullptr;
				} else {
					Node* node = createElementNode(base::string_intern(tagName));
					if (parent)
						parent->appendChild(node);

					// Setup properties
					setupProps(node, atts);

					// Create children
					int64_t length = 0;
					JS_GetPropertyLength(jctx, &length, kids);
					for (uint32_t i = 0; i < (uint32_t)length; ++i) {
						JSValue child_comp_data = JS_GetPropertyUint32(jctx, kids, i);
						createComponentNode(node, child_comp_data);
						JS_FreeValue(jctx, child_comp_data);
					}

					return node;
				}
			} else {
				auto a = JS_JSONStringify(jctx, comp_data, JS_UNDEFINED, JS_UNDEFINED);
				LOG(WARNING) << "Unknown: " << script_ctx_->parse<std::string>(a);
				JS_FreeValue(jctx, a);
				return nullptr;
			}
		}
	} else if (JS_IsNull(comp_data) || JS_IsUndefined(comp_data) || JS_IsUninitialized(comp_data)) {
		return nullptr;
	} else if (JS_IsException(comp_data)) {
		js_std_dump_error(jctx);
	}
	LOG(WARNING) << "Unknown: " << script_ctx_->parse<std::string>(comp_data);
	return nullptr;
}

void Scene::updateComponentNodeChildren(Node* node, JSValue comp_data)
{
	//LOG(INFO) << "Scene::updateComponentNode";
	CHECK(node->type_ == NodeType::NODE_COMPONENT)
		<< "Scene::updateComponentNode(): expect component node";
	JSContext* ctx = script_ctx_->get();

	JSValue arr = JS_NewFastArray(ctx, 1, &comp_data);
	updateNodeChildren(node, ctx, arr);
	JS_FreeValue(ctx, arr);

	requestUpdate();
	requestPaint();
}

void Scene::setScriptModule(const std::string& base_filename,
	const std::string& module_path,
	const script::Value& module_params)
{
	script_module_.emplace<ModuleInfo>({ base_filename, module_path, module_params });
}

void Scene::reloadScriptModule()
{
	if (!script_module_.has_value()) {
		LOG(INFO) << "reload dialog failed, not script module.";
		return;
	}

	script_ctx_->incrementReloadVersion();
	JSContext* ctx = script_ctx_->get();
	JSModuleDef* mod = JS_RunModule(
		ctx,
		script_module_->base_filename.c_str(),
		script_module_->module_path.c_str());
	if (!mod) {
		LOG(INFO) << absl::StrFormat(
			"reload dialog failed, run module %s %s failed.",
			script_module_->base_filename.c_str(),
			script_module_->module_path.c_str());
		js_std_dump_error(ctx);
		return;
	}

	JSValue builder = JS_GetModuleExportItemStr(ctx, mod, "builder");
	absl::Cleanup _ = [&]() {
		JS_FreeValue(ctx, builder);
		};
	if (JS_IsFunction(ctx, builder)) {
		JSValue ret = JS_Call(ctx, builder, JS_UNDEFINED, 1, &script_module_->module_params.jsValue());
		absl::Cleanup _ = [&]() {
			JS_FreeValue(ctx, ret);
			};
		if (JS_IsObjectPlain(ctx, ret)) {
			JSValue root = JS_GetPropertyStr(ctx, ret, "root");
			JSValue stylesheet = JS_GetPropertyStr(ctx, ret, "stylesheet");
			absl::Cleanup _ = [&]() {
				JS_FreeValue(ctx, root);
				JS_FreeValue(ctx, stylesheet);
				};

			setStyleSheet(stylesheet);
			if (root != JS_UNDEFINED) {
				JSValue kids = JS_NewFastArray(ctx, 1, &root);
				updateNodeChildren(root_, ctx, kids);
				JS_FreeValue(ctx, kids);
			}
			LOG(INFO) << "reload dialog finished";
		} else {
			LOG(INFO) << absl::StrFormat(
				"reload dialog failed, module.builder return value not { root, stylesheet }: %s, %s",
				script_module_->base_filename.c_str(),
				script_module_->module_path.c_str());
		}
	} else {
		LOG(INFO) << absl::StrFormat(
			"reload dialog failed, module.builder not function: %s, %s",
			script_module_->base_filename.c_str(),
			script_module_->module_path.c_str());
	}
}

void Scene::setStyleSheet(JSValue stylesheet)
{
	style_rules_ = makeStyleRules(script_ctx_->get(), stylesheet);
}

Node* Scene::pickNode(const PointF& pos, int flag_mask, PointF* out_local_pos)
{
	for (auto it = flow_roots_.rbegin(); it != flow_roots_.rend(); ++it) {
		auto scene_pos = mapPointToScene(it->root->node, PointF());
		style::LayoutObject* o = style::LayoutObject::pick(it->root, pos - scene_pos, flag_mask, out_local_pos);
		if (o)
			return o->node;
	}
	return nullptr;
}

void Scene::appendStyleRule(std::unique_ptr<style::StyleRule>&& rule)
{
	if (style_rules_.empty()) {
		style_rules_.emplace_back(std::move(rule));
		return;
	}
	auto sp = rule->specificity();
	if (style_rules_.back()->specificity() <= sp) {
		style_rules_.emplace_back(std::move(rule));
		return;
	}
	for (auto it = style_rules_.begin(); it != style_rules_.end(); ++it) {
		if (sp < (*it)->specificity()) {
			style_rules_.insert(it, std::move(rule));
			break;
		}
	}
}

void Scene::resolveStyle()
{
	SceneStyleResolveContext ctx = {};
	ctx.global = absl::MakeSpan(style_rules_);
	resolveNodeStyle(ctx, root_);
}

void Scene::computeLayout(float width, absl::optional<float> height)
{
	style::LayoutTreeBuilder b(root_);
	flow_roots_ = b.build();

	for (auto& flow_root : flow_roots_) {
		style::LayoutObject::reflow(flow_root, width, height);
	}

	// trigger Control::onLayout
	layoutComputed(root_);
}

void Scene::paint(graph2d::PainterInterface* painter)
{
	for (auto& fl : flow_roots_) {
		auto scene_pos = mapPointToScene(fl.root->node, PointF());
		// LOG(INFO) << "Paint flow root " << *fl.root << ", scene_pos=" << scene_pos;
		painter->setTranslation(scene_pos, false);
		style::LayoutObject::paint(fl.root, painter);
		painter->restore();
	}
}

scene2d::PointF Scene::getMousePosition() const
{
	return event_ctx_.GetMousePosition();
}

void Scene::requestPaint()
{
	event_ctx_.RequestPaint();
}

void Scene::requestUpdate()
{
	event_ctx_.RequestUpdate();
}

void Scene::requestAnimationFrame(scene2d::Node* node)
{
	event_ctx_.RequestAnimationFrame(node);
}

PointF Scene::mapPointToScene(Node* node, const PointF& pos, bool include_flow_root) const
{
	// LOG(INFO) << "Scene::mapPointToScene";
	style::LayoutObject* o = &node->layout_;
	scene2d::PointF p = pos;
	while (o->parent) {
		p += style::LayoutObject::contentRect(o).origin();
		p += style::LayoutObject::pos(o);
		o = o->parent;
		if (o->scroll_object.has_value()) {
			p -= o->scroll_object.value().scroll_offset;
		}
	}

	// TODO: handle scroll in positioned ascendants
	auto it = std::find_if(flow_roots_.begin(), flow_roots_.end(), [&](const style::FlowRoot& fl) {
		return fl.root == o;
		});
	if (it != flow_roots_.end()) {
		const style::FlowRoot& fl = *it;

		if (include_flow_root) {
			p += style::LayoutObject::contentRect(o).origin();
			p += style::LayoutObject::pos(o);
		}

		if (it->positioned_parent) {
			p += mapPointToScene(it->positioned_parent->node, PointF());
		}

		auto po = fl.positioned_parent;
		while (po) {
			if (po->scroll_object.has_value()) {
				LOG(INFO) << "TODO: handle scroll in positioned ascendants";
				break;
			}
			auto it = std::find_if(flow_roots_.begin(), flow_roots_.end(), [&](const style::FlowRoot& fl) {
				return fl.root == po;
				});
			po = (it == flow_roots_.end()) ? nullptr : it->root;
		}
	}

	return p;
}

std::string Scene::eventContextId() const
{
	return event_ctx_.eventContextId();
}

base::TaskQueue::TaskId Scene::addPostRenderTask(std::function<void()>&& fn)
{
	return post_render_task_queue_.add(std::move(fn));
}

bool Scene::removePostRenderTask(base::TaskQueue::TaskId id)
{
	return post_render_task_queue_.remove(id);
}

void Scene::runPostRenderTasks()
{
	post_render_task_queue_.run();
}

void Scene::dispatchEvent(Node* node, Event& event, bool bubble)
{
	node->onEvent(event);
	if (bubble) {
		auto p = node->parent();
		while (p) {
			p->onEvent(event);
			p = p->parent();
		}
	}
}

std::tuple<float, float> Scene::intrinsicWidth()
{
	resolveStyle();
	computeLayout(std::numeric_limits<float>::max(), absl::nullopt);
	return std::make_tuple(std::max(0.0f, root_->layout_.min_width),
		std::max(0.0f, root_->layout_.max_width));
}

float Scene::intrinsicHeight(float width)
{
	resolveStyle();
	computeLayout(width, absl::nullopt);
	if (root_->layout_.bfc.has_value()) {
		return std::max(0.0f, root_->layout_.bfc.value().margin_bottom_edge);
	} else {
		return 0.0f;
	}
}

void Scene::setupProps(Node* node, JSValue props)
{
	JSContext* ctx = script_ctx_->get();
	bool new_style = false;
	bool new_id = false;
	bool new_class = false;
	std::map<base::string_atom, NodeAttributeValue> new_attrs;
	std::map<base::string_atom, script::Value> new_ehandlers;
	script_ctx_->eachObjectField(props, [&](const char* name_str, JSValue value) {
		auto name = base::string_intern(name_str);
		if (name == base::string_intern("style")) {
			new_style = true;
			if (JS_IsObject(value)) {
				node->setStyle(script_ctx_->parse<style::StyleSpec>(value));
			} else if (JS_IsString(value)) {
				const char* s = JS_ToCString(ctx, value);
				auto style_res = style::parse_inline_style(s);
				JS_FreeCString(ctx, s);
				if (style_res.ok()) {
					node->setStyle(style_res.value());
				} else {
					LOG(ERROR) << "parse inline style [" << s << "] failed.";
				}
			}
		} else if (name == base::string_intern("id")) {
			new_id = true;
			node->setId(script_ctx_->parse<base::string_atom>(value));
		} else if (name == base::string_intern("class")) {
			new_class = true;
			const char* s = JS_ToCString(ctx, value);
			if (s) {
				node->setClass(style::Classes::parse(s));
			}
			JS_FreeCString(ctx, s);
		} else if (name == base::string_intern("ref")) {
			if (JS_IsObjectPlain(ctx, value)) {
				JSValue elem_ref = script::ElementRef::create(ctx, node->weaken());
				JS_SetPropertyStr(ctx, value, "current", elem_ref);
			}
		} else if (JS_IsFunction(ctx, value)) {
			new_ehandlers[name] = script::Value(ctx, value);
		} else {
			new_attrs[name] = script::wrap(ctx, value);
		}
#if 0
		} else if (JS_IsNumber(value)) {
			double f64;
			JS_ToFloat64(ctx, &f64, value);
			new_attrs[name] = (float)f64;
		} else if (JS_IsString(value)) {
			const char* s = JS_ToCString(ctx, value);
			new_attrs[name] = std::string(s);
			JS_FreeCString(ctx, s);
		} else {
			JSValue jval = JS_ToString(ctx, value);
			const char* s = JS_ToCString(ctx, jval);
			new_attrs[name] = std::string(s);
			JS_FreeCString(ctx, s);
			JS_FreeValue(ctx, jval);
		}
#endif
		});

// Check to clear style/id/class
if (!new_style)
node->setStyle(style::StyleSpec());
if (!new_id)
node->setId(base::string_atom());
if (!new_class)
node->setClass(style::Classes());

// Update attributes
std::set<base::string_atom> attrs_to_remove;
for (auto& p : node->attrs_) {
	if (new_attrs.find(p.first) == new_attrs.end())
		attrs_to_remove.insert(p.first);
}
for (auto& p : new_attrs) {
	node->setAttribute(p.first, p.second);
}
for (auto& a : attrs_to_remove) {
	node->setAttribute(a, NodeAttributeValue());
}

// Update event handlers
std::set<base::string_atom> ehandlers_to_remove;
for (auto& p : node->event_handlers_) {
	if (new_ehandlers.find(p.first) == new_ehandlers.end())
		ehandlers_to_remove.insert(p.first);
}
for (auto& p : new_ehandlers) {
	node->setEventHandler(p.first, p.second);
}
for (auto& e : ehandlers_to_remove) {
	node->setEventHandler(e, script::Value());
}
}

bool Scene::match(absl::Span<Node*> precedents, Node* node, style::Selector* selector)
{
	if (!node)
		return false;

	if (!node->matchSimple(selector))
		return false;

	if (!selector->dep_selector)
		return true;

	if (selector->dep_type == style::SelectorDependency::DirectParent)
		return match(precedents, node->parentElement(), selector->dep_selector.get());

	if (selector->dep_type == style::SelectorDependency::Ancestor) {
		Node* pnode = node->parentElement();
		style::Selector* psel = selector->dep_selector.get();
		while (pnode) {
			if (match(precedents, pnode, psel))
				return true;
			pnode = pnode->parentElement();
		}
		return false;
	}

	if (selector->dep_type == style::SelectorDependency::DirectPrecedent) {
		if (precedents.empty())
			return false;
		return match(precedents.subspan(0, precedents.size() - 1),
			precedents.back(),
			selector->dep_selector.get());
	}

	if (selector->dep_type == style::SelectorDependency::Precedent) {
		if (precedents.empty())
			return false;
		for (size_t idx = precedents.size() - 1; idx >= 0; --idx) {
			if (match(precedents.subspan(0, idx),
				precedents[idx],
				selector->dep_selector.get())) {
				return true;
			}
		}
	}

	return true;
}

void Scene::resolveNodeStyle(SceneStyleResolveContext& sctx, Node* node)
{
	// Handle scoped css
	if (!node->style_rules_.empty()) {
		sctx.scopes.push_back(SceneStyleResolveContext::Scope{
			absl::MakeSpan(node->style_rules_), node
			});
	}

	if (node->type() == NodeType::NODE_COMPONENT) {
		node->computed_style_ = node->parent_->computed_style_;
		node->eachChild(absl::bind_front(&Scene::resolveNodeStyle, this, std::ref(sctx)));
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		node->resolveDefaultStyle();
		NodeStyleResolveContext nctx;
		nctx.scoped_root = sctx.currentScopedRoot();
		for (auto& rule : sctx.global) {
			if (match(sctx.currentPrecedents(), node, rule->selector.get())) {
				node->resolveStyle(nctx, rule->spec);
			}
		}
		for (auto& scope : sctx.scopes) {
			for (auto& rule : scope.rules) {
				if (match(sctx.currentPrecedents(), node, rule->selector.get())) {
					node->resolveStyle(nctx, rule->spec);
				}
			}
		}
		node->resolveInlineStyle(nctx);

		if (!sctx.pelem_ctxs.empty()) {
			sctx.pelem_ctxs.back()->prev_siblings.push_back(node);
		}

		sctx.pelem_ctxs.push_back(&nctx);
		node->eachChild(absl::bind_front(&Scene::resolveNodeStyle, this, std::ref(sctx)));
		sctx.pelem_ctxs.pop_back();
	} else if (node->type() == NodeType::NODE_TEXT) {
		node->resolveDefaultStyle();
		node->updateTextLayout();
	}

	// Handle scoped css
	if (!node->style_rules_.empty()) {
		sctx.scopes.pop_back();
	}
}

void Scene::paintNode(Node* node, graph2d::PainterInterface* painter)
{
	style::LayoutObject::paint(&node->layout_, painter);
}

void Scene::layoutComputed(Node* node)
{
	node->layoutComputed();
	Node::eachLayoutChild(node, absl::bind_front(&Scene::layoutComputed, this));
}

void Scene::updateNodeChildren(Node* node, JSContext* ctx, JSValue comp_data)
{
	if (!JS_IsArray(ctx, comp_data)) {
		LOG(WARNING) << "updateNodeChildren failed, comp_data not array";
		return;
	}

	int prev_child_count = (int)node->children_.size();
	int64_t len = 0;
	JS_GetPropertyLength(ctx, &len, comp_data);
	int next_child_count = (int)std::max<int64_t>(0, len);
	int prev_patched = 0, next_patched = 0;
	for (prev_patched = 0, next_patched = 0; prev_patched < prev_child_count && next_patched < next_child_count; ++prev_patched, ++next_patched) {
		Node* old_child = node->children_[prev_patched];
		JSValue child_comp_data = JS_GetPropertyUint32(ctx, comp_data, (uint32_t)next_patched);
		absl::Cleanup _ = [&]() {
			JS_FreeValue(ctx, child_comp_data);
			};
		NodeCompareResult res = compareNode(old_child, ctx, child_comp_data);
		if (res == NodeCompareResult::PatchableTextNode) {
			updateTextNode(old_child, ctx, child_comp_data);
		} else if (res == NodeCompareResult::PatchableElementNode) {
			updateElementNode(old_child, ctx, child_comp_data);
		} else if (res == NodeCompareResult::PatchableFragmentElementNode) {
			updateNodeChildren(old_child, ctx, child_comp_data);
		} else if (res == NodeCompareResult::PatchableComponentNode) {
			updateComponentNode(old_child, ctx, child_comp_data);
		} else if (res == NodeCompareResult::PatchableStyleSheet) {
			JSValue kids = JS_GetPropertyStr(ctx, comp_data, "kids");
			absl::Cleanup _ = [&]() {
				JS_FreeValue(ctx, kids);
			};
			int64_t length = 0;
			JS_GetPropertyLength(ctx, &length, kids);
			if (length == 1) {
				JSValue jstyle = JS_GetPropertyUint32(ctx, kids, 0);
				setStyleSheet(node, jstyle);
				JS_FreeValue(ctx, jstyle);
			} else {
				LOG(WARNING) << "parse <style> failed: invalid children count " << length;
			}
			--prev_patched;
		} else {
			break;
		}
	}

	for (size_t i = next_patched; i < next_child_count; ++i) {
		JSValue child_comp_data = JS_GetPropertyUint32(ctx, comp_data, (uint32_t)i);
		createComponentNode(node, child_comp_data);
		JS_FreeValue(ctx, child_comp_data);
	}

	for (size_t i = prev_patched; i < prev_child_count; ++i) {
		Node* old_child = node->removeChildAt(prev_patched);
		old_child->release();
	}
}

Scene::NodeCompareResult Scene::compareNode(Node* node, JSContext* ctx, JSValue comp_data)
{
	if (JS_IsString(comp_data)) {
		if (node->type_ == NodeType::NODE_TEXT)
			return NodeCompareResult::PatchableTextNode;
	} else if (JS_IsArray(ctx, comp_data)) {
		if (node->type_ == NodeType::NODE_ELEMENT && node->tag_ == "fragment")
			return NodeCompareResult::PatchableFragmentElementNode;
	} else if (JS_IsObject(comp_data)) {
		JSValue render = JS_GetPropertyStr(ctx, comp_data, "renderFn");
		absl::Cleanup _ = [&]() {
			JS_FreeValue(ctx, render);
			};
		//{
		//	auto j = JS_JSONStringify(ctx, comp_data, JS_UNDEFINED, JS_UNDEFINED);
		//	auto jstr = JS_ToCString(ctx, j);
		//	JS_FreeCString(ctx, jstr);
		//	JS_FreeValue(ctx, j);
		//}
		if (JS_IsFunction(ctx, render)) {
			if (node->type_ == NodeType::NODE_COMPONENT) {
				JSValue node_render = JS_GetPropertyStr(ctx, node->comp_state_, "renderFn");
				absl::Cleanup _ = [&]() {
					JS_FreeValue(ctx, node_render);
					};
				if (JS_AreFunctionsOfSameOrigin(ctx, render, node_render)) {
					return NodeCompareResult::PatchableComponentNode;
				}
			}
		} else {
			JSValue tag = JS_GetPropertyStr(ctx, comp_data, "tag");
			JSValue atts = JS_GetPropertyStr(ctx, comp_data, "atts");
			JSValue kids = JS_GetPropertyStr(ctx, comp_data, "kids");
			absl::Cleanup _ = [&]() {
				JS_FreeValue(ctx, tag);
				JS_FreeValue(ctx, atts);
				JS_FreeValue(ctx, kids);
				};
			if (JS_IsString(tag) && JS_IsObject(atts) && JS_IsArray(ctx, kids)) {
				std::string tagName = script_ctx_->parse<std::string>(tag);
				if (tagName == "style") {
					return NodeCompareResult::PatchableStyleSheet;
				} else {
					if (node->type_ == NodeType::NODE_ELEMENT && node->tag_ == base::string_intern(tagName)) {
						return NodeCompareResult::PatchableElementNode;
					}
				}
			}
		}
	}
	return NodeCompareResult::Unpatchable;
}

void Scene::updateTextNode(Node* node, JSContext* ctx, JSValue comp_data)
{
	node->text_ = script::parse<std::string>(ctx, comp_data);
}

void Scene::updateElementNode(Node* node, JSContext* ctx, JSValue comp_data)
{
	JSValue atts = JS_GetPropertyStr(ctx, comp_data, "atts");
	JSValue kids = JS_GetPropertyStr(ctx, comp_data, "kids");
	absl::Cleanup _ = [&]() {
		JS_FreeValue(ctx, atts);
		JS_FreeValue(ctx, kids);
		};
	setupProps(node, atts);
	updateNodeChildren(node, ctx, kids);
}

void Scene::updateComponentNode(Node* node, JSContext* ctx, JSValue comp_data)
{
	JS_SetPropertyStr(ctx, node->comp_state_, "props", JS_GetPropertyStr(ctx, comp_data, "props"));
	JS_SetPropertyStr(ctx, node->comp_state_, "children", JS_GetPropertyStr(ctx, comp_data, "children"));
	JSValue render = JS_GetPropertyStr(ctx, node->comp_state_, "render");
	JSValue child_comp_data = JS_Call(ctx, render, node->comp_state_, 0, nullptr);
	updateComponentNodeChildren(node, child_comp_data);
	JS_FreeValue(ctx, render);
	JS_FreeValue(ctx, child_comp_data);
}

void Scene::setStyleSheet(Node* node, JSValue stylesheet)
{
	if (!node)
		return setStyleSheet(stylesheet);
	node->style_rules_ = makeStyleRules(script_ctx_->get(), stylesheet);
}

} // namespace scene2d

