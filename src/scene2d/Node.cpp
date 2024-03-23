#include "Node.h"
#include "graph2d/graph2d.h"
#include "style/style.h"
#include "script/Value.h"
#include "control.h"
#include "Scene.h"
#include "style/BoxConstraintSolver.h"
#include "base/scoped_setter.h"
#include "style/BlockLayout.h"
#include "style/InlineLayout.h"
#include "absl/functional/bind_front.h"

namespace scene2d {

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
static void resolve_style(style::VerticalAlign& style, const style::VerticalAlign* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::TextDecorationLineType& style, const style::TextDecorationLineType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::OverflowType& style, const style::OverflowType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::BoxSizingType& style, const style::BoxSizingType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::Color& style, const style::Color* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::CursorType& style, const style::CursorType* parent,
	const style::ValueSpec& spec);
static void resolve_style(std::shared_ptr<graph2d::BitmapInterface>& style, const std::shared_ptr<graph2d::BitmapInterface>* parent,
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
	comp_state_ = JS_DupValue(scene_->script_ctx_->get(), comp_state);
	weakptr_ = new base::WeakObjectProxy<Node>(this);
	weakptr_->retain();
	script::ComponentState::setNode(scene->scriptContext().get(), comp_state_, this);
}

Node::~Node()
{
	for (Node* child : children_) {
		child->release();
	}
	children_.clear();
	parent_ = nullptr;
	if (comp_state_ != JS_UNINITIALIZED) {
		script::ComponentState::setNode(scene_->scriptContext().get(), comp_state_, nullptr);
		JS_FreeValue(scene_->script_ctx_->get(), comp_state_);
		comp_state_ = JS_UNINITIALIZED;
	}
	if (control_) {
		control_->onDetach(this);
		control_ = nullptr;
	}
	if (weakptr_) {
		weakptr_->clear();
		weakptr_->release();
		weakptr_ = nullptr;
	}
}

Node* Node::parentElement() const
{
	auto p = parent();
	if (!p)
		return nullptr;
	if (p->type_ == NodeType::NODE_ELEMENT)
		return p;
	return p->parentElement();
}

void Node::appendChild(Node* child)
{
	if (!child)
		return;
	child->retain();
	child->parent_ = weaken();
	child->child_index = (int)children_.size();
	children_.push_back(child);
}

Node* Node::removeChildAt(size_t idx)
{
	if (idx >= children_.size())
		return nullptr;
	Node* node = children_[idx];
	for (size_t i = idx + 1; i < children_.size(); ++i) {
		--children_[i]->child_index;
	}
	children_.erase(children_.begin() + idx);
	node->parent_ = nullptr;
	node->child_index = -1;
	return node;
}

bool Node::hitTest(const PointF &pos, int flags) const
{
	if (layout_.scroll_object.has_value()) {
		auto padding = style::LayoutObject::padding(&layout_);
		PointF p = pos + PointF(padding.left, padding.top);
		if (style::ScrollObject::hitTest(&layout_.scroll_object.value(), p, flags))
			return true;
	}
	if (control_)
		return control_->hitTest(pos, flags);
	return false;
}

void Node::onEvent(Event& event)
{
	switch (event.group()) {
	case MouseEvent::EVENT_GROUP:
		return onMouseEvent((MouseEvent&)event);
	case KeyEvent::EVENT_GROUP:
		return onKeyEvent((KeyEvent&)event);
	case FocusEvent::EVENT_GROUP:
		return onFocusEvent((FocusEvent&)event);
	case ImeEvent::EVENT_GROUP:
		return onImeEvent((ImeEvent&)event);
	default:
		LOG(ERROR) << absl::StrFormat("event: group=%d cmd=%d not handled", event.group(), event.cmd);
	}
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
	if (value.isNull() || (value.isBool() && !value.toBool())) {
		attrs_.erase(name);
		if (name == base::string_intern("checked")) {
			state_ &= ~NODE_STATE_CHECKED;
		} else if (name == base::string_intern("disabled")) {
			state_ &= ~NODE_STATE_DISABLED;
		}
	} else {
		attrs_[name] = value;
		if (name == base::string_intern("checked")) {
			state_ |= NODE_STATE_CHECKED;
		} else if (name == base::string_intern("disabled")) {
			state_ |= NODE_STATE_DISABLED;
		}
	}
	if (control_)
		control_->onSetAttribute(name, value);
}

void Node::setEventHandler(base::string_atom name, const script::Value& func)
{
	if (func.isUndefined()) {
		event_handlers_.erase(name);
	} else {
		event_handlers_[name] = func;
	}
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
		} else if (tag_ == base::string_intern("button")
			|| tag_ == base::string_intern("spinner")) {
			computed_style_.display = style::DisplayType::InlineBlock;
			//computed_style_.box_sizing = style::BoxSizingType::BorderBox;
		} else {
			computed_style_.display = style::DisplayType::Block;
		}
	}
	computed_style_.position = style::PositionType::Static;
	computed_style_.resolveDefault(parent_ ? &parent_->computed_style_ : nullptr);
}

