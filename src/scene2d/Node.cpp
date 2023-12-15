#include "Node.h"
#include "graph2d/graph2d.h"
#include "style/style.h"
#include "control.h"
#include "Scene.h"
#include "style/BoxConstraintSolver.h"
#include "base/scoped_setter.h"
#include "style/BlockLayout.h"
#include "style/InlineLayout.h"
#include "absl/functional/bind_front.h"

namespace scene2d {

static const float SCROLLBAR_WIDTH = 16.0f;

static inline void resolve_style(style::Value& style,
	const style::Value* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		style = *spec.value;
	}
}
static void resolve_style(style::DisplayType& style, const style::DisplayType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::PositionType& style, const style::PositionType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::FontStyle& style, const style::FontStyle* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::FontWeight& style, const style::FontWeight* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::TextAlign& style, const style::TextAlign* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::OverflowType& style, const style::OverflowType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::BoxSizingType& style, const style::BoxSizingType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::Color& style, const style::Color* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::CursorType& style, const style::CursorType* parent,
	const style::ValueSpec& spec);

Node::Node(Scene* scene, NodeType type)
	: scene_(scene)
	, type_(type)
{
	weakptr_ = new base::WeakObjectProxy<Node>(this);
	weakptr_->retain();
	layout_.init(&computed_style_, this);
}

Node::Node(Scene* scene, NodeType type, const std::string& text)
	: Node(scene, type)
{
	text_ = text;
}

Node::Node(Scene* scene, NodeType type, base::string_atom tag)
	: Node(scene, type)
{
	tag_ = tag;
	control_.reset(ControlRegistry::get()->createControl(tag));
	if (control_)
		control_->onAttach(this);
}

Node::Node(Scene* scene, NodeType type, JSValue comp_state)
	: Node(scene, type)
{
	//comp_state_ = JS_DupValue(scene->script_ctx_->get(), comp_state);
	comp_state_ = comp_state;
	weakptr_ = new base::WeakObjectProxy<Node>(this);
	weakptr_->retain();
}

Node::~Node()
{
	if (control_) {
		control_->onDetach(this);
		control_ = nullptr;
	}
	if (weakptr_) {
		weakptr_->release();
		weakptr_ = nullptr;
	}
	for (Node* child : children_) {
		child->release();
	}
	children_.clear();
	if (comp_state_ != JS_UNINITIALIZED) {
		JS_FreeValue(scene_->script_ctx_->get(), comp_state_);
		comp_state_ = JS_UNINITIALIZED;
	}
}

void Node::appendChild(Node* child)
{
	child->retain();
	child->parent_ = this;
	child->child_index = (int)children_.size();
	children_.push_back(child);
}

bool Node::hitTest(const PointF &pos, int flags) const
{
	if (layout_.scroll_object.has_value()) {
		if (style::ScrollObject::hitTest(layout_.scroll_object.operator->(), pos, flags))
			return true;
	}
	if (control_)
		return control_->hitTest(pos, flags);
	return false;
}

void Node::onEvent(MouseEvent& event)
{
	if (layout_.scroll_object.has_value()) {
		handleScrollEvent(event);
	}
	if (control_)
		control_->onMouseEvent(this, event);
}

void Node::onEvent(KeyEvent& event)
{
	if (control_)
		control_->onKeyEvent(this, event);
}

void Node::onEvent(FocusEvent& event)
{
	if (control_)
		control_->onFocusEvent(this, event);
}

void Node::onEvent(ImeEvent& event)
{
	if (control_)
		control_->onImeEvent(this, event);
}

void Node::onAnimationFrame(absl::Time timestamp)
{
	if (control_)
		control_->onAnimationFrame(this, timestamp);
}

void Node::setId(base::string_atom id)
{
	id_ = id;
}

void Node::setClass(const style::Classes& klass)
{
	klass_ = klass;
}

void Node::setStyle(const style::StyleSpec& style)
{
	specStyle_ = style;
}

