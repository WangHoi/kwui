#pragma once
#include "Layout.h"
#include "StyleValue.h"
#include "scene2d/geom_types.h"
#include "base/log.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "absl/strings/str_format.h"
#include <vector>
#include <memory>
#include <string>

namespace scene2d {
class Node;
}

namespace style {

struct BlockFormatContext;
class BlockBox;
class BlockBoxBuilder;

enum class BlockBoxType {
	Empty,
	WithBlockChildren,
	WithInlineChildren,
	WithBFC,
};
struct BlockBox {
	scene2d::PointF pos; // BFC coordinates
	EdgeOffsetF margin; // margin edges
	EdgeOffsetF border; // border edges
	EdgeOffsetF padding; // padding edges
	EdgeOffsetF scrollbar_gutter; // scrollbar space between inner-border-edge and outer-padding-edge
	scene2d::DimensionF content; // content size

	absl::optional<float> prefer_height;

	inline scene2d::RectF marginRect() const
	{
		return scene2d::RectF::fromXYWH(
			0,
			0,
			margin.left + border.left + padding.left + scrollbar_gutter.left + content.width + scrollbar_gutter.right + padding.right + border.right + margin.right,
			margin.top + border.top + padding.top + scrollbar_gutter.top + content.height + scrollbar_gutter.bottom + padding.bottom + border.bottom + margin.bottom);
	}
	inline scene2d::RectF borderRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left,
			margin.top,
			border.left + padding.left + scrollbar_gutter.left + content.width + scrollbar_gutter.right + padding.right + border.right,
			border.top + padding.top + scrollbar_gutter.top + content.height + scrollbar_gutter.bottom + padding.bottom + border.bottom);
	}
	inline scene2d::RectF paddingRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left + border.left,
			margin.top + border.top,
			padding.left + scrollbar_gutter.left + content.width + scrollbar_gutter.right + padding.right,
			padding.top + scrollbar_gutter.top + content.height + scrollbar_gutter.bottom + padding.bottom);
	}
	inline scene2d::RectF clientRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left + border.left + scrollbar_gutter.left,
			margin.top + border.top + scrollbar_gutter.top,
			padding.left + content.width + padding.right,
			padding.top + content.height + padding.bottom);
	}
	inline scene2d::RectF contentRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left + border.left + padding.left + scrollbar_gutter.left,
			margin.top + border.top + padding.top + scrollbar_gutter.top,
			content.width,
			content.height);
	}

	template <typename T>
	void eachChild(T&& f)
	{
		if (type == BlockBoxType::WithBlockChildren) {
			BlockBox* first_child = std::get<BlockBox*>(payload);
			BlockBox* child = first_child;
			do {
				f(child);
				child = child->next_sibling;
			} while (child != first_child);
		}
	}

	template <typename Sink>
	friend void AbslStringify(Sink& sink, const BlockBox& o) {
		absl::Format(&sink, "BlockBox { ");
		absl::Format(&sink, "pos=%v, ", o.pos);
		absl::Format(&sink, "margin=%v, ", o.margin);
		absl::Format(&sink, "border=%v, ", o.border);
		absl::Format(&sink, "padding=%v, ", o.padding);
		absl::Format(&sink, "inner_pad=%v, ", o.scrollbar_gutter);
		absl::Format(&sink, "content=%v, ", o.content);
		absl::Format(&sink, "}");
	}

	BlockBox() = default;
	BlockBox(const BlockBox&) = delete;
	BlockBox& operator=(const BlockBox&) = delete;
};

struct BlockFormatContext {
	// Owner of this BFC
	scene2d::Node* owner = nullptr;

	std::vector<scene2d::Node*> abs_pos_nodes; // 'absolute' and 'fixed' positioned nodes

	float contg_left_edge = 0;
	float contg_right_edge = 0.0f;
	float border_bottom_edge = 0;
	float margin_bottom_edge = 0;
	absl::optional<float> contg_height;

	BlockFormatContext(scene2d::Node* owner_)
		: owner(owner_) {}

	BlockFormatContext(const BlockFormatContext&) = delete;
	BlockFormatContext& operator=(const BlockFormatContext&) = delete;
};

template <typename Sink>
void AbslStringify(Sink& sink, BlockBoxType p) {
	switch (p) {
	case BlockBoxType::Empty:
		absl::Format(&sink, "BlockBoxType::Empty");
		break;
	case BlockBoxType::WithBlockChildren:
		absl::Format(&sink, "BlockBoxType::WithBlockChildren");
		break;
	case BlockBoxType::WithInlineChildren:
		absl::Format(&sink, "BlockBoxType::WithInlineChildren");
		break;
	case BlockBoxType::WithBFC:
		absl::Format(&sink, "BlockBoxType::WithBFC");
		break;
	default:
		absl::Format(&sink, "BlockBoxType::Custom(%d)", (int)p);
	}
}


} // namespace style
