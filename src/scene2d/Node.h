#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <map>

#include "base/base.h"
#include "style/style.h"
#include "script/script.h"
#include "geom_types.h"
#include "Attribute.h"
#include "Event.h"
#include "style/LayoutObject.h"

namespace windows {
class Dialog;
namespace graphics {
class Painter;
}
}
namespace style {
class BlockWidthSolverInterface;
class InlineBoxBuilder;
class LayoutTreeBuilder;
class LayoutObject;
}

namespace scene2d {

enum class NodeType {
	NODE_TEXT = 1,
	NODE_ELEMENT = 2,
	NODE_COMPONENT = 3,
};

enum NodeState {
	NODE_STATE_HOVER = 1,
	NODE_STATE_ACTIVE = 2,
	NODE_STATE_FOCUSED = 4,
};

template <typename Sink>
void AbslStringify(Sink& sink, NodeType p) {
	switch (p) {
	case NodeType::NODE_TEXT:
		absl::Format(&sink, "NODE_TEXT");
		break;
	case NodeType::NODE_ELEMENT:
		absl::Format(&sink, "NODE_ELEMENT");
		break;
	case NodeType::NODE_COMPONENT:
		absl::Format(&sink, "NODE_COMPONENT");
		break;
	default:
		absl::Format(&sink, "NODE_CUSTOM(%d)", (int)p);
	}
}

enum NodeFlag {
	NODE_FLAG_FOCUSABLE = 1,
	NODE_FLAG_HOVERABLE = 2,
	NODE_FLAG_CLICKABLE = 4,
	NODE_FLAG_SCROLLABLE = 8,
};

class Control;
class Scene;
class BlockWidthSolverInterface;
class Node : public base::Object {
public:
	Node(Scene* scene, NodeType type);
	Node(Scene* scene, NodeType type, const std::string& text);
	Node(Scene* scene, NodeType type, base::string_atom tag);
	Node(Scene* scene, NodeType type, JSValue comp_state);
	~Node();

	Scene* scene() const
	{
		return scene_;
	}

	NodeType type() const
	{
		return type_;
	}

	inline Node* parent() const
	{
		return parent_;
	}
	void appendChild(Node* child);
	inline const std::vector<Node*>& children() const
	{
		return children_;
	}

	inline base::WeakObjectProxy<Node>* weakProxy() const
	{
		return weakptr_;
	}
	inline base::object_weakptr<Node> weaken() const
	{
		return base::object_weakptr<Node>(weakptr_);
	}

	inline bool visible() const
	{
		return visible_;
	}
	bool visibleInHierarchy() const
	{
		if (!visible_)
			return false;
		if (parent_)
			return parent_->visibleInHierarchy();
		return true;
	}

	// pos: related to padding box
	bool hitTest(const PointF& pos, int flags) const;

	void onEvent(MouseEvent &event);
	void onEvent(KeyEvent &event);
	void onEvent(FocusEvent &event);
	void onEvent(ImeEvent &event);
	void onAnimationFrame(absl::Time timestamp);

	void setId(base::string_atom id);
	void setClass(const style::Classes &klass);
	void setStyle(const style::StyleSpec &style);
	void setAttribute(base::string_atom name, const NodeAttributeValue &value);
	void setEventHandler(base::string_atom name, JSValue func);
	
	void resolveDefaultStyle();
	void resolveStyle(const style::StyleSpec &style);
	void resolveInlineStyle();
	
	inline const style::Style& computedStyle() const
	{
		return computed_style_;
	}
	bool matchSimple(style::Selector *selector) const;
	void computeLayout();

	bool positioned() const;
	bool relativePositioned() const;
	bool absolutelyPositioned() const;
	Node* positionedAncestor() const;

	scene2d::PointF getMousePosition() const;
	void requestPaint();
	void requestUpdate();
	void requestAnimationFrame(scene2d::Node* node);

	void updateTextLayout();
	void layoutComputed();

	template<typename F>
	inline void eachChild(F&& f)
	{
		return Node::eachChild(this, std::move(f));
	}
	template<typename F>
	static void eachChild(Node* node, F f);
	template<typename F>
	inline void eachLayoutChild(F&& f)
	{
		return Node::eachLayoutChild(this, std::move(f));
	}
	template<typename F>
	static void eachLayoutChild(Node* node, F f);

protected:
	bool matchPseudoClasses(const style::PseudoClasses& pseudo_classes) const;
	void handleScrollEvent(scene2d::MouseEvent& event);

	// Tree nodes
	Scene* scene_ = nullptr;

	Node* parent_ = nullptr;
	int child_index = -1;
	std::vector<Node*> children_;

	NodeType type_;
	uint8_t state_ = 0;

	// Text
	std::string text_;
	std::unique_ptr<graph2d::TextFlowInterface> text_flow_;
	style::GlyphRunBoxes text_boxes_; // Formatted text boxes

	// Element
	base::string_atom tag_;
	base::string_atom id_;
	style::Classes klass_;
	style::LayoutObject layout_;

	// Component
	JSValue comp_state_ = JS_UNINITIALIZED;
	base::WeakObjectProxy<Node>* weakptr_ = nullptr;

	// Attribute and Event handlers
	std::map<base::string_atom, NodeAttributeValue> attrs_;
	std::map<base::string_atom, JSValue> event_handlers_;

	// Control
	std::unique_ptr<Control> control_;

	// Style and layout
	style::StyleSpec specStyle_;
	style::Style computed_style_;

	bool visible_ = true;
	struct ScrollData {
		PointF offset;
		absl::optional<PointF> mouse_down_v_scrollbar;
		absl::optional<PointF> mouse_down_h_scrollbar;
	} scroll_data_;

	friend class Scene;
	friend class windows::Dialog;
	friend class style::InlineBoxBuilder;
	friend class style::LayoutTreeBuilder;
	friend class style::LayoutObject;
};

template<typename F>
void Node::eachChild(Node* node, F f)
{
	for (Node* child : node->children()) {
		f(child);
	}
}

template<typename F>
void Node::eachLayoutChild(Node* node, F f)
{
	for (Node* child : node->children()) {
		if (child->type() == NodeType::NODE_TEXT) {
			f(child);
		} else if (child->type() == NodeType::NODE_ELEMENT) {
			f(child);
		} else if (child->type() == NodeType::NODE_COMPONENT) {
			eachLayoutChild(child, std::move(f));
		}
	}
}

}
