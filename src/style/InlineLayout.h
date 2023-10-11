#pragma once
#include "Layout.h"
#include "scene2d/geom_types.h"
#include "base/log.h"
#include "style/style.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include <vector>
#include <memory>
#include <string>

namespace graph2d {
class TextLayoutInterface;
}
namespace scene2d {
class Node;
}

namespace style {

class BlockFormatContext;
class InlineFormatContext;
class InlineBox;
struct LineBox;
enum class InlineBoxType {
	Empty,
	WithText,
	WithInlineChildren,
};
struct InlineBox {
	scene2d::PointF offset; // set by LineBox::layout()
	scene2d::DimensionF size;
	float baseline = 0; // offset from bottom

	InlineBoxType type = InlineBoxType::Empty;
	absl::variant<
		absl::monostate,				// empty
		graph2d::TextLayoutInterface*,	// text
		std::vector<InlineBox*> 		// inline boxes
	> payload;

	LineBox* line_box; // set by IFC::setupBox()
	float line_box_offset_x; // set by IFC::setupBox()
};

struct LineBox {
	scene2d::PointF offset;

	float left; // BFC coord: left
	float avail_width;
	// float line_gap;  // leading

	scene2d::DimensionF used_size;
	float used_baseline; // offset from used_size's bottom

	std::vector<InlineBox*> inline_boxes;

	LineBox(float left_, float avail_w);
	~LineBox();
	int addInlineBox(InlineBox* box)
	{
		int idx = (int)inline_boxes.size();
		inline_boxes.push_back(box);
		return idx;
	}
	/*
	std::tuple<float, float> getAvailWidth() const
	{
		float x = left;
		float w = avail_width;
		for (auto& b : inline_boxes) {
			x += b->size.width;
			w -= b->size.width;
		}
		return std::make_tuple(x, w);
	}
	float remainWidth() const
	{
		float w = avail_width;
		for (auto& b : inline_boxes) {
			w -= b->size.width;
		}
		return w;
	}
	*/
	void layout(float offset_y)
	{
		used_size.width = 0;
		float top = 0, bottom = 0;
		for (auto& b : inline_boxes) {
			used_size.width += b->size.width;
			top = std::max(top, b->size.height - b->baseline);
			bottom = std::max(bottom, b->baseline);
		}
		used_size.height = top + bottom;
		used_baseline = bottom;

		float x = left;
		for (auto& b : inline_boxes) {
			b->offset.x = x;
			b->offset.y = offset_y + (top - (b->size.height - b->baseline));
			x += b->size.width;
		}
	}
};

class InlineFormatContext {
public:
	InlineFormatContext(BlockFormatContext& bfc, float left, float avail_width);
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

	BlockFormatContext& bfc_;
	float left_;
	float avail_width_;
	std::vector<std::unique_ptr<LineBox>> line_boxes_;

	float height_;
};

} // namespace style