void Node::setAttribute(base::string_atom name, const NodeAttributeValue& value)
{
	attrs_[name] = value;
	if (control_)
		control_->onSetAttribute(name, value);
}

void Node::setEventHandler(base::string_atom name, JSValue func)
{
	event_handlers_[name] = func;
	if (control_)
		control_->onSetEventHandler(name, func);
}

void Node::resolveDefaultStyle()
{
	if (type_ == NodeType::NODE_TEXT) {
		computed_style_.display = style::DisplayType::Inline;
	} else {
		/* box-sizing: border-box is the default styling that browsers use for the <table>, <select>, and <button> elements,
		 * and for <input> elements whose type is radio, checkbox, reset, button, submit, color, or search
		 */
		if (tag_ == base::string_intern("img")
			|| tag_ == base::string_intern("span")
			|| tag_ == base::string_intern("strong")
			|| tag_ == base::string_intern("em")
			|| tag_ == base::string_intern("b")) {
			computed_style_.display = style::DisplayType::Inline;
		} else if (tag_ == base::string_intern("button")) {
			computed_style_.display = style::DisplayType::InlineBlock;
			computed_style_.box_sizing = style::BoxSizingType::BorderBox;
		} else {
			computed_style_.display = style::DisplayType::Block;
		}
	}
	computed_style_.position = style::PositionType::Static;

#define RESOLVE_STYLE_DEFAULT(x, def) \
    computed_style_.x = def

	RESOLVE_STYLE_DEFAULT(margin_left, style::Value::fromPixel(0));
	RESOLVE_STYLE_DEFAULT(margin_top, style::Value::fromPixel(0));
	RESOLVE_STYLE_DEFAULT(margin_right, style::Value::fromPixel(0));
	RESOLVE_STYLE_DEFAULT(margin_bottom, style::Value::fromPixel(0));
	RESOLVE_STYLE_DEFAULT(border_left_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_right_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_left_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_right_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_right_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_left_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_left, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_top, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_right, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_bottom, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(left, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(top, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(right, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(bottom, style::Value::auto_());

	RESOLVE_STYLE_DEFAULT(min_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(min_height, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(max_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(max_height, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(height, style::Value::auto_());

	RESOLVE_STYLE_DEFAULT(border_color, style::Color());
	RESOLVE_STYLE_DEFAULT(background_color, style::Color());

	RESOLVE_STYLE_DEFAULT(overflow_x, style::OverflowType::Visible);
	RESOLVE_STYLE_DEFAULT(overflow_y, style::OverflowType::Visible);
#undef RESOLVE_STYLE_DEFAULT

#define RESOLVE_STYLE_DEFAULT_INHERIT(x, def) \
    computed_style_.x = (parent_ ? parent_->computed_style_.x : def)
	
	RESOLVE_STYLE_DEFAULT_INHERIT(color, style::named_color::black);
	RESOLVE_STYLE_DEFAULT_INHERIT(line_height, style::Value::fromPixel(18));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_family, style::Value::fromKeyword(base::string_intern("Microsoft YaHei")));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_size, style::Value::fromPixel(12));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_style, style::FontStyle::Normal);
	RESOLVE_STYLE_DEFAULT_INHERIT(font_weight, style::FontWeight());
	RESOLVE_STYLE_DEFAULT_INHERIT(text_align, style::TextAlign::Left);
	RESOLVE_STYLE_DEFAULT_INHERIT(cursor, style::CursorType::Auto);
#undef RESOLVE_STYLE_DEFAULT_INHERIT
}

void Node::resolveStyle(const style::StyleSpec& spec)
{
	CHECK(type_ == NodeType::NODE_ELEMENT);
#define RESOLVE_STYLE(x) \
    resolve_style(computed_style_.x, \
        parent_ ? &parent_->computed_style_.x : nullptr, \
        spec.x)
	RESOLVE_STYLE(display);
	RESOLVE_STYLE(position);
	RESOLVE_STYLE(margin_left);
	RESOLVE_STYLE(margin_top);
	RESOLVE_STYLE(margin_right);
	RESOLVE_STYLE(margin_bottom);
	RESOLVE_STYLE(border_left_width);
	RESOLVE_STYLE(border_top_width);
	RESOLVE_STYLE(border_right_width);
	RESOLVE_STYLE(border_bottom_width);
	RESOLVE_STYLE(border_top_left_radius);
	RESOLVE_STYLE(border_top_right_radius);
	RESOLVE_STYLE(border_bottom_right_radius);
	RESOLVE_STYLE(border_bottom_left_radius);
	RESOLVE_STYLE(padding_left);
	RESOLVE_STYLE(padding_top);
	RESOLVE_STYLE(padding_right);
	RESOLVE_STYLE(padding_bottom);
	RESOLVE_STYLE(left);
	RESOLVE_STYLE(top);
	RESOLVE_STYLE(right);
	RESOLVE_STYLE(bottom);

	RESOLVE_STYLE(min_width);
	RESOLVE_STYLE(min_height);
	RESOLVE_STYLE(max_width);
	RESOLVE_STYLE(max_height);
	RESOLVE_STYLE(width);
	RESOLVE_STYLE(height);

	RESOLVE_STYLE(border_color);
	RESOLVE_STYLE(background_color);
	RESOLVE_STYLE(color);
	RESOLVE_STYLE(line_height);
	RESOLVE_STYLE(font_family);
	RESOLVE_STYLE(font_size);
	RESOLVE_STYLE(font_style);
	RESOLVE_STYLE(font_weight);
	RESOLVE_STYLE(text_align);
	RESOLVE_STYLE(overflow_x);
	RESOLVE_STYLE(overflow_y);
	RESOLVE_STYLE(box_sizing);
	RESOLVE_STYLE(cursor);
#undef RESOLVE_STYLE
}

void Node::resolveInlineStyle()
{
	resolveStyle(specStyle_);
	if (type_ == NodeType::NODE_ELEMENT && absolutelyPositioned()
		&& computed_style_.display != style::DisplayType::Block) {
		computed_style_.display = style::DisplayType::Block;
	}
}

bool Node::matchSimple(style::Selector* selector) const
{
	if (type_ != NodeType::NODE_ELEMENT)
		return false;
	if (selector->id != base::string_atom() && selector->id != id_)
		return false;
	if (!klass_.containsClass(selector->klass))
		return false;
	if (!matchPseudoClasses(selector->pseudo_classes))
		return false;
	if (selector->tag != base::string_atom() && selector->tag != base::string_intern("*")
		&& selector->tag != tag_)
		return false;
	return true;
}

void Node::computeLayout()
{
	LOG(WARNING) << "TODO: remove Node::computeLayout()";
}

bool Node::positioned() const
{
	return computed_style_.position != style::PositionType::Static;
}

bool Node::relativePositioned() const
{
	return computed_style_.position == style::PositionType::Relative;
}

bool Node::absolutelyPositioned() const
{
	return computed_style_.position != style::PositionType::Static
		&& computed_style_.position != style::PositionType::Relative;
}

Node* Node::positionedAncestor() const
{
	Node* p = parent_;
	while (p) {
		if (p->positioned())
			return p;
		p = p->parent_;
	}
	return nullptr;
}

scene2d::PointF Node::getMousePosition() const
{
	return scene_->getMousePosition();
}

void Node::requestPaint()
{
	scene_->requestPaint();
}

void Node::requestUpdate()
{
	scene_->requestUpdate();
}

void Node::requestAnimationFrame(scene2d::Node* node)
{
	scene_->requestAnimationFrame(node);
}

void Node::updateTextLayout()
{
	text_flow_ = graph2d::createTextFlow(
		text_,
		computed_style_.line_height.pixelOrZero(),
		computed_style_.font_family.keyword_val.c_str(),
		computed_style_.font_style,
		computed_style_.font_weight,
		computed_style_.font_size.pixelOrZero());
}

void Node::layoutComputed()
{
	if (layout_.scroll_object.has_value()) {
		// update: ScrollData <-- ScrollObject
		DimensionF viewport_size = layout_.scroll_object->viewport_size;
		float max_x = std::max(layout_.scroll_object->content_size.width - viewport_size.width, 0.0f);
		float max_y = std::max(layout_.scroll_object->content_size.height - viewport_size.height, 0.0f);
		scroll_data_.offset.x = std::min(scroll_data_.offset.x, max_x);
		scroll_data_.offset.y = std::min(scroll_data_.offset.y, max_y);
		
		// sync: ScrollData --> ScrollObject
		layout_.scroll_object->scroll_offset = scroll_data_.offset;
		layout_.scroll_object->v_scrollbar_active = scroll_data_.v_scrollbar_active_pos.has_value();
		layout_.scroll_object->v_scrollbar_hover = scroll_data_.v_scrollbar_hover;
		layout_.scroll_object->h_scrollbar_active = scroll_data_.h_scrollbar_active_pos.has_value();
		layout_.scroll_object->h_scrollbar_hover = scroll_data_.h_scrollbar_hover;
	} else {
		scroll_data_.offset = PointF();
	}
	if (control_) {
		RectF content_rect = style::LayoutObject::contentRect(&layout_);
		control_->onLayout(this, content_rect);
	}
}

bool Node::matchPseudoClasses(const style::PseudoClasses& pseudo_classes) const
{
	for (auto it = pseudo_classes.begin(); it != pseudo_classes.end(); ++it) {
		const base::string_atom& klass = *it;
		if (klass == base::string_intern("active")) {
			if (!(state_ & NODE_STATE_ACTIVE))
				return false;
		} else if (klass == base::string_intern("hover")) {
			if (!(state_ & NODE_STATE_HOVER))
				return false;
		} else if (klass == base::string_intern("focus")) {
			if (!(state_ & NODE_STATE_FOCUSED))
				return false;
		} else {
			return false;
		}
	}
	return true;
}

void Node::handleScrollEvent(scene2d::MouseEvent& event)
{
	static const float SCROLLBAR_WHEEL_FACTOR = 16.0f;

	if (!layout_.scroll_object.has_value())
		return;

	style::ScrollObject& so = layout_.scroll_object.value();

	if (event.cmd == scene2d::MOUSE_WHEEL) {
		float d = -event.wheel_delta * SCROLLBAR_WHEEL_FACTOR;
		if (event.modifiers == scene2d::LSHIFT_MODIFIER || event.modifiers == scene2d::RSHIFT_MODIFIER) {
			float w = so.viewport_size.width;
			float x = std::clamp(so.scroll_offset.x + d, 0.0f, so.content_size.width - w);
			if (so.scroll_offset.x != x) {
				so.scroll_offset.x = x;
				scroll_data_.offset = so.scroll_offset;
				requestPaint();
			}
		} else {
			float h = so.viewport_size.height;
			float y = std::clamp(so.scroll_offset.y + d, 0.0f, so.content_size.height - h);
			if (so.scroll_offset.y != y) {
				so.scroll_offset.y = y;
				scroll_data_.offset = so.scroll_offset;
				requestPaint();
			}
		}
	} else if (event.cmd == scene2d::MOUSE_DOWN) {
		if (event.button & scene2d::LEFT_BUTTON) {
			style::ScrollObject::HitTestResult part = style::ScrollObject::hitTestPart(&so, event.pos);
			if (part == style::ScrollObject::HitTestResult::HScrollbarThumb
				&& !scroll_data_.h_scrollbar_active_pos.has_value()) {
				scroll_data_.h_scrollbar_active_pos.emplace(event.pos);
				requestPaint();
			} else if (part == style::ScrollObject::HitTestResult::VScrollbarThumb
				&& !scroll_data_.v_scrollbar_active_pos.has_value()) {
				scroll_data_.v_scrollbar_active_pos.emplace(event.pos);
				requestPaint();
			} else if (part == style::ScrollObject::HitTestResult::VScrollbarTrackStartPiece
				&& !scroll_data_.v_scrollbar_active_pos.has_value()) {
				
				style::ScrollObject& so = layout_.scroll_object.value();
				float factor = so.viewport_size.height / so.content_size.height;
				float y1 = so.scroll_offset.y * factor;
				float y2 = (so.scroll_offset.y + so.viewport_size.height) * factor;

				scroll_data_.offset.y = std::max(0.0f, scroll_data_.offset.y - (y2 - y1));
				so.scroll_offset = scroll_data_.offset;

				requestPaint();
			} else if (part == style::ScrollObject::HitTestResult::VScrollbarTrackEndPiece
				&& !scroll_data_.v_scrollbar_active_pos.has_value()) {

				style::ScrollObject& so = layout_.scroll_object.value();
				float factor = so.viewport_size.height / so.content_size.height;
				float y1 = so.scroll_offset.y * factor;
				float y2 = (so.scroll_offset.y + so.viewport_size.height) * factor;

				scroll_data_.offset.y = std::min(so.content_size.height - so.viewport_size.height,
					scroll_data_.offset.y + (y2 - y1));
				so.scroll_offset = scroll_data_.offset;

				requestPaint();
			} else if (part == style::ScrollObject::HitTestResult::HScrollbarTrackStartPiece
				&& !scroll_data_.v_scrollbar_active_pos.has_value()) {

				style::ScrollObject& so = layout_.scroll_object.value();
				float factor = so.viewport_size.width / so.content_size.width;
				float x1 = so.scroll_offset.x * factor;
				float x2 = (so.scroll_offset.x + so.viewport_size.width) * factor;

				scroll_data_.offset.x = std::max(0.0f, scroll_data_.offset.x - (x2 - x1));
				so.scroll_offset = scroll_data_.offset;

				requestPaint();
			} else if (part == style::ScrollObject::HitTestResult::HScrollbarTrackEndPiece
				&& !scroll_data_.v_scrollbar_active_pos.has_value()) {

				style::ScrollObject& so = layout_.scroll_object.value();
				float factor = so.viewport_size.width / so.content_size.width;
				float x1 = so.scroll_offset.x * factor;
				float x2 = (so.scroll_offset.x + so.viewport_size.width) * factor;

				scroll_data_.offset.x = std::min(so.content_size.width - so.viewport_size.width,
					scroll_data_.offset.x + (x2 - x1));
				so.scroll_offset = scroll_data_.offset;

				requestPaint();
			}
		}
	} else if (event.cmd == scene2d::MOUSE_MOVE || event.cmd == scene2d::MOUSE_OVER || event.cmd == scene2d::MOUSE_OUT) {
		scroll_data_.h_scrollbar_hover = false;
		scroll_data_.v_scrollbar_hover = false;
		if (!event.buttons) {
			style::ScrollObject::HitTestResult part = style::ScrollObject::hitTestPart(&so, event.pos);
			if (part == style::ScrollObject::HitTestResult::HScrollbarThumb
				&& !scroll_data_.h_scrollbar_active_pos.has_value()) {
				scroll_data_.h_scrollbar_hover = true;
				requestPaint();
			} else if (part == style::ScrollObject::HitTestResult::VScrollbarThumb
				&& !scroll_data_.v_scrollbar_active_pos.has_value()) {
				scroll_data_.v_scrollbar_hover = true;
				requestPaint();
			}
		}
	} else if (event.cmd == scene2d::MOUSE_UP) {
		if (event.button & scene2d::LEFT_BUTTON) {
			scroll_data_.h_scrollbar_active_pos = absl::nullopt;
			scroll_data_.v_scrollbar_active_pos = absl::nullopt;
			requestPaint();
		}
	}
}

void resolve_style(style::DisplayType& style, const style::DisplayType* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("block"))
				style = style::DisplayType::Block;
			else if (spec.value->keyword_val == base::string_intern("inline"))
				style = style::DisplayType::Inline;
			else if (spec.value->keyword_val == base::string_intern("inline-block"))
				style = style::DisplayType::InlineBlock;
		}
	}
}

void resolve_style(style::PositionType& style, const style::PositionType* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("static"))
				style = style::PositionType::Static;
			else if (spec.value->keyword_val == base::string_intern("relative"))
				style = style::PositionType::Relative;
			else if (spec.value->keyword_val == base::string_intern("absolute"))
				style = style::PositionType::Absolute;
			else if (spec.value->keyword_val == base::string_intern("fixed"))
				style = style::PositionType::Fixed;
		}
	}
}

