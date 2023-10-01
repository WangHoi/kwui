#pragma once

#include <string>
#include <memory>
#include "base/base.h"
#include "quickjs.h"
#include "geom_types.h"

namespace script {
class Context;
}

namespace scene2d {

class Node;
class Scene : public base::Object {
public:
	Scene();
	~Scene();
	inline base::WeakObjectProxy<Scene> *weakObject() const
	{
		return weakptr_;
	}
	Node* createTextNode(const std::string &text);
	Node* createElementNode(base::string_atom tag);
	Node* createComponentNode(JSValue comp_data);
	void updateComponent(JSValue comp_state);

	inline Node *root() const
	{
		return root_;
	}
    inline Node* pickNode(const PointF& pos, int flag_mask, PointF* out_local_pos = nullptr)
	{
		return pickNode(root_, pos, flag_mask, out_local_pos);
	}
    Node* pickNode(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos = nullptr);

	void resolveStyle(Node* node);
	void computeLayout(Node* node);

private:
	Node* createComponentNodeWithState(JSValue comp_data);
    Node* pickSelf(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos);
	void setupProps(Node* node, JSValue props);

	std::unique_ptr<script::Context> ctx_;
	Node* root_;
	base::WeakObjectProxy<Scene> *weakptr_;
};
}
