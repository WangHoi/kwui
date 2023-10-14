#include "BlockLayout.h"
#include "scene2d/Node.h"
#include <utility>

namespace style {

BlockBoxBuilder::BlockBoxBuilder(BlockBox* root)
    : root_(root), contg_(root) {}

float BlockBoxBuilder::containingBlockWidth() const
{
    return contg_->content.width;
}

absl::optional<float> BlockBoxBuilder::containingBlockHeight() const
{
    return contg_->prefer_height;
}

void BlockBoxBuilder::addText(scene2d::Node* node)
{
    if (contg_->type == BlockBoxType::Empty) {
        contg_->type = BlockBoxType::WithInlineChildren;
        contg_->payload = node->parent();
    }
}

void BlockBoxBuilder::beginInline(scene2d::Node* node)
{
    if (contg_->type == BlockBoxType::Empty) {
        contg_->type = BlockBoxType::WithInlineChildren;
        contg_->payload = node->parent();
    } else if (contg_->type == BlockBoxType::WithBlockChildren) {
        LOG(WARNING) << "begin Block in containing block type (Block) not implemented.";
    } else if (contg_->type == BlockBoxType::WithInlineChildren) {
        ;
    } else if (contg_->type == BlockBoxType::WithBFC) {
        LOG(WARNING) << "begin Block in containing block type (nested BFC) not implemented.";
    }
}

void BlockBoxBuilder::endInline()
{
}

void BlockBoxBuilder::beginBlock(BlockBox* box)
{
    if (contg_->type != BlockBoxType::Empty && contg_->type != BlockBoxType::WithBlockChildren) {
        LOG(WARNING) << "begin Block in containing block " << contg_->type << " not implemented.";
    }

    if (last_child_) {
        box->next_sibling = last_child_->next_sibling;
        box->prev_sibling = last_child_;
        last_child_->next_sibling = box;
        box->next_sibling->prev_sibling = box;
    } else {
        box->next_sibling = box->prev_sibling = box;
    }
    last_child_ = box;
    if (contg_->type == BlockBoxType::Empty) {
        contg_->type = BlockBoxType::WithBlockChildren;
        contg_->payload = box;
    }
    stack_.push_back(std::make_tuple(contg_, last_child_));

    contg_ = box;
    last_child_ = nullptr;
}

void BlockBoxBuilder::endBlock()
{
    std::tie(contg_, last_child_) = stack_.back();
    stack_.pop_back();
}

}
