#pragma once

#include <string>
#include <memory>
#include "base/base.h"
#include "quickjs.h"
#include "geom_types.h"
#include "style/StyleRule.h"
#include "style/StylePaint.h"
#include "absl/functional/function_ref.h"

namespace script {
class Context;
}
namespace graph2d {
class PainterInterface;
}

namespace scene2d {

class Node;
class Scene : public base::Object {
public:
	Scene();
	~Scene();
	inline base::WeakObjectProxy<Scene> *weakProxy() const
	{
		return weakptr_;
	}
	inline base::object_weakptr<Scene> weaken() const
	{
		return base::object_weakptr<Scene>(weakptr_);
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
	
	void appendStyleRule(std::unique_ptr<style::StyleRule>&& rule);
	void resolveStyle();
	void computeLayout(const scene2d::DimensionF &size);

	void paint(graph2d::PainterInterface* painter);

private:
	Node* createComponentNodeWithState(JSValue comp_data);
    Node* pickSelf(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos);
	void setupProps(Node* node, JSValue props);
	bool match(Node* node, style::Selector* selector);
	void resolveNodeStyle(Node* node);
	void paintNode(Node* node, style::BlockPaintContext& bpc, graph2d::PainterInterface* painter);

	std::unique_ptr<script::Context> ctx_;
	Node* root_;
	base::WeakObjectProxy<Scene> *weakptr_;
	std::vector<std::unique_ptr<style::StyleRule>> style_rules_;

	friend class Node;
};
}
