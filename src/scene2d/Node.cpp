#include "Node.h"

namespace scene2d {

Node::Node(NodeType type)
    : type_(type) {}

Node::Node(NodeType type, const std::string& text)
    : Node(type)
{
    text_ = text;
}

Node::Node(NodeType type, base::string_atom tag)
    : Node(type)
{
    tag_ = tag;
}

Node::Node(NodeType type, JSValue comp_state)
    : Node(type)
{
    comp_state_ = comp_state;
    weakptr_ = new base::WeakObject<Node>(this);
    weakptr_->retain();
}

Node::~Node()
{
    if (weakptr_) {
        weakptr_->release();
        weakptr_ = nullptr;
    }
}

void Node::appendChild(Node *child)
{
    child->retain();
    child->parent_ = this;
    child->child_index = (int)children_.size();
    children_.push_back(child);
}

}
