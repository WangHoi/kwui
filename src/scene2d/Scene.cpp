#include "Scene.h"
#include "Node.h"
#include "script/script.h"

namespace scene2d {

Scene::Scene()
{
	ctx_ = std::make_unique<script::Context>();
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
	auto actor = new Node(NodeType::NODE_TEXT, text);
	actor->retain();
	return actor;
}

Node* Scene::createElementNode(base::string_atom tag)
{
	auto actor = new Node(NodeType::NODE_ELEMENT, tag);
	actor->retain();
	return actor;
}

Node* Scene::createComponentNode(JSValue comp_data)
{
	JSContext* jctx = ctx_->get();
	if (JS_IsString(comp_data)) {
		std::string text = ctx_->parse<std::string>(comp_data);
		LOG(INFO) << "createTextNode: " << text;
		return createTextNode(text);
	} else if (JS_IsArray(jctx, comp_data)) {
		LOG(INFO) << "createElementNode: " << "fragment";
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
		if (JS_IsFunction(jctx, render)) {
			LOG(INFO) << "createComponentNode";
			Node* node = createComponentNodeWithState(comp_data);
			JS_SetOpaque(comp_data, node->weakProxy());

			JSValue child_comp_data = JS_Call(jctx, render, comp_data, 0, nullptr);
			node->appendChild(createComponentNode(child_comp_data));
			return node;
		} else {
			JSValue tag = JS_GetPropertyStr(jctx, comp_data, "tag");
			JSValue atts = JS_GetPropertyStr(jctx, comp_data, "atts");
			JSValue kids = JS_GetPropertyStr(jctx, comp_data, "kids");
			if (JS_IsString(tag) && JS_IsObject(atts) && JS_IsArray(jctx, kids)) {
				std::string tagName = ctx_->parse<std::string>(tag);
				LOG(INFO) << "createElementNode: " << tagName;
				Node* node = createElementNode(base::string_intern(tagName));

				// Setup properties
				setupProps(node, atts);

				// Create children
				int64_t length = 0;
				JS_GetPropertyLength(jctx, &length, kids);
				for (uint32_t i = 0; i < (uint32_t)length; ++i) {
					JSValue child_comp_data = JS_GetPropertyUint32(jctx, kids, i);
					Node* child = createComponentNode(child_comp_data);
					node->appendChild(child);
				}

				return node;
			} else {
				auto a = JS_JSONStringify(jctx, comp_data, JS_UNDEFINED, JS_UNDEFINED);
				LOG(WARNING) << "Unknown: " << ctx_->parse<std::string>(a);
				return nullptr;
			}
		}
	}
	LOG(WARNING) << "Unknown: " << ctx_->parse<std::string>(comp_data);
	return nullptr;
}

void Scene::updateComponent(JSValue comp_state)
{

}

Node* Scene::pickNode(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos/* = nullptr */)
{
	if (!node->visible()) {
		return nullptr;
	}
	scene2d::PointF local_pos = pos - node->origin();
	const auto& children = node->children_;
	for (auto it = children.rbegin(); it != children.rend(); ++it) {
		Node* node = pickNode(*it, local_pos, flag_mask, out_local_pos);
		if (node)
			return node;
	}
	return pickSelf(node, pos, flag_mask, out_local_pos);
}

void Scene::resolveStyle(Node* node)
{
	node->resolveStyle();
	for (auto child : node->children())
		resolveStyle(child);
}

void Scene::computeLayout(Node* node)
{
	node->computeLayout();
	for (auto child : node->children())
		computeLayout(child);
}

Node* Scene::pickSelf(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos)
{
	if (!node->visible())
		return nullptr;
	if (node->testFlags(flag_mask) && node->hitTestNode(pos)) {
		if (out_local_pos)
			*out_local_pos = pos - node->origin();
		return node;
	} else {
		return nullptr;
	}

}

Node* Scene::createComponentNodeWithState(JSValue comp_state)
{
	return new Node(NodeType::NODE_COMPONENT, comp_state);
}

void Scene::setupProps(Node* node, JSValue props)
{
	ctx_->eachObjectField(props, [&](base::string_atom name, JSValue value) {
		JSContext* jctx = ctx_->get();
		if (name == base::string_intern("style")) {
			node->setStyle(ctx_->parse<style::StyleSpec>(value));
		} else if (JS_IsFunction(ctx_->get(), value)) {
			node->setEventHandler(name, JS_DupValue(jctx, value));
		} else if (JS_IsNumber(value)) {
			double f64;
			JS_ToFloat64(jctx, &f64, value);
			node->setAttribute(name, (float)f64);
		} else if (JS_IsString(value)) {
			const char *s = JS_ToCString(jctx, value);
			node->setAttribute(name, std::string(s));
			JS_FreeCString(jctx, s);
		} else {
			JSValue jval = JS_ToString(jctx, value);
			const char *s = JS_ToCString(jctx, jval);
			node->setAttribute(name, std::string(s));
			JS_FreeCString(jctx, s);
			JS_FreeValue(jctx, jval);
		}
	});
}

}