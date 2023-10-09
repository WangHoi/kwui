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
#include "style/Layout.h"

namespace windows {
class Dialog;
namespace graphics {
class Painter;
}
}
namespace graph2d {
class TextLayoutInterface;
}
namespace style {
class BlockWidthSolverInterface;
}

namespace scene2d {

enum class NodeType {
	NODE_TEXT = 1,
	NODE_ELEMENT = 2,
	NODE_COMPONENT = 3,
};

enum NodeFlag {
	NODE_FLAG_FOCUSABLE = 1,
	NODE_FLAG_HOVERABLE = 2,
	NODE_FLAG_CLICKABLE = 4,
};

class Control;
class Scene;
class BlockWidthSolverInterface;
class Node : public base::Object {
public:
	Node(NodeType type);
	Node(NodeType type, const std::string& text);
	Node(NodeType type, base::string_atom tag);
	Node(NodeType type, JSValue comp_state);
	~Node();

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

	inline const PointF& origin() const
	{
		return origin_;
	}

	inline const DimensionF& size() const
	{
		return size_;
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

	bool testFlags(int flags) const;

	inline bool hitTestNode(const PointF& p) {
		return origin_.x <= p.x && p.x < origin_.x + size_.width
			&& origin_.y <= p.y && p.y < origin_.y + size_.height;
	}

	void onEvent(MouseEvent &event);
	void onEvent(KeyEvent &event);
	void onEvent(FocusEvent &event);
	void onEvent(ImeEvent &event);
	
	void setId(base::string_atom id);
	void setClass(const style::Classes &klass);
	void setStyle(const style::StyleSpec &style);
	void setAttribute(base::string_atom name, const NodeAttributeValue &value);
	void setEventHandler(base::string_atom name, JSValue func);
	void paintControl(windows::graphics::Painter &painter);
	void resolveStyle(const style::StyleSpec &style);
	inline void resolveInlineStyle()
	{
		resolveStyle(specStyle_);
	}
	inline const style::Style& computedStyle() const
	{
		return computed_style_;
	}
	bool matchSimple(style::Selector *selector) const;
	void computeLayout();

	// layout with new BFC
	void layoutBlockElement(float contg_blk_width, absl::optional<float> contg_blk_height);

	void layoutText(style::InlineFormatContext& ifc);
	// layout self and children
	void layoutInlineElement(style::InlineFormatContext& ifc, int element_depth);

protected:
	static void layoutInlineChild(Node* node, style::InlineFormatContext& ifc, int element_depth);
	static void assembleInlineChild(Node* child, std::vector<style::InlineBox>& box);
	bool anyBlockChildren() const;
	void layoutBlockChild(style::BlockFormatContext& bfc, style::BlockBox& box, Node* child, int element_depth);
	std::unique_ptr<style::BlockWidthSolverInterface> createBlockWidthSolver(style::BlockFormatContext& bfc);
	// in-flow layout
	static void layoutMeasure(style::BlockFormatContext& bfc, style::BlockBoxBuilder& bbb, Node* node);
	// in-flow placing
	static void layoutArrange(style::BlockFormatContext& bfc, style::BlockBox& box);
	template<typename F>
	inline void eachChild(F&& f)
	{
		return Node::eachChild(this, std::move(f));
	}
	template<typename F>
	static void eachChild(Node* node, F&& f);
	template<typename F>
	inline void eachLayoutChild(F&& f)
	{
		return Node::eachLayoutChild(this, std::move(f));
	}
	template<typename F>
	static void eachLayoutChild(Node* node, F&& f);

	// Tree nodes
	Scene* stage_ = nullptr;

	Node* parent_ = nullptr;
	int child_index = -1;
	std::vector<Node*> children_;

	NodeType type_;

	// Text
	std::string text_;
	std::unique_ptr<graph2d::TextLayoutInterface> text_layout_;
	style::InlineBox text_box_;

	// Element
	base::string_atom tag_;
	base::string_atom id_;
	style::Classes klass_;
	std::vector<style::InlineBox> inline_boxes_; // inline-level layout
	style::BlockBox block_box_; // block-level layout
	absl::optional<style::BlockFormatContext> bfc_;
	absl::optional<style::InlineFormatContext> ifc_;

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
	absl::optional<style::BoxF> layoutBox_;
	PointF origin_;
	DimensionF size_;

	bool visible_ = true;

	friend class Scene;
	friend class windows::Dialog;
};

template<typename F>
void Node::eachChild(Node* node, F&& f)
{
	for (Node* child : node->children()) {
		f(child);
	}
}

template<typename F>
void Node::eachLayoutChild(Node* node, F&& f)
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
