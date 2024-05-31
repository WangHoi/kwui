#pragma once

#include <string>
#include <memory>
#include "base/base.h"
#include "quickjs.h"
#include "geom_types.h"
#include "style/StyleRule.h"
#include "style/StylePaint.h"
#include "script/Value.h"
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
	Node* createComponentNode(Node* parent, JSValue comp_data);
	void updateComponentNodeChildren(Node* node, JSValue comp_state);

	void setScriptModule(const std::string& base_filename,
		const std::string& module_path,
		const script::Value& module_params);
	void reloadScriptModule();
	void setStyleSheet(JSValue stylesheet);

	inline Node *root() const
	{
		return root_;
	}
    Node* pickNode(const PointF& pos, int flag_mask, PointF* out_local_pos = nullptr);
	
	void appendStyleRule(std::unique_ptr<style::StyleRule>&& rule);
	void resolveStyle();
	void computeLayout(float width, absl::optional<float> height);

	void paint(graph2d::PainterInterface* painter);

	scene2d::PointF getMousePosition() const;
	void requestPaint();
	void requestUpdate();
	void requestAnimationFrame(scene2d::Node* node);
	// map from control coordinate to scene
	// FIXME: temp solution: include_flow_root when query ime caret_rect
	PointF mapPointToScene(Node* node, const PointF& pos, bool include_flow_root = false) const;

	script::Context& scriptContext() const
	{
		return *script_ctx_;
	}
	std::string eventContextId() const;

	base::TaskQueue::TaskId addPostRenderTask(std::function<void()>&& fn);
	bool removePostRenderTask(base::TaskQueue::TaskId id);
	void runPostRenderTasks();

	void dispatchEvent(Node* node, Event& event, bool bubble);

	std::tuple<float, float> intrinsicWidth();
	float intrinsicHeight(float width);

private:
	enum class NodeCompareResult {
		Unpatchable,
		PatchableTextNode,
		PatchableElementNode,
		PatchableFragmentElementNode,
		PatchableComponentNode,
	};
	void setupProps(Node* node, JSValue props);
	bool match(Node* node, style::Selector* selector);
	void resolveNodeStyle(Node* node);
	void paintNode(Node* node, graph2d::PainterInterface* painter);
	void layoutComputed(Node* node);

	void updateNodeChildren(Node* node, JSContext* ctx, JSValue comp_data);
	NodeCompareResult compareNode(Node* node, JSContext* ctx, JSValue comp_data);
	void updateTextNode(Node* node, JSContext* ctx, JSValue comp_data);
	void updateElementNode(Node* node, JSContext* ctx, JSValue comp_data);
	void updateComponentNode(Node* node, JSContext* ctx, JSValue comp_data);

	EventContext& event_ctx_;
	std::unique_ptr<script::Context> script_ctx_;
	struct ModuleInfo {
		std::string base_filename;
		std::string module_path;
		script::Value module_params;
	};
	absl::optional<ModuleInfo> script_module_;
	base::TaskQueue post_render_task_queue_;
	Node* root_;
	base::WeakObjectProxy<Scene> *weakptr_;
	std::vector<std::unique_ptr<style::StyleRule>> style_rules_;
	std::vector<style::FlowRoot> flow_roots_;

	friend class Node;
};
}
