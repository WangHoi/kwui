#pragma once
#include "BlockLayout.h"
#include "InlineLayout.h"
#include "TextLayout.h"
#include "absl/strings/str_format.h"

namespace scene2d {
class Node;
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
		absl::Format(&sink, "TextBox { }");
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
	};
	absl::variant<
		absl::monostate,
		BlockBox,
		std::vector<InlineBox>,
		TextBox
	> box;

	uint32_t flags = 0;
	absl::optional<BlockFormatContext> bfc;
	absl::optional<InlineFormatContext> ifc;
	std::unique_ptr<LayoutObject> anon_block;

	float min_width = 0.0f;
	float max_width = std::numeric_limits<float>::infinity();
	absl::optional<float> prefer_height;

	const Style* style = nullptr;
	LayoutObject* parent = nullptr;
	LayoutObject* next_sibling = nullptr;
	LayoutObject* prev_sibling = nullptr;
	LayoutObject* first_child = nullptr;

	scene2d::Node* node = nullptr;

	void init(const Style* style, scene2d::Node* node);
	void reset();

	static void reflow(FlowRoot root, const scene2d::DimensionF& viewport_size);
	static void paint(LayoutObject* o, graph2d::PainterInterface* painter);
	static LayoutObject* pick(FlowRoot root, const scene2d::PointF& pos, int flag_mask, scene2d::PointF* out_local_pos);

private:
	enum ScrollbarPolicy {
		Hidden,
		Stable,
		StableBothEdges,
	};

	static void measure(LayoutObject* o, float viewport_height);
	static void arrange(LayoutObject* o,
		BlockFormatContext& bfc,
		const scene2d::DimensionF& viewport_size);
	static void arrange(LayoutObject* o,
		BlockFormatContext& bfc,
		const scene2d::DimensionF& viewport_size,
		ScrollbarPolicy scroll_y);
	static void arrangeBlockX(LayoutObject* o,
		BlockFormatContext& bfc,
		const scene2d::DimensionF& viewport_size,
		ScrollbarPolicy scroll_y);
	static void arrangeBlockTop(LayoutObject* o, BlockFormatContext& bfc);
	static void arrangeBlockChildren(LayoutObject* o,
		BlockFormatContext& bfc,
		const scene2d::DimensionF& viewport_size);
	static void arrangeBlockBottom(LayoutObject* o, BlockFormatContext& bfc);
	static void arrange(LayoutObject* o, InlineFormatContext& ifc);
	static LayoutObject* pick(LayoutObject *o, const scene2d::PointF& pos, int flag_mask, scene2d::PointF* out_local_pos);

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
	void parentAddBlockChild();
	void parentAddInlineChild();

	scene2d::Node* root_ = nullptr;
	std::vector<scene2d::Node*> abs_pos_nodes_; // 'absolute' and 'fixed' positioned nodes

	LayoutObject* current_ = nullptr;
	LayoutObject* last_child_ = nullptr;
	// (contg, last_child)
	std::vector<std::tuple<LayoutObject*, LayoutObject*>> stack_;
	bool reparent_to_anon_block_pending_ = false;
	bool new_bfc_pending_ = false;
};

}
