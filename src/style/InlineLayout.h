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
struct TextBoxes;
enum class InlineBoxType {
	Empty,
	WithTextBoxes,
	WithGlyphRun,
	WithInlineChildren,
};
struct InlineBox {
	scene2d::PointF pos; // BFC coordinates, set by LineBox::layout(),
	scene2d::DimensionF size;
	float baseline = 0; // offset from top

	InlineBoxType type = InlineBoxType::Empty;
	absl::variant<
		absl::monostate,			// empty
		TextBoxes*,					// text boxes
		InlineBox* 					// first inline child
	> payload;
	InlineBox* parent;
	InlineBox* next_sibling;
	InlineBox* prev_sibling;

	LineBox* line_box = nullptr; // set by IFC::setupBox()
	float line_box_offset_x = 0; // set by IFC::setupBox()

	scene2d::RectF boundingRect() const;
};

class InlineBoxBuilder {
public:
	InlineBoxBuilder(InlineFormatContext& ifc, InlineBox* root);
	//float containingBlockWidth() const;
	//absl::optional<float> containingBlockHeight() const;
	void addText(scene2d::Node* node);
	void beginInline(InlineBox* box);
	void endInline();

private:
	InlineFormatContext& ifc_;
	InlineBox* root_;
	InlineBox* contg_;
	InlineBox* last_child_ = nullptr;
	std::vector<std::tuple<InlineBox*, InlineBox*>> stack_;
};

class InlineFormatContext {
public:
	InlineFormatContext(BlockFormatContext& bfc, float left, float avail_width);
	~InlineFormatContext();

	inline BlockFormatContext& bfc() const { return bfc_; }
	float getAvailWidth() const;
	void setupBox(InlineBox* box);
	void layoutText(const std::string& text, InlineBox& inline_box, TextBoxes& text_boxes);

	void addBox(InlineBox* box);

	void layoutArrange();
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
