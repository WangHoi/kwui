#pragma once
#include "scene2d/geom_types.h"
#include "base/log.h"
#include "style/style.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include <vector>
#include <memory>
#include <string>

namespace scene2d
{
class Node;
}

namespace style
{
struct BlockFormatContext;
class InlineFormatContext;
class InlineBox;
class BlockBox;
class BlockBoxBuilder;

struct EdgeOffsetF {
	float left = 0;
	float top = 0;
	float right = 0;
	float bottom = 0;
};

struct BoxF {
	// margin edges
	EdgeOffsetF margin;
	// border edges
	EdgeOffsetF border;
	// padding edges
	EdgeOffsetF padding;
	// content size
	scene2d::DimensionF content;
};

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
	scene2d::DimensionF content; // content size

	float avail_width = 0;
	absl::optional<float> prefer_height;
	
	BlockBoxType type = BlockBoxType::Empty;
	absl::variant<
		absl::monostate,	// empty
		BlockBox*,			// first-child
		scene2d::Node*,		// Node containing inlines
		std::unique_ptr<BlockFormatContext>		// nested BFC, in-flow
	> payload;
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
			margin.left + border.left + padding.left + content.width + padding.right + border.right + margin.right,
			margin.top + border.top + padding.top + content.height + padding.bottom + border.bottom + margin.bottom);
	}
	inline scene2d::RectF borderRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left,
			margin.top,
			border.left + padding.left + content.width + padding.right + border.right,
			border.top + padding.top + content.height + padding.bottom + border.bottom);
	}
	inline scene2d::RectF paddingRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left + border.left,
			margin.top + border.top,
			padding.left + content.width + padding.right,
			padding.top + content.height + padding.bottom);
	}
	inline scene2d::RectF contentRect() const
	{
		return scene2d::RectF::fromXYWH(
			margin.left + border.left + padding.left,
			margin.top + border.top + padding.top,
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

	float border_bottom = 0;
	float margin_bottom = 0;

	BlockFormatContext(scene2d::Node* owner_)
		: owner(owner_) {}
};

struct LineBox;

struct InlineBox {
	scene2d::PointF offset; // set by LineBox::layout()
	scene2d::DimensionF size;
	float baseline = 0; // offset from bottom

	std::vector<InlineBox*> children;

	LineBox* line_box; // set by IFC::setupBox()
	float line_box_offset_x; // set by IFC::setupBox()
};

class InlineFormatContext {
public:
	InlineFormatContext(float avail_width);
	~InlineFormatContext();
	float getAvailWidth() const;
	void setupBox(InlineBox* box);

	void addBox(InlineBox* box);

	void layout();
	inline float getLayoutHeight() const
	{
		return height_;
	}
	float getLayoutWidth() const;

private:
	LineBox* newLineBox();
	LineBox* getLineBox(float pref_min_width);

	float avail_width_;
	std::vector<std::unique_ptr<LineBox>> line_boxes_;

	float height_;
};

void try_convert_to_px(style::Value& v, float percent_base);
absl::optional<float> try_resolve_to_px(const style::Value& v, float percent_base);
absl::optional<float> try_resolve_to_px(const style::Value& v, absl::optional<float> percent_base);
float collapse_margin(float m1, float m2);

} // namespace scene2d
