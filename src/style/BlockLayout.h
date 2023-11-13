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
	EdgeOffsetF inner_padding;
	scene2d::DimensionF content; // content size

	absl::optional<float> prefer_height;

	BlockBoxType type = BlockBoxType::Empty;
	absl::variant<
		absl::monostate,	// empty
		BlockBox*,			// first-child
		scene2d::Node*,		// Node containing inlines
		std::unique_ptr<BlockFormatContext>		// nested BFC, in-flow
	> payload;
	const Style* style;
	absl::optional<std::tuple<float, float>> measure_width;
	std::optional<EdgeOffsetF> rel_offset; // Relative positioned box offset
	BlockBox* parent;
	BlockBox* next_sibling;
	BlockBox* prev_sibling;

	BlockBox()
		: parent(nullptr), next_sibling(this), prev_sibling(this) {}

	inline scene2d::RectF marginRect() const
	{
		return scene2d::RectF::fromXYWH(
			0,
			0,
			margin.left + border.left + padding.left + inner_padding.left + content.width + inner_padding.right + padding.right + border.right + margin.right,
			margin.top + border.top + padding.top + inner_padding.top + content.height + inner_padding.bottom + padding.bottom + border.bottom + margin.bottom);
	}
	inline scene2d::RectF borderRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left,
			margin.top,
			border.left + padding.left + inner_padding.left + content.width + inner_padding.right + padding.right + border.right,
			border.top + padding.top + inner_padding.top + content.height + inner_padding.bottom + padding.bottom + border.bottom);
	}
	inline scene2d::RectF paddingRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left + border.left,
			margin.top + border.top,
			padding.left + inner_padding.left + content.width + inner_padding.right + padding.right,
			padding.top + inner_padding.top + content.height + inner_padding.bottom + padding.bottom);
	}
	inline scene2d::RectF innerPaddingRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left + border.left + inner_padding.left,
			margin.top + border.top + inner_padding.top,
			padding.left + content.width + padding.right,
			padding.top + content.height + padding.bottom);
	}
	inline scene2d::RectF contentRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left + border.left + padding.left + inner_padding.left,
			margin.top + border.top + padding.top + inner_padding.top,
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
		absl::Format(&sink, "inner_pad=%v, ", o.inner_padding);
		absl::Format(&sink, "content=%v, ", o.content);
		absl::Format(&sink, "}");
	}
};

class BlockBoxBuilder {
public:
	BlockBoxBuilder(BlockBox* root);
	float containingBlockWidth() const;
	absl::optional<float> containingBlockHeight() const;
	void addText(scene2d::Node* node);
	void beginInline(scene2d::Node* node);
	void endInline();
	void beginBlock(BlockBox* box);
	void endBlock();

private:
	BlockBox* root_;
	BlockBox* contg_;
	BlockBox* last_child_ = nullptr;
	std::vector<std::tuple<BlockBox*, BlockBox*>> stack_;
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
	float max_border_right_edge = 0;	// max children's border right edge
	float max_border_bottom_edge = 0;	// max children's border bottom edge

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
