#pragma once
#include "BlockLayout.h"
#include "InlineLayout.h"
#include "TextLayout.h"
#include "InlineBlockLayout.h"
#include "ScrollObject.h"
#include "absl/strings/str_format.h"
#include <sstream>

namespace scene2d {
class Node;
struct MouseEvent;
}

namespace graph2d {
class PainterInterface;
}

namespace style {

struct LayoutObject;

struct TextBox {
	graph2d::TextFlowInterface* text_flow = nullptr;
	GlyphRunBoxes glyph_run_boxes;

	template <typename Sink>
	friend void AbslStringify(Sink& sink, const TextBox& o) {
		absl::Format(&sink, "TextBox { ");
		for (auto& inline_box : o.glyph_run_boxes.inline_boxes) {
			absl::Format(&sink, "%v, ", *inline_box);
		}
		absl::Format(&sink, "}");
	}
};

struct FlowRoot {
	LayoutObject* root = nullptr;
	LayoutObject* positioned_parent = nullptr;
};

struct LayoutObject {
	enum Flag {
		HAS_BLOCK_CHILD_FLAG = 1,
		HAS_INLINE_CHILD_FLAG = 2,
		NEW_BFC_FLAG = 4,
		ANON_SPAN_FLAG = 8,
		ANON_BLOCK_FLAG = 16,
		PHANTOM_SPAN_FLAG = 32,
	};
	absl::variant<
		absl::monostate,
		BlockBox,
		std::vector<std::unique_ptr<InlineBox>>,
		InlineBlockBox,
		TextBox
	> box;

	uint32_t flags = 0;
	absl::optional<BlockFormatContext> bfc;
	absl::optional<InlineFormatContext> ifc;
	std::vector<std::unique_ptr<LayoutObject>> aux_boxes;
	std::unique_ptr<Style> anon_style;

	float min_width = 0.0f;
	float max_width = std::numeric_limits<float>::infinity();
	absl::optional<float> prefer_height;

	absl::optional<ScrollObject> scroll_object;

	const Style* style = nullptr;
	LayoutObject* parent = nullptr;
	LayoutObject* next_sibling = nullptr;
	LayoutObject* prev_sibling = nullptr;
	LayoutObject* first_child = nullptr;
	LayoutObject* last_child = nullptr;

	std::vector<LayoutObject*> positioned_children;

	scene2d::Node* node = nullptr;

	void init(const Style* style, scene2d::Node* node);
	void reset();

	static void measure(LayoutObject* o);
	static void reflow(FlowRoot root, const scene2d::DimensionF& viewport_size);
	static void paint(LayoutObject* o, graph2d::PainterInterface* painter);
	static LayoutObject* pick(LayoutObject* o, scene2d::PointF pos, int flag_mask, scene2d::PointF* out_local_pos);
	static scene2d::PointF getOffset(LayoutObject* o);

	static scene2d::PointF pos(LayoutObject* o);
	static void setPos(LayoutObject* o, const scene2d::PointF& pos);
	/* relative to o's margin origin,
	 * bounding box of in-flow children's border-box
	 * and positioned-children's border-box */
	static absl::optional<scene2d::RectF> getChildrenBoundingRect(LayoutObject* o);

	// relative to o's margin box origin
	static scene2d::RectF borderRect(LayoutObject* o);
	// relative to o's margin box origin
	static scene2d::RectF paddingRect(LayoutObject* o);
	// relative to o's margin box origin
	static scene2d::RectF contentRect(LayoutObject* o);
	static EdgeOffsetF padding(const LayoutObject* o);

	void removeFromParent();
	void append(LayoutObject* child);
	void insertBeforeMe(LayoutObject* o);
	void insertAfterMe(LayoutObject* o);
	void dumpTree();
	void dumpTree(int indent, std::ostringstream& stream);

private:
	enum ScrollbarPolicy {
		Hidden,
		Stable,
		StableBothEdges,
	};