void Node::resolveStyle(StyleResolveContext& ctx, const style::StyleSpec& spec)
{
	CHECK(type_ == NodeType::NODE_ELEMENT);
#define RESOLVE_STYLE(x) \
	if (ctx.testAndUpdate(#x, spec.x.important)) { \
		resolve_style(computed_style_.x, \
			parent_ ? &parent_->computed_style_.x : nullptr, \
			spec.x); \
	}
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
	RESOLVE_STYLE(background_image);
	RESOLVE_STYLE(line_height);
	RESOLVE_STYLE(font_family);
	RESOLVE_STYLE(font_size);
	RESOLVE_STYLE(font_style);
	RESOLVE_STYLE(font_weight);
	RESOLVE_STYLE(text_align);
	RESOLVE_STYLE(vertical_align);
	RESOLVE_STYLE(text_decoration_line);
	RESOLVE_STYLE(overflow_x);
	RESOLVE_STYLE(overflow_y);
	RESOLVE_STYLE(box_sizing);
	RESOLVE_STYLE(cursor);
#undef RESOLVE_STYLE
}

void Node::resolveInlineStyle(StyleResolveContext& ctx)
{
	resolveStyle(ctx, specStyle_);
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
	Node* p = parent();
	while (p) {
		if (p->positioned())
			return p;
		p = p->parent();
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
	if (!text_flow_) {
		text_flow_ = graph2d::createTextFlow(
			text_,
			computed_style_.line_height.pixelOrZero(),
			computed_style_.font_family.keyword_val.c_str(),
			computed_style_.font_style,
			computed_style_.font_weight,
			computed_style_.font_size.pixelOrZero());
	} else {
		graph2d::updateTextFlow(
			text_flow_,
			text_,
			computed_style_.line_height.pixelOrZero(),
			computed_style_.font_family.keyword_val.c_str(),
			computed_style_.font_style,
			computed_style_.font_weight,
			computed_style_.font_size.pixelOrZero());
	}
}

void Node::layoutComputed()
{
	if (layout_.scroll_object.has_value()) {
		//LOG(INFO) << &scroll_data_.hover_sub_control << " layoutComputed, hover_sub_control " << scroll_data_.hover_sub_control.has_value();
		// update: ScrollData <-- ScrollObject
		DimensionF viewport_size = layout_.scroll_object->viewport_size;
		float max_x = std::max(layout_.scroll_object->content_size.width - viewport_size.width, 0.0f);
		float max_y = std::max(layout_.scroll_object->content_size.height - viewport_size.height, 0.0f);
		scroll_data_.offset.x = std::min(scroll_data_.offset.x, max_x);
		scroll_data_.offset.y = std::min(scroll_data_.offset.y, max_y);
		
		// sync: ScrollData --> ScrollObject
		layout_.scroll_object->scroll_offset = scroll_data_.offset;
		layout_.scroll_object->scrollbar_flags = style::ScrollObject::State_None;
		if (scroll_data_.active_sub_control.has_value()) {
			uint32_t shift = style::ScrollObject::State_SubControl_ShiftBits + (uint32_t)scroll_data_.active_sub_control.value();
			layout_.scroll_object->scrollbar_flags |= (1 << shift);
			layout_.scroll_object->scrollbar_flags |= style::ScrollObject::State_Active;
		} else if (scroll_data_.hover_sub_control.has_value()) {
			uint32_t shift = style::ScrollObject::State_SubControl_ShiftBits + (uint32_t)scroll_data_.hover_sub_control.value();
			layout_.scroll_object->scrollbar_flags |= (1 << shift);
			layout_.scroll_object->scrollbar_flags |= style::ScrollObject::State_MouseOver;
		}
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
		} else if (klass == base::string_intern("checked")) {
			if (!(state_ & NODE_STATE_CHECKED))
				return false;
		} else if (klass == base::string_intern("disabled")) {
			if (!(state_ & NODE_STATE_DISABLED))
				return false;
		} else if (klass == base::string_intern("enabled")) {
			if ((state_ & NODE_STATE_DISABLED))
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
	if (event.isHandled())
		return;

	style::ScrollObject& so = layout_.scroll_object.value();

	if (event.cmd == scene2d::MOUSE_WHEEL) {
		float d = -event.wheel_delta * SCROLLBAR_WHEEL_FACTOR;
		if (event.modifiers == scene2d::LSHIFT_MODIFIER || event.modifiers == scene2d::RSHIFT_MODIFIER) {
			float w = so.viewport_size.width;
			float x = std::clamp(so.scroll_offset.x + d, 0.0f, so.content_size.width - w);
			if (so.scroll_offset.x != x) {
				event.setHandled();
				so.scroll_offset.x = x;
				scroll_data_.offset = so.scroll_offset;
				requestPaint();
			}
		} else {
			float h = so.viewport_size.height;
			float y = std::clamp(so.scroll_offset.y + d, 0.0f, so.content_size.height - h);
			if (so.scroll_offset.y != y) {
				event.setHandled();
				so.scroll_offset.y = y;
				scroll_data_.offset = so.scroll_offset;
				requestPaint();
			}
		}
	} else if (event.cmd == scene2d::MOUSE_HWHEEL) {
		float d = event.wheel_delta * SCROLLBAR_WHEEL_FACTOR;
		float w = so.viewport_size.width;
		float x = std::clamp(so.scroll_offset.x + d, 0.0f, so.content_size.width - w);
		if (so.scroll_offset.x != x) {
			event.setHandled();
			so.scroll_offset.x = x;
			scroll_data_.offset = so.scroll_offset;
			requestPaint();
		}
	} else if (event.cmd == scene2d::MOUSE_DOWN) {
		event.setHandled();
		if (event.button & scene2d::LEFT_BUTTON) {
			scroll_data_.active_sub_control = style::ScrollObject::subControlHitTest(&so, event.pos);
			if (scroll_data_.active_sub_control.has_value()) {
				scroll_data_.active_pos.emplace(std::make_pair(event.pos, scroll_data_.offset));

				auto sc = scroll_data_.active_sub_control.value();
				if (sc == style::ScrollObject::SubControl::VTrackStartPiece) {
					// Scroll one page up
					so.scroll_offset.y
						= scroll_data_.offset.y
						= std::max(0.0f,
							scroll_data_.offset.y - so.viewport_size.height);
				} else if (sc == style::ScrollObject::SubControl::VTrackEndPiece) {
					// Scroll one page down
					so.scroll_offset.y
						= scroll_data_.offset.y
						= std::min(so.content_size.height - so.viewport_size.height,
							scroll_data_.offset.y + so.viewport_size.height);
				} else if (sc == style::ScrollObject::SubControl::VStartButton) {
					// Scroll up
					auto delta = std::max(computed_style_.lineHeightInPixels(), 8.0f) * 3.0f;
					so.scroll_offset.y
						= scroll_data_.offset.y
						= std::max(0.0f,
							scroll_data_.offset.y - delta);
				} else if (sc == style::ScrollObject::SubControl::VEndButton) {
					// Scroll down
					auto delta = std::max(computed_style_.lineHeightInPixels(), 8.0f) * 3.0f;
					so.scroll_offset.y
						= scroll_data_.offset.y
						= std::min(so.content_size.height - so.viewport_size.height,
							scroll_data_.offset.y + delta);
				} else if (sc == style::ScrollObject::SubControl::HTrackStartPiece) {
					// Scroll one page left
					so.scroll_offset.x
						= scroll_data_.offset.x
						= std::max(0.0f,
							scroll_data_.offset.x - so.viewport_size.width);
				} else if (sc == style::ScrollObject::SubControl::HTrackEndPiece) {
					// Scroll one page right
					so.scroll_offset.x
						= scroll_data_.offset.x
						= std::min(so.content_size.width - so.viewport_size.width,
							scroll_data_.offset.x + so.viewport_size.width);
				} else if (sc == style::ScrollObject::SubControl::HStartButton) {
					// Scroll left
					auto delta = std::max(computed_style_.lineHeightInPixels(), 8.0f) * 3.0f;
					so.scroll_offset.x
						= scroll_data_.offset.x
						= std::max(0.0f,
							scroll_data_.offset.x - delta);
				} else if (sc == style::ScrollObject::SubControl::HEndButton) {
					// Scroll right
					auto delta = std::max(computed_style_.lineHeightInPixels(), 8.0f) * 3.0f;
					so.scroll_offset.x
						= scroll_data_.offset.x
						= std::min(so.content_size.width - so.viewport_size.width,
							scroll_data_.offset.x + delta);
				}
				requestPaint();
			} else {
				scroll_data_.active_pos = absl::nullopt;
			}
		}
	} else if (event.cmd == scene2d::MOUSE_MOVE || event.cmd == scene2d::MOUSE_OVER) {
		event.setHandled();
		auto old_hover = scroll_data_.hover_sub_control;
		scroll_data_.hover_sub_control = style::ScrollObject::subControlHitTest(&so, event.pos);
		if (old_hover != scroll_data_.hover_sub_control) {
			requestPaint();
		}

		if (event.cmd == scene2d::MOUSE_MOVE && scroll_data_.active_pos.has_value()) {
			if (scroll_data_.active_sub_control == style::ScrollObject::SubControl::VThumb) {
				// Drag vertical scrollbar thumb
				float factor = so.content_size.height / (so.viewport_size.height - 2.0f * style::ScrollObject::SCROLLBAR_GUTTER_WIDTH);
				float y = scroll_data_.active_pos.value().second.y
					+ (event.pos.y - scroll_data_.active_pos.value().first.y) * factor;

				auto old = so.scroll_offset.y;
				so.scroll_offset.y
					= scroll_data_.offset.y
					= std::max(0.0f, std::min(so.content_size.height - so.viewport_size.height, y));
				//LOG(INFO) << absl::StrFormat("event_pos.y old=%.0f new=%.0f", scroll_data_.active_pos.value().first.y, event.pos.y);
				//LOG(INFO) << absl::StrFormat("scroll_offset y=%.0f so.y=%.0f", y, so.scroll_offset.y);
				requestPaint();
			} else if (scroll_data_.active_sub_control == style::ScrollObject::SubControl::HThumb) {
				// Drag horizontal scrollbar thumb
				float factor = so.content_size.width / (so.viewport_size.width - 2.0f * style::ScrollObject::SCROLLBAR_GUTTER_WIDTH);
				float x = scroll_data_.active_pos.value().second.x
					+ (event.pos.x - scroll_data_.active_pos.value().first.x) * factor;

				auto old = so.scroll_offset.x;
				so.scroll_offset.x
					= scroll_data_.offset.x
					= std::max(0.0f, std::min(so.content_size.width - so.viewport_size.width, x));
				//LOG(INFO) << absl::StrFormat("event_pos.x old=%.0f new=%.0f", scroll_data_.active_pos.value().first.x, event.pos.x);
				//LOG(INFO) << absl::StrFormat("scroll_offset x=%.0f so.x=%.0f", x, so.scroll_offset.x);
				requestPaint();
			}
		}
	} else if (event.cmd == scene2d::MOUSE_OUT) {
		event.setHandled();
		scroll_data_.hover_sub_control = absl::nullopt;
		//LOG(INFO) << this << " hover_sub_control MOUSE_OUT changed, now " << scroll_data_.hover_sub_control.has_value();
		requestPaint();
	} else if (event.cmd == scene2d::MOUSE_UP) {
		event.setHandled();
		if (event.button == scene2d::LEFT_BUTTON) {
			scroll_data_.active_sub_control = absl::nullopt;
			scroll_data_.active_pos = absl::nullopt;
			//LOG(INFO) << "cleanup active_pos";
			requestPaint();
		}
	}
}

void Node::onMouseEvent(MouseEvent& event)
{
	if (layout_.scroll_object.has_value()) {
		auto padding = style::LayoutObject::padding(&layout_);
		PointF pos = event.pos + PointF(padding.left, padding.top);
		base::scoped_setter<PointF> _(event.pos, pos);
		handleScrollEvent(event);
	}
	if (control_)
		control_->onMouseEvent(this, event);
}

void Node::onKeyEvent(KeyEvent& event)
{
	if (control_)
		control_->onKeyEvent(this, event);
}

void Node::onFocusEvent(FocusEvent& event)
{
	if (control_)
		control_->onFocusEvent(this, event);
}

void Node::onImeEvent(ImeEvent& event)
{
	if (control_)
		control_->onImeEvent(this, event);
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

void resolve_style(style::VerticalAlign& style, const style::VerticalAlign* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("baseline"))
				style = { style::VerticalAlignType::Baseline };
			else if (spec.value->keyword_val == base::string_intern("top"))
				style = { style::VerticalAlignType::Top };
			else if (spec.value->keyword_val == base::string_intern("bottom"))
				style = { style::VerticalAlignType::Bottom };
			else if (spec.value->keyword_val == base::string_intern("middle"))
				style = { style::VerticalAlignType::Middle };
			else if (spec.value->keyword_val == base::string_intern("text-top"))
				style = { style::VerticalAlignType::TextTop };
			else if (spec.value->keyword_val == base::string_intern("text-bottom"))
				style = { style::VerticalAlignType::TextBottom };
			else if (spec.value->keyword_val == base::string_intern("super"))
				style = { style::VerticalAlignType::Super };
			else if (spec.value->keyword_val == base::string_intern("sub"))
				style = { style::VerticalAlignType::Sub };
			else
				style = { style::VerticalAlignType::Baseline };
		} else if (spec.value->unit == style::ValueUnit::Percent || spec.value->unit == style::ValueUnit::Pixel) {
			style = { style::VerticalAlignType::Value, spec.value };
		}
	}
}

void resolve_style(style::TextDecorationLineType& style, const style::TextDecorationLineType* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("none"))
				style = style::TextDecorationLineType::None;
			else if (spec.value->keyword_val == base::string_intern("underline"))
				style = style::TextDecorationLineType::Underline;
			else if (spec.value->keyword_val == base::string_intern("overline"))
				style = style::TextDecorationLineType::Overline;
			else if (spec.value->keyword_val == base::string_intern("line-through"))
				style = style::TextDecorationLineType::LineThrough;
			else
				style = style::TextDecorationLineType::None;
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

static void resolve_style(std::shared_ptr<graph2d::BitmapInterface>& style, const std::shared_ptr<graph2d::BitmapInterface>* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Url) {
			style = graph2d::createBitmap(spec.value->string_val);
		}
	}
}

}
