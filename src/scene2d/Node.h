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

namespace android {
class DialogAndroid;
}
namespace windows {
class DialogWin32;
namespace graphics {
class Painter;
}
}
namespace script {
class Value;
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
	NODE_STATE_CHECKED = 8,
	NODE_STATE_DISABLED = 16,
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
struct StyleResolveContext {
	std::unordered_set<std::string> key_set;
	inline bool testAndUpdate(const std::string& key, bool important) {
		bool found = (key_set.find(key) != key_set.end());
		if (!important) {
			return !found;
		} else {
			if (!found) {
				key_set.insert(key);
			}
			return true;
		}
	}
};
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
		return parent_ ? parent_.get() : nullptr;
	}
	Node* parentElement() const;
	void appendChild(Node* child);
	// keep return child's reference 
	Node* removeChildAt(size_t idx);
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
		if (parent())
			return parent()->visibleInHierarchy();
		return true;
	}
	inline base::string_atom elementTag() const
	{
		return tag_;
	}
	inline JSValue componentState() const
	{
		return comp_state_;
	}

	// pos: related to padding box
	bool hitTest(const PointF& pos, int flags) const;

	void onEvent(Event& event);
	void onAnimationFrame(absl::Time timestamp);

	void setId(base::string_atom id);
	void setClass(const style::Classes &klass);
	void setStyle(const style::StyleSpec &style);
	void setAttribute(base::string_atom name, const NodeAttributeValue &value);
	void setEventHandler(base::string_atom name, const script::Value& func);
	
	void resolveDefaultStyle();
	void resolveStyle(StyleResolveContext& ctx, const style::StyleSpec &style);
	void resolveInlineStyle(StyleResolveContext& ctx);
	
	inline const style::Style& computedStyle() const
	{
		return computed_style_;
	}
	bool matchSimple(style::Selector *selector) const;
	void computeLayout();

	inline style::LayoutObject& layoutObject()
	{
		return layout_;
	}

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
	void onMouseEvent(MouseEvent& event);
	void onKeyEvent(KeyEvent& event);
	void onFocusEvent(FocusEvent& event);
	void onImeEvent(ImeEvent& event);

	// Tree nodes
	Scene* scene_ = nullptr;

	base::object_weakptr<Node> parent_ = nullptr;
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
	std::map<base::string_atom, script::Value> event_handlers_;

	// Control
	std::unique_ptr<Control> control_;

	// Style and layout
	style::StyleSpec specStyle_;
	style::Style computed_style_;

	bool visible_ = true;
	struct ScrollData {
		PointF offset;
		absl::optional<style::ScrollObject::SubControl> hover_sub_control;
		absl::optional<style::ScrollObject::SubControl> active_sub_control;
		absl::optional<std::pair<PointF, PointF>> active_pos; // [event_pos, scroll_offset]
	} scroll_data_;

	friend class Scene;
	friend class android::DialogAndroid;
	friend class windows::DialogWin32;
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
