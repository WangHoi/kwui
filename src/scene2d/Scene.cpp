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
		return createTextNode(text);
	} else {
		JSValue* obj = nullptr;
		uint32_t count = 0;
		if (!JS_GetFastArray(jctx, comp_data, &obj, &count))
			return nullptr;
		if (count != 3)
			return nullptr;

		if (JS_IsString(obj[0])) {
			std::string tagName = ctx_->parse<std::string>(comp_data);
			Node* node = createElementNode(base::string_intern(tagName));
			
			// Setup properties
			if (JS_IsObject(obj[1])) {
			}

			// Create children
			if (JS_IsArray(jctx, obj[2])) {
				int64_t length = 0;
				JS_GetPropertyLength(jctx, &length, obj[2]);
				for (uint32_t i = 0; i < (uint32_t)length; ++i) {
					JSValue child_comp_data = JS_GetPropertyUint32(jctx, obj[2], i);
					Node* child = createComponentNode(child_comp_data);
					node->appendChild(child);
				}
			}

			return node;
		} else if (JS_IsFunction(jctx, obj[0])) {
			JSValue global = JS_GetGlobalObject(jctx);
			JSValue comp_state_ctor = JS_GetPropertyStr(jctx, global, "__ComponentState__");
			JSValue comp_state = JS_CallConstructor(jctx, comp_state_ctor, 1, &comp_data);
			Node* node = createComponentNodeWithState(comp_state);
			JS_SetOpaque(comp_state, node->weakObject());

			// Setup properties
			if (JS_IsObject(obj[1])) {
			}

			// Create children
			JSValue render = JS_GetPropertyStr(jctx, comp_state, "render");
			JSValue child_comp_data = JS_Call(jctx, render, comp_state, 0, nullptr);
			node->appendChild(createComponentNode(child_comp_data));
			JS_FreeValue(jctx, global);

			return node;
		} else {
			return nullptr;
		}
	}
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

}