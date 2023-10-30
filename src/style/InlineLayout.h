#pragma once
#include "Layout.h"
#include "StyleValue.h"
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
struct TextBoxes;
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

	InlineBoxType type = InlineBoxType::Empty;
	absl::variant<
		absl::monostate,				// empty
		graph2d::GlyphRunInterface*,	// text boxes
		InlineBox* 						// first inline child
	> payload;
	InlineBox* parent;
	InlineBox* next_sibling;
	InlineBox* prev_sibling;

	LineBox* line_box = nullptr; // set by IFC::setupBox()
	float line_box_offset_x = 0; // set by IFC::setupBox()

	scene2d::RectF boundingRect() const;
};

struct LineBox {
	float offset_x; // used while building

	float left; // BFC coord: left
	float avail_width;
	// float line_gap;  // leading

	scene2d::DimensionF used_size;
	float used_baseline; // offset from used_size's top

	std::vector<InlineBox*> inline_boxes;

	LineBox(float left_, float avail_w);
	~LineBox() = default;
	int addInlineBox(InlineBox* box);
	void layoutArrange(float offset_y, style::TextAlign text_align);
};

class InlineBoxBuilder {
public:
	InlineBoxBuilder(InlineFormatContext& ifc, InlineBox* root);

	inline InlineFormatContext& ifc() const { return ifc_; }

	//float containingBlockWidth() const;
	//absl::optional<float> containingBlockHeight() const;
	void addText(scene2d::Node* node);
	void addInlineBlock(scene2d::Node* node);
	void addGlyphRun(
		const scene2d::PointF& pos,
		std::unique_ptr<graph2d::GlyphRunInterface> glyph_run);
	void appendGlyphRun(InlineBox* box);
	LineBox* getCurrentLine();
	LineBox* getNextLine();
	void beginInline(InlineBox* box);
	void endInline();

private:
	InlineFormatContext& ifc_;
	InlineBox* root_;
	InlineBox* contg_;
	InlineBox* last_child_ = nullptr;
	scene2d::Node* text_node_ = nullptr;
	LineBox* line_ = nullptr;
	std::vector<std::tuple<InlineBox*, InlineBox*>> stack_;
};

class InlineFormatContext {
public:
	InlineFormatContext(BlockFormatContext& bfc, float left, float avail_width);
	~InlineFormatContext();

	inline BlockFormatContext& bfc() const { return bfc_; }
	float getAvailWidth() const;
	void setupBox(InlineBox* box);
	LineBox* getLineBox(float pref_min_width);
	LineBox* newLineBox();

	// Add inline box to LineBox
	void addBox(InlineBox* box);

	void layoutArrange(style::TextAlign text_align);
	inline float getLayoutHeight() const
	{
		return height_;
	}
	float getLayoutWidth() const;

private:
	BlockFormatContext& bfc_;
	float left_;
	float avail_width_;
	std::vector<std::unique_ptr<LineBox>> line_boxes_;

	float height_;

	friend class InlineBoxBuilder;
};

} // namespace style
