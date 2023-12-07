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

Scene::Scene(EventContext& ctx)
	: event_ctx_(ctx)
{
	script_ctx_ = std::make_unique<script::Context>();
	root_ = createElementNode(base::string_intern("kml"));
	root_->retain();
	weakptr_ = new base::WeakObjectProxy<Scene>(this);
	weakptr_->retain();
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

Node* Scene::createComponentNode(JSValue comp_data)
{
	JSContext* jctx = script_ctx_->get();
	if (JS_IsString(comp_data)) {
		std::string text = script_ctx_->parse<std::string>(comp_data);
		//LOG(INFO) << "createTextNode: " << text;
		return createTextNode(text);
	} else if (JS_IsArray(jctx, comp_data)) {
		//LOG(INFO) << "createElementNode: " << "fragment";
		Node* node = createElementNode(base::string_intern("fragment"));
		int64_t length = 0;
		JS_GetPropertyLength(jctx, &length, comp_data);
		for (uint32_t i = 0; i < (uint32_t)length; ++i) {
			JSValue child_comp_data = JS_GetPropertyUint32(jctx, comp_data, i);
			Node* child = createComponentNode(child_comp_data);
			node->appendChild(child);
		}
		return node;
	} else if (JS_IsObject(comp_data)) {
		JSValue render = JS_GetPropertyStr(jctx, comp_data, "render");
		absl::Cleanup _ = [&]() { JS_FreeValue(jctx, render); };
		if (JS_IsFunction(jctx, render)) {
			//LOG(INFO) << "createComponentNode";
			Node* node = createComponentNodeWithState(comp_data);
			JS_SetOpaque(comp_data, node->weakProxy());

			JSValue child_comp_data = JS_Call(jctx, render, comp_data, 0, nullptr);
			node->appendChild(createComponentNode(child_comp_data));
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
				Node* node = createElementNode(base::string_intern(tagName));

				// Setup properties
				setupProps(node, atts);

				// Create children
				int64_t length = 0;
				JS_GetPropertyLength(jctx, &length, kids);
				for (uint32_t i = 0; i < (uint32_t)length; ++i) {
					JSValue child_comp_data = JS_GetPropertyUint32(jctx, kids, i);
					Node* child = createComponentNode(child_comp_data);
					JS_FreeValue(jctx, child_comp_data);

					node->appendChild(child);
				}

				return node;
			} else {
				auto a = JS_JSONStringify(jctx, comp_data, JS_UNDEFINED, JS_UNDEFINED);
				LOG(WARNING) << "Unknown: " << script_ctx_->parse<std::string>(a);
				JS_FreeValue(jctx, a);
				return nullptr;
			}
		}
	}
	LOG(WARNING) << "Unknown: " << script_ctx_->parse<std::string>(comp_data);
	return nullptr;
}

void Scene::updateComponent(JSValue comp_state)
{

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
	resolveNodeStyle(root_);
}

void Scene::computeLayout(const scene2d::DimensionF& size)
{
	style::LayoutTreeBuilder b(root_);
	flow_roots_ = b.build();

	for (auto& flow_root : flow_roots_) {
		style::LayoutObject::reflow(flow_root, size);
	}

	// trigger Control::onLayout
	updateControlLayout(root_);
}

