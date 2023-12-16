#pragma once

#include <string>
#include <memory>
#include "base/base.h"
#include "quickjs.h"
#include "geom_types.h"
#include "style/StyleRule.h"
#include "style/StylePaint.h"
#include "Control.h"
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
	Scene(EventContext& ctx);
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
	void updateComponentNode(Node* node, JSValue comp_state);

	inline Node *root() const
	{
		return root_;
	}
    Node* pickNode(const PointF& pos, int flag_mask, PointF* out_local_pos = nullptr);
	
	void appendStyleRule(std::unique_ptr<style::StyleRule>&& rule);
	void resolveStyle();
	void computeLayout(const scene2d::DimensionF &size);

	void paint(graph2d::PainterInterface* painter);

	scene2d::PointF getMousePosition() const;
	void requestPaint();
	void requestUpdate();
	void requestAnimationFrame(scene2d::Node* node);
	// map from control coordinate to scene
	PointF mapPointToScene(Node* node, const PointF& pos) const;

	script::Context& scriptContext() const
	{
		return *script_ctx_;
	}

private:
	Node* createComponentNodeWithState(JSValue comp_data);
	void setupProps(Node* node, JSValue props);
	bool match(Node* node, style::Selector* selector);
	void resolveNodeStyle(Node* node);
	void paintNode(Node* node, graph2d::PainterInterface* painter);
	void layoutComputed(Node* node);

	EventContext& event_ctx_;
	std::unique_ptr<script::Context> script_ctx_;
	Node* root_;
	base::WeakObjectProxy<Scene> *weakptr_;
	std::vector<std::unique_ptr<style::StyleRule>> style_rules_;
	std::vector<style::FlowRoot> flow_roots_;

	friend class Node;
};
}