void resolve_style(style::FontStyle& style, const style::FontStyle* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("normal"))
				style = style::FontStyle::Normal;
			else if (spec.value->keyword_val == base::string_intern("italic"))
				style = style::FontStyle::Italic;
		}
	}
}

void resolve_style(style::FontWeight& style, const style::FontWeight* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("normal")) {
				style = style::FontWeight(400);
			} else if (spec.value->keyword_val == base::string_intern("bold")) {
				style = style::FontWeight(600);
			} else if (spec.value->keyword_val == base::string_intern("lighter")) {
				if (style.raw() <= 500) {
					style = style::FontWeight(100);
				} else if (style.raw() <= 700) {
					style = style::FontWeight(400);
				} else {
					style = style::FontWeight(400);
				}
			} else if (spec.value->keyword_val == base::string_intern("bolder")) {
				if (style.raw() <= 300) {
					style = style::FontWeight(400);
				} else if (style.raw() <= 500) {
					style = style::FontWeight(700);
				} else {
					style = style::FontWeight(900);
				}
			}
		}
	}
}

void resolve_style(style::TextAlign& style, const style::TextAlign* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("left"))
				style = style::TextAlign::Left;
			else if (spec.value->keyword_val == base::string_intern("center"))
				style = style::TextAlign::Center;
			else if (spec.value->keyword_val == base::string_intern("right"))
				style = style::TextAlign::Right;
		}
	}
}