	static void arrangeBlock(LayoutObject* o,
		BlockFormatContext& bfc,
		const scene2d::DimensionF& viewport_size);
	static void arrangeBlock(LayoutObject* o,
		BlockFormatContext& bfc,
		const scene2d::DimensionF& viewport_size,
		ScrollbarPolicy scroll_y);
	static void arrangeBlockX(LayoutObject* o,
		BlockFormatContext& bfc,
		const scene2d::DimensionF& viewport_size,
		ScrollbarPolicy scroll_y);
	static void arrangeBfcTop(LayoutObject* o,
		BlockFormatContext& bfc,
		BlockBox& box);
	static void arrangeBfcChildren(LayoutObject* o,
		BlockFormatContext& bfc,
		BlockBox& box,
		const scene2d::DimensionF& viewport_size);
	static void arrangeBfcBottom(LayoutObject* o,
		BlockFormatContext& bfc,
		BlockBox& box);
	static void prepare(LayoutObject* o, InlineFormatContext& ifc, const scene2d::DimensionF& viewport_size);
	static void arrange(LayoutObject* o, InlineFormatContext& ifc, const scene2d::DimensionF& viewport_size);
	static void translate(LayoutObject* o, scene2d::PointF offset);
	static void arrangeInlineBlock(LayoutObject* o, InlineFormatContext& ifc, const scene2d::DimensionF& viewport_size);
	static void arrangeInlineBlockX(LayoutObject* o,
		BlockFormatContext& bfc,
		const scene2d::DimensionF& viewport_size,
		ScrollbarPolicy scroll_y);
	static void arrangeInlineBlockChildren(LayoutObject* o,
		absl::optional<float> contg_height,
		const scene2d::DimensionF& viewport_size);
	static absl::optional<float> findFirstBaseline(LayoutObject* o, float accum_y = 0.0f);
	/* For box-container: clientRect(),
	 *   inline box container: bounding rect of first line-box's inline-box's top-left
	 *     and last line-box's inline-box's bottom-right.
	 * used by reflow() and getChildrenBoundingRect() */
	static absl::optional<scene2d::RectF> containingRectForPositionedChildren(LayoutObject* o);
	static void arrangePositionedChildren(LayoutObject* o, const scene2d::DimensionF& viewport_size);
	static void paintTextDecoration(LayoutObject* o, graph2d::PainterInterface* painter, const InlineBox* ib);

	template <typename Sink>
	friend void AbslStringify(Sink& sink, const LayoutObject& o) {
		absl::Format(&sink, "LayoutObject { ");
		absl::Format(&sink, "flags=0x%04x, ", o.flags);
		if (absl::holds_alternative<BlockBox>(o.box)) {
			absl::Format(&sink, "box=%v, ", absl::get<BlockBox>(o.box));
		} else if (absl::holds_alternative<std::vector<InlineBox>>(o.box)) {
			absl::Format(&sink, "box=[ ");
			const auto& ibs = absl::get<std::vector<InlineBox>>(o.box);
			for (auto& ib : ibs) {
				absl::Format(&sink, "%v, ", ib);
			}
			absl::Format(&sink, "]");
		} else if (absl::holds_alternative<TextBox>(o.box)) {
			absl::Format(&sink, "box=%v, ", absl::get<TextBox>(o.box));
		}
		absl::Format(&sink, "}");
	}
};

class LayoutTreeBuilder {
public:
	LayoutTreeBuilder(scene2d::Node* node);
	std::vector<FlowRoot> build();
private:
	void initFlowRoot(scene2d::Node* node);
	void prepareChild(scene2d::Node* node);
	void addText(scene2d::Node* node);
	void beginChild(LayoutObject* o);
	void endChild();
	void bubbleUp(LayoutObject* blk);

	scene2d::Node* root_ = nullptr;
	std::vector<scene2d::Node*> abs_pos_nodes_;

	FlowRoot* flow_root_ = nullptr;
	LayoutObject* contg_ = nullptr;
	std::vector<LayoutObject*> stack_;
	std::map<LayoutObject*, std::deque<LayoutObject*>> bbmap_;
};

}
