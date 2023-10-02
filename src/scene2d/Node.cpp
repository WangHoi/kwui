#include "Node.h"
#include "graph2d/graph2d.h"
#include "style/style.h"
#include "control.h"

namespace scene2d {

static void resolve_style(style::Value& style,
    const style::Value* parent,
    const style::ValueSpec& spec,
    const style::Value& default_);
static void resolve_style(style::DisplayType& style, const style::DisplayType* parent,
    const style::ValueSpec& spec, const style::DisplayType& default_);
static void resolve_style(style::PositionType& style, const style::PositionType* parent,
    const style::ValueSpec& spec, const style::PositionType& default_);

Node::Node(NodeType type)
    : type_(type)
    , origin_(PointF::fromZeros())
    , size_(DimensionF::fromZeros()) {}

Node::Node(NodeType type, const std::string& text)
    : Node(type)
{
    text_ = text;
    text_layout_ = graph2d::createTextLayout(text);
}

Node::Node(NodeType type, base::string_atom tag)
    : Node(type)
{
    tag_ = tag;
    control_.reset(ControlRegistry::get()->createControl(tag));
    if (control_)
        control_->onAttach(this);
}

Node::Node(NodeType type, JSValue comp_state)
    : Node(type)
{
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
}

void Node::appendChild(Node* child)
{
    child->retain();
    child->parent_ = this;
    child->child_index = (int)children_.size();
    children_.push_back(child);
}

bool Node::testFlags(int flags) const
{
    if (control_)
        return control_->testFlags(flags);
    return false;
}

void Node::onEvent(MouseEvent& event)
{
    if (control_)
        control_->onMouseEvent(event);
}

void Node::onEvent(KeyEvent& event)
{
    if (control_)
        control_->onKeyEvent(event);
}

void Node::onEvent(FocusEvent& event)
{
    if (control_)
        control_->onFocusEvent(event);
}

void Node::onEvent(ImeEvent& event)
{
    if (control_)
        control_->onImeEvent(event);
}

void Node::setId(base::string_atom id)
{
    id_ = id;
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

void Node::paintControl(windows::graphics::Painter& painter)
{
    if (control_)
        control_->onPaint(painter);
}

void Node::resolveStyle(const style::StyleSpec& spec)
{
#define RESOLVE_STYLE(x, def) \
    resolve_style(computedStyle_.x, \
        parent_ ? &parent_->computedStyle_.x : nullptr, \
        spec.x,\
        def)
    RESOLVE_STYLE(display, style::DisplayType::Block);
    RESOLVE_STYLE(position, style::PositionType::Static);
    RESOLVE_STYLE(margin_left, style::Value::fromPixel(0));
    RESOLVE_STYLE(margin_top, style::Value::fromPixel(0));
    RESOLVE_STYLE(margin_right, style::Value::fromPixel(0));
    RESOLVE_STYLE(margin_bottom, style::Value::fromPixel(0));
    RESOLVE_STYLE(border_left_width, style::Value::fromPixel(0));
    RESOLVE_STYLE(border_top_width, style::Value::fromPixel(0));
    RESOLVE_STYLE(border_right_width, style::Value::fromPixel(0));
    RESOLVE_STYLE(border_bottom_width, style::Value::fromPixel(0));
    RESOLVE_STYLE(border_top_left_radius, style::Value::fromPixel(0));
    RESOLVE_STYLE(border_top_right_radius, style::Value::fromPixel(0));
    RESOLVE_STYLE(border_bottom_right_radius, style::Value::fromPixel(0));
    RESOLVE_STYLE(border_bottom_left_radius, style::Value::fromPixel(0));
    RESOLVE_STYLE(padding_left, style::Value::fromPixel(0));
    RESOLVE_STYLE(padding_top, style::Value::fromPixel(0));
    RESOLVE_STYLE(padding_right, style::Value::fromPixel(0));
    RESOLVE_STYLE(padding_bottom, style::Value::fromPixel(0));
    RESOLVE_STYLE(left, style::Value::fromPixel(0));
    RESOLVE_STYLE(top, style::Value::fromPixel(0));
    RESOLVE_STYLE(right, style::Value::fromPixel(0));
    RESOLVE_STYLE(bottom, style::Value::fromPixel(0));

    RESOLVE_STYLE(min_width, style::Value::fromPixel(0));
    RESOLVE_STYLE(min_height, style::Value::fromPixel(0));
    RESOLVE_STYLE(max_width, style::Value::fromPixel(0));
    RESOLVE_STYLE(max_height, style::Value::fromPixel(0));
    RESOLVE_STYLE(width, style::Value::fromPixel(0));
    RESOLVE_STYLE(height, style::Value::fromPixel(0));
#undef RESOLVE_STYLE
}

bool Node::matchSimple(style::Selector* selector) const
{
    if (type_ != NodeType::NODE_ELEMENT)
        return false;
    if (selector->id != base::string_atom() && selector->id != id_)
        return false;
    LOG(WARNING) << "handle css klasses match";
    // if (selector->klasses != base::string_atom() && selector->klasses != klasses_)
    //     return false;
    if (selector->tag != base::string_atom() && selector->tag != base::string_intern("*")
        && selector->tag != tag_)
        return false;
    return true;
}

void Node::computeLayout()
{
    origin_.x = computedStyle_.left.f32_val;
    origin_.y = computedStyle_.top.f32_val;
    size_.width = computedStyle_.width.f32_val;
    size_.height = computedStyle_.height.f32_val;
}

void resolve_style(style::Value& style, const style::Value* parent,
    const style::ValueSpec& spec, const style::Value& default_)
{
    if (spec.type == style::ValueSpecType::Inherit) {
        style = parent ? *parent : default_;
    } else if (spec.type == style::ValueSpecType::Specified) {
        style = *spec.value;
    } else {
        style = default_;
    }
}

void resolve_style(style::DisplayType& style, const style::DisplayType* parent,
    const style::ValueSpec& spec, const style::DisplayType& default_)
{
    if (spec.type == style::ValueSpecType::Inherit) {
        style = parent ? *parent : default_;
    } else if (spec.type == style::ValueSpecType::Specified) {
        if (spec.value->unit == style::ValueUnit::Keyword) {
            if (spec.value->keyword_val == base::string_intern("block"))
                style = style::DisplayType::Block;
            else if (spec.value->keyword_val == base::string_intern("inline"))
                style = style::DisplayType::Inline;
            else if (spec.value->keyword_val == base::string_intern("inline-block"))
                style = style::DisplayType::InlineBlock;
        } else {
            style = default_;
        }
    } else {
        style = default_;
    }
}

void resolve_style(style::PositionType& style, const style::PositionType* parent,
    const style::ValueSpec& spec, const style::PositionType& default_)
{
    if (spec.type == style::ValueSpecType::Inherit) {
        style = parent ? *parent : default_;
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
        } else {
            style = default_;
        }
    } else {
        style = default_;
    }
}

}