void resolve_style(style::OverflowType& style, const style::OverflowType* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("visible"))
				style = style::OverflowType::Visible;
			else if (spec.value->keyword_val == base::string_intern("hidden"))
				style = style::OverflowType::Hidden;
			else if (spec.value->keyword_val == base::string_intern("auto"))
				style = style::OverflowType::Auto;
			else if (spec.value->keyword_val == base::string_intern("scroll"))
				style = style::OverflowType::Scroll;
		}
	}
}

void resolve_style(style::BoxSizingType& style, const style::BoxSizingType* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("content-box"))
				style = style::BoxSizingType::ContentBox;
			else if (spec.value->keyword_val == base::string_intern("border-box"))
				style = style::BoxSizingType::BorderBox;
		}
	}
}

void resolve_style(style::Color& style, const style::Color* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			style = style::Color::fromString(spec.value->keyword_val.c_str());
		} else if (spec.value->unit == style::ValueUnit::HexColor) {
			style = style::Color::fromString(spec.value->string_val);
		}
	}
}

void resolve_style(style::CursorType& style, const style::CursorType* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("none")) {
				style = style::CursorType::None;
			} else if (spec.value->keyword_val == base::string_intern("auto")) {
				style = style::CursorType::Auto;
			} else if (spec.value->keyword_val == base::string_intern("default")) {
				style = style::CursorType::Default;
			} else if (spec.value->keyword_val == base::string_intern("crosshair")) {
				style = style::CursorType::Crosshair;
			} else if (spec.value->keyword_val == base::string_intern("pointer")) {
				style = style::CursorType::Pointer;
			} else if (spec.value->keyword_val == base::string_intern("text")) {
				style = style::CursorType::Text;
			} else if (spec.value->keyword_val == base::string_intern("wait")) {
				style = style::CursorType::Wait;
			} else {
				LOG(WARNING) << "unimplemented css cursor " << spec.value->keyword_val;
			}
		}
	}
}

}
