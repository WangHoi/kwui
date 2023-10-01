#include "Node.h"
#include "graph2d/graph2d.h"
#include "style/style.h"
#include "control.h"

namespace scene2d {

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

void Node::onEvent(MouseEvent &event)
{
    if (control_)
        control_->onMouseEvent(event);
}

void Node::onEvent(KeyEvent &event)
{
    if (control_)
        control_->onKeyEvent(event);    
}

void Node::onEvent(FocusEvent &event)
{
    if (control_)
        control_->onFocusEvent(event);
}

void Node::onEvent(ImeEvent &event)
{
    if (control_)
        control_->onImeEvent(event);
}

void Node::setStyle(const style::StyleSpec& style)
{
    specStyle_ = style;
}

void Node::setAttribute(base::string_atom name, const NodeAttributeValue &value)
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

void Node::paintControl(windows::graphics::Painter &painter)
{
    if (control_)
        control_->onPaint(painter);
}


void Node::resolveStyle()
{
#define RESOLVE_ZERO_PX_DEFAULT(x) \
    computedStyle_.x = resolve(\
        parent_ ? &parent_->computedStyle_.x : nullptr, \
        specStyle_.x,\
        style::Value::fromPixel(0))
    RESOLVE_ZERO_PX_DEFAULT(margin_left);
    RESOLVE_ZERO_PX_DEFAULT(margin_top);
    RESOLVE_ZERO_PX_DEFAULT(margin_right);
    RESOLVE_ZERO_PX_DEFAULT(margin_bottom);
    RESOLVE_ZERO_PX_DEFAULT(border_left);
    RESOLVE_ZERO_PX_DEFAULT(border_top);
    RESOLVE_ZERO_PX_DEFAULT(border_right);
    RESOLVE_ZERO_PX_DEFAULT(border_bottom);
    RESOLVE_ZERO_PX_DEFAULT(padding_left);
    RESOLVE_ZERO_PX_DEFAULT(padding_top);
    RESOLVE_ZERO_PX_DEFAULT(padding_right);
    RESOLVE_ZERO_PX_DEFAULT(padding_bottom);
    RESOLVE_ZERO_PX_DEFAULT(left);
    RESOLVE_ZERO_PX_DEFAULT(top);
    RESOLVE_ZERO_PX_DEFAULT(right);
    RESOLVE_ZERO_PX_DEFAULT(bottom);

    RESOLVE_ZERO_PX_DEFAULT(min_width);
    RESOLVE_ZERO_PX_DEFAULT(min_height);
    RESOLVE_ZERO_PX_DEFAULT(max_width);
    RESOLVE_ZERO_PX_DEFAULT(max_height);
    RESOLVE_ZERO_PX_DEFAULT(width);
    RESOLVE_ZERO_PX_DEFAULT(height);
#undef RESOLVE_ZERO_PX_DEFAULT
}

void Node::computeLayout()
{
    origin_.x = computedStyle_.left.f32_val;
    origin_.y = computedStyle_.top.f32_val;
    size_.width = computedStyle_.width.f32_val;
    size_.height = computedStyle_.height.f32_val;
}

style::Value Node::resolve(const style::Value* parent,
    const style::ValueSpec& spec, const style::Value& default_)
{
    if (spec.type == style::ValueSpecType::Inherit) {
        return parent ? *parent : default_;
    } else if (spec.type == style::ValueSpecType::Specified) {
        return *spec.value;
    } else {
        return default_;
    }
}

}
