#pragma once
#include "BlockLayout.h"
#include "InlineLayout.h"
#include "TextLayout.h"
namespace scene2d {
class Node;
}

namespace style {

struct TextBox {
	graph2d::TextFlowInterface* text_flow = nullptr;
	TextBoxes text_boxes;
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
		InlineBox,
		TextBox
	> box;

	absl::variant<
		absl::monostate,	// empty
		BlockBox*,			// first-child
		scene2d::Node*,		// Node containing inlines
		std::unique_ptr<BlockFormatContext>		// nested BFC, in-flow
	> inner;
	uint32_t flags = 0;
	absl::optional<BlockFormatContext> bfc;
	absl::optional<InlineFormatContext> ifc;
	std::unique_ptr<LayoutObject> anon_block;

	const Style* style;
	LayoutObject* parent;
	LayoutObject* next_sibling;
	LayoutObject* prev_sibling;
	LayoutObject* first_child;

	void reset();
	
	static void reflow(LayoutObject* o, const scene2d::DimensionF& viewport_size);
};

class LayoutTreeBuilder {
public:
	LayoutTreeBuilder(scene2d::Node* node);
	std::vector<LayoutObject*> build();
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
