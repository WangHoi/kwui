#pragma once
#include "Layout.h"
#include "TextLayout.h"
#include "StyleValue.h"
#include "StyleFont.h"
#include "scene2d/geom_types.h"
#include "base/log.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "graph2d/TextLayout.h"
#include <vector>
#include <memory>
#include <string>

namespace scene2d {
class Node;
}

namespace style {

class BlockFormatContext;
class InlineFormatContext;
class InlineBox;
struct LineBox;
struct GlyphRunBoxes;
enum class InlineBoxType {
	Empty,
	WithGlyphRun,
	WithInlineChildren,
	WithBFC,
};
struct InlineBox {
	scene2d::PointF pos; // BFC coordinates, set by LineBox::layout(),
	scene2d::DimensionF size;
	float baseline = 0; // offset from top

	LineBox* line_box = nullptr; // set by IFC::setupBox()
	float line_box_offset_x = 0; // set by IFC::setupBox()

	template <typename Sink>
	friend void AbslStringify(Sink& sink, const InlineBox& o) {
		absl::Format(&sink, "InlineBox { ");
		absl::Format(&sink, "pos=%v, ", o.pos);
		absl::Format(&sink, "size=%v, ", o.size);
		absl::Format(&sink, "baseline=%.0f, ", o.baseline);
		absl::Format(&sink, "}");
	}
};

struct InlineFragment {
	float line_height = 0;
	FontMetrics font_metrics;
	VerticalAlign align;
	LayoutObject* layout_object = nullptr;
	InlineBox* box = nullptr;
	std::vector<InlineFragment> children;

	void initFrom(LayoutObject* o, InlineBox* box = nullptr);
};

struct LineBox {
	float offset_x; // used while building

	float left; // BFC coord: left
	float avail_width;
	float line_height = 0.0f;

	scene2d::DimensionF used_size;
	float used_baseline; // offset from used_size's top

	std::vector<InlineFragment> inline_frags;

	LineBox(float left_, float avail_w);
	~LineBox() = default;
	int addInlineBox(LayoutObject* o, InlineBox* box);
	void mergeInlineBox(LayoutObject* o, InlineBox* box,
		InlineBox* first_child, InlineBox* last_child);
	void arrange(float offset_y, style::TextAlign text_align);
	void arrangeX(style::TextAlign text_align);
	void arrangeY(LayoutObject* owner, float offset_y);

private:
	void arrangeY(const InlineFragment& strut, absl::Span<InlineFragment> slice);
};

class InlineFormatContext : public LineBoxInterface {
public:
	InlineFormatContext(LayoutObject* owner, BlockFormatContext& bfc, float avail_width);
	~InlineFormatContext();

	inline BlockFormatContext& bfc() const { return bfc_; }
	float getAvailWidth() const;
	void setupBox(InlineBox* box);
	LineBox* getLineBox(float pref_min_width);
	LineBox* newLineBox();

	void arrange(style::TextAlign text_align);
	void arrangeX(style::TextAlign text_align);
	void arrangeY();
	inline float getLayoutHeight() const
	{
		return height_;
	}
	float getLayoutWidth() const;
	inline const std::vector<std::unique_ptr<LineBox>>& lineBoxes() const
	{
		return line_boxes_;
	}

	LineBox* getCurrentLine() override;
	LineBox* getNextLine() override;

private:
	LayoutObject* owner_;
	BlockFormatContext& bfc_;
	float avail_width_;
	std::vector<std::unique_ptr<LineBox>> line_boxes_;

	float height_;

	friend class InlineBoxBuilder;
};

} // namespace style