void Scene::paint(graph2d::PainterInterface* painter)
{
	for (auto& fl : flow_roots_) {
		auto scene_pos = mapPointToScene(fl.root->node, PointF());
		//LOG(INFO) << "Paint flow root " << *fl.root << ", scene_pos=" << scene_pos;
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

PointF Scene::mapPointToScene(Node* node, const PointF& pos) const
{
	// LOG(INFO) << "Scene::mapPointToScene";
	style::LayoutObject* o = &node->layout_;
	scene2d::PointF p = pos;
	while (o->parent) {
		p += style::LayoutObject::contentRect(o).origin();
		p += style::LayoutObject::pos(o);
		if (o->scroll_object.has_value()) {
			p -= o->scroll_object.value().viewport_rect.origin();
		}
		o = o->parent;
	}
	if (o->scroll_object.has_value()) {
		p -= o->scroll_object.value().viewport_rect.origin();
	}

	// TODO: handle scroll in positioned ascendants
	auto it = std::find_if(flow_roots_.begin(), flow_roots_.end(), [&](const style::FlowRoot& fl) {
		return fl.root == o;
		});
	if (it != flow_roots_.end()) {
		const style::FlowRoot& fl = *it;
		if (it->positioned_parent)
			p += mapPointToScene(it->positioned_parent->node, PointF());

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

Node* Scene::createComponentNodeWithState(JSValue comp_state)
{
	return new Node(this, NodeType::NODE_COMPONENT, comp_state);
}

void Scene::setupProps(Node* node, JSValue props)
{
	script_ctx_->eachObjectField(props, [&](const char* name_str, JSValue value) {
		JSContext* jctx = script_ctx_->get();
		auto name = base::string_intern(name_str);
		if (name == base::string_intern("style")) {
			if (JS_IsObject(value)) {
				node->setStyle(script_ctx_->parse<style::StyleSpec>(value));
			} else if (JS_IsString(value)) {
				const char* s = JS_ToCString(jctx, value);
				auto style_res = style::parse_inline_style(s);
				JS_FreeCString(jctx, s);
				if (style_res.ok())
					node->setStyle(style_res.value());
			}
		} else if (name == base::string_intern("id")) {
			node->setId(script_ctx_->parse<base::string_atom>(value));
		} else if (name == base::string_intern("class")) {
			const char* s = JS_ToCString(jctx, value);
			if (s) {
				node->setClass(style::Classes::parse(s));
			}
			JS_FreeCString(jctx, s);
		} else if (JS_IsFunction(script_ctx_->get(), value)) {
			node->setEventHandler(name, JS_DupValue(jctx, value));
		} else if (JS_IsNumber(value)) {
			double f64;
			JS_ToFloat64(jctx, &f64, value);
			node->setAttribute(name, (float)f64);
		} else if (JS_IsString(value)) {
			const char* s = JS_ToCString(jctx, value);
			node->setAttribute(name, std::string(s));
			JS_FreeCString(jctx, s);
		} else {
			JSValue jval = JS_ToString(jctx, value);
			const char* s = JS_ToCString(jctx, jval);
			node->setAttribute(name, std::string(s));
			JS_FreeCString(jctx, s);
			JS_FreeValue(jctx, jval);
		}
		});
}

bool Scene::match(Node* node, style::Selector* selector)
{
	if (!node)
		return false;

	if (!node->matchSimple(selector))
		return false;

	if (!selector->dep_selector)
		return true;

	if (selector->dep_type == style::SelectorDependency::DirectParent)
		return match(node->parent_, selector->dep_selector.get());

	if (selector->dep_type == style::SelectorDependency::Ancestor) {
		Node* pnode = node->parent_;
		style::Selector* psel = selector->dep_selector.get();
		while (pnode) {
			if (match(pnode, psel))
				return true;
			pnode = pnode->parent_;
		}
		return false;
	}

	return true;
}

void Scene::resolveNodeStyle(Node* node)
{
	if (node->type() == NodeType::NODE_COMPONENT) {
		node->computed_style_ = node->parent_->computed_style_;
		node->eachChild(absl::bind_front(&Scene::resolveNodeStyle, this));
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		node->resolveDefaultStyle();
		for (auto& rule : style_rules_) {
			if (match(node, rule->selector.get())) {
				node->resolveStyle(rule->spec);
			}
		}
		node->resolveInlineStyle();
		node->eachChild(absl::bind_front(&Scene::resolveNodeStyle, this));
	} else if (node->type() == NodeType::NODE_TEXT) {
		node->resolveDefaultStyle();
		node->updateTextLayout();
	}
}

void Scene::paintNode(Node* node, graph2d::PainterInterface* painter)
{
	style::LayoutObject::paint(&node->layout_, painter);
}

void Scene::updateControlLayout(Node* node)
{
	node->updateControlLayout();
	Node::eachLayoutChild(node, absl::bind_front(&Scene::updateControlLayout, this));
}

} // namespace scene2d
