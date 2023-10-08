#include "Node.h"
#include "graph2d/graph2d.h"
#include "style/style.h"
#include "control.h"
#include "Scene.h"
#include "BoxConstraintSolver.h"

namespace scene2d {

static void resolve_style(style::Value& style,
	const style::Value* parent,
	const style::ValueSpec& spec,
	const style::Value& default_);
static void resolve_style(style::DisplayType& style, const style::DisplayType* parent,
	const style::ValueSpec& spec, const style::DisplayType& default_);
static void resolve_style(style::PositionType& style, const style::PositionType* parent,
	const style::ValueSpec& spec, const style::PositionType& default_);

Node::Node(NodeType type)
	: type_(type)
	, origin_(PointF::fromZeros())
	, size_(DimensionF::fromZeros()) {}

Node::Node(NodeType type, const std::string& text)
	: Node(type)
{
	text_ = text;
	text_layout_ = graph2d::createTextLayout(text);
}

Node::Node(NodeType type, base::string_atom tag)
	: Node(type)
{
	tag_ = tag;
	control_.reset(ControlRegistry::get()->createControl(tag));
	if (control_)
		control_->onAttach(this);
}

Node::Node(NodeType type, JSValue comp_state)
	: Node(type)
{
	comp_state_ = comp_state;
	weakptr_ = new base::WeakObjectProxy<Node>(this);
	weakptr_->retain();
}

Node::~Node()
{
	if (control_) {
		control_->onDetach(this);
		control_ = nullptr;
	}
	if (weakptr_) {
		weakptr_->release();
		weakptr_ = nullptr;
	}
}

void Node::appendChild(Node* child)
{
	child->retain();
	child->parent_ = this;
	child->child_index = (int)children_.size();
	children_.push_back(child);
}

bool Node::testFlags(int flags) const
{
	if (control_)
		return control_->testFlags(flags);
	return false;
}

void Node::onEvent(MouseEvent& event)
{
	if (control_)
		control_->onMouseEvent(event);
}

void Node::onEvent(KeyEvent& event)
{
	if (control_)
		control_->onKeyEvent(event);
}

void Node::onEvent(FocusEvent& event)
{
	if (control_)
		control_->onFocusEvent(event);
}

void Node::onEvent(ImeEvent& event)
{
	if (control_)
		control_->onImeEvent(event);
}

void Node::setId(base::string_atom id)
{
	id_ = id;
}

void Node::setClass(const style::Classes& klass)
{
	klass_ = klass;
}

void Node::setStyle(const style::StyleSpec& style)
{
	specStyle_ = style;
}

void Node::setAttribute(base::string_atom name, const NodeAttributeValue& value)
{
	attrs_[name] = value;
	if (control_)
		control_->onSetAttribute(name, value);
}

void Node::setEventHandler(base::string_atom name, JSValue func)
{
	event_handlers_[name] = func;
	if (control_)
		control_->onSetEventHandler(name, func);
}

void Node::paintControl(windows::graphics::Painter& painter)
{
	if (control_)
		control_->onPaint(painter);
}

void Node::resolveStyle(const style::StyleSpec& spec)
{
	if (type_ != NodeType::NODE_ELEMENT) {
		if (parent_)
			computed_style_ = parent_->computed_style_;
		return;
	}
#define RESOLVE_STYLE(x, def) \
    resolve_style(computed_style_.x, \
        parent_ ? &parent_->computed_style_.x : nullptr, \
        spec.x,\
        def)
	RESOLVE_STYLE(display, style::DisplayType::Block);
	RESOLVE_STYLE(position, style::PositionType::Static);
	RESOLVE_STYLE(margin_left, style::Value::fromPixel(0));
	RESOLVE_STYLE(margin_top, style::Value::fromPixel(0));
	RESOLVE_STYLE(margin_right, style::Value::fromPixel(0));
	RESOLVE_STYLE(margin_bottom, style::Value::fromPixel(0));
	RESOLVE_STYLE(border_left_width, style::Value::fromPixel(0));
	RESOLVE_STYLE(border_top_width, style::Value::fromPixel(0));
	RESOLVE_STYLE(border_right_width, style::Value::fromPixel(0));
	RESOLVE_STYLE(border_bottom_width, style::Value::fromPixel(0));
	RESOLVE_STYLE(border_top_left_radius, style::Value::fromPixel(0));
	RESOLVE_STYLE(border_top_right_radius, style::Value::fromPixel(0));
	RESOLVE_STYLE(border_bottom_right_radius, style::Value::fromPixel(0));
	RESOLVE_STYLE(border_bottom_left_radius, style::Value::fromPixel(0));
	RESOLVE_STYLE(padding_left, style::Value::fromPixel(0));
	RESOLVE_STYLE(padding_top, style::Value::fromPixel(0));
	RESOLVE_STYLE(padding_right, style::Value::fromPixel(0));
	RESOLVE_STYLE(padding_bottom, style::Value::fromPixel(0));
	RESOLVE_STYLE(left, style::Value::fromPixel(0));
	RESOLVE_STYLE(top, style::Value::fromPixel(0));
	RESOLVE_STYLE(right, style::Value::fromPixel(0));
	RESOLVE_STYLE(bottom, style::Value::fromPixel(0));

	RESOLVE_STYLE(min_width, style::Value::fromPixel(0));
	RESOLVE_STYLE(min_height, style::Value::fromPixel(0));
	RESOLVE_STYLE(max_width, style::Value::fromPixel(0));
	RESOLVE_STYLE(max_height, style::Value::fromPixel(0));
	RESOLVE_STYLE(width, style::Value::fromPixel(0));
	RESOLVE_STYLE(height, style::Value::fromPixel(0));
#undef RESOLVE_STYLE
}

bool Node::matchSimple(style::Selector* selector) const
{
	if (type_ != NodeType::NODE_ELEMENT)
		return false;
	if (selector->id != base::string_atom() && selector->id != id_)
		return false;
	LOG(WARNING) << "handle css klasses match";
	if (!klass_.containsClass(selector->klass))
		return false;
	if (selector->tag != base::string_atom() && selector->tag != base::string_intern("*")
		&& selector->tag != tag_)
		return false;
	return true;
}

void Node::computeLayout()
{
	origin_.x = computed_style_.left.f32_val;
	origin_.y = computed_style_.top.f32_val;
	size_.width = computed_style_.width.f32_val;
	size_.height = computed_style_.height.f32_val;
}

void Node::layoutBlockElement(float contg_blk_width, absl::optional<float> contg_blk_height)
{
	CHECK(type_ == NodeType::NODE_ELEMENT) << "layoutBlockElement(): expect NODE_ELEMENT";

	bfc_ = std::make_optional<BlockFormatContext>();
	bfc_->owner = this;
	bfc_->contg_block_width = contg_blk_width;
	bfc_->contg_block_height = contg_blk_height;
	layoutBlockElement(*bfc_, 0);
	arrangeBlockElement(*bfc_, 0);
}

void Node::layoutText(InlineFormatContext& ifc)
{
	CHECK(type_ == NodeType::NODE_TEXT) << "layoutText(): expect NODE_TEXT";
	text_box_.size = text_layout_->rect().size();
	text_box_.baseline = text_layout_->baseline();
	ifc.setupBox(&text_box_);
}

void Node::layoutInlineElement(InlineFormatContext& ifc, int element_depth)
{
	CHECK(type_ == NodeType::NODE_ELEMENT) << "layoutInlineElement(): expect NODE_ELEMENT";

	inline_boxes_.clear();
	for (Node* child : children()) {
		layoutInlineChild(child, ifc, element_depth);
		assembleInlineChild(child, inline_boxes_);
	}
}

void Node::layoutBlockElement(BlockFormatContext& bfc, int element_depth)
{
	CHECK(type_ == NodeType::NODE_ELEMENT) << "layoutBlockElement(): expect NODE_ELEMENT";

	block_box_.children.clear();
	if (anyBlockChildren()) {
		std::unique_ptr<BlockWidthSolverInterface> solver = createBlockWidthSolver(bfc);
		if (!solver)
			return;

		float measure_width = solver->measureWidth();
		solver->setLayoutWidth(measure_width);

		// Compute width, left and right margins
		float base_width = solver->containingBlockWidth();
		block_box_.box.margin.left = solver->marginLeft();
		block_box_.box.border.left = try_resolve_to_px(computed_style_.border_left_width, base_width).value_or(0);
		block_box_.box.padding.left = try_resolve_to_px(computed_style_.padding_left, base_width).value_or(0);
		block_box_.box.content.width = solver->width();
		block_box_.box.padding.right = try_resolve_to_px(computed_style_.padding_right, base_width).value_or(0);
		block_box_.box.border.right = try_resolve_to_px(computed_style_.border_right_width, base_width).value_or(0);
		block_box_.box.margin.right = solver->marginRight();

		// Compute top and bottom margins
		absl::optional<float> base_height = bfc.contg_block_height;
		block_box_.box.margin.top = try_resolve_to_px(computed_style_.margin_top, base_width).value_or(0);
		block_box_.box.border.top = try_resolve_to_px(computed_style_.border_top_width, base_width).value_or(0);
		block_box_.box.padding.top = try_resolve_to_px(computed_style_.padding_top, base_width).value_or(0);

		block_box_.box.padding.bottom = try_resolve_to_px(computed_style_.padding_bottom, base_width).value_or(0);
		block_box_.box.border.bottom = try_resolve_to_px(computed_style_.border_bottom_width, base_width).value_or(0);
		block_box_.box.margin.bottom = try_resolve_to_px(computed_style_.margin_bottom, base_width).value_or(0);

		// Layout children, Normal Flow
		{
			base::scoped_setter _1(bfc.contg_block_width, measure_width);
			base::scoped_setter _2(bfc.contg_block_height,
				try_resolve_to_px(computed_style_.height, bfc.contg_block_height));
			eachLayoutChild([&](Node* child) {
				if (child->computed_style_.position == style::PositionType::Static
					|| child->computed_style_.position == style::PositionType::Relative) {
					layoutBlockChild(bfc, block_box_, child, element_depth);
				}
				});
		}
	} else {
		std::unique_ptr<BlockWidthSolverInterface> solver = createBlockWidthSolver(bfc);
		if (!solver)
			return;

		float measure_width = solver->measureWidth();

		InlineFormatContext ifc(measure_width);
		for (Node* child : children())
			layoutInlineChild(child, ifc, 0);
		ifc.layout();

		float layout_width = ifc.getLayoutWidth();
		float layout_height = ifc.getLayoutHeight();
		solver->setLayoutWidth(layout_width);

		// Compute width and margins
		float base_width = solver->containingBlockWidth();
		block_box_.box.margin.left = solver->marginLeft();
		block_box_.box.border.left = try_resolve_to_px(computed_style_.border_left_width, base_width).value_or(0);
		block_box_.box.padding.left = try_resolve_to_px(computed_style_.padding_left, base_width).value_or(0);
		block_box_.box.content.width = solver->width();
		block_box_.box.padding.right = try_resolve_to_px(computed_style_.padding_right, base_width).value_or(0);
		block_box_.box.border.right = try_resolve_to_px(computed_style_.border_right_width, base_width).value_or(0);
		block_box_.box.margin.right = solver->marginRight();

		// Compute height and margins
		absl::optional<float> base_height = bfc.contg_block_height;
		block_box_.box.margin.top = try_resolve_to_px(computed_style_.margin_top, base_height).value_or(0);
		block_box_.box.border.top = try_resolve_to_px(computed_style_.border_top_width, base_height).value_or(0);
		block_box_.box.padding.top = try_resolve_to_px(computed_style_.padding_top, base_height).value_or(0);

		block_box_.box.padding.bottom = try_resolve_to_px(computed_style_.padding_bottom, base_height).value_or(0);
		block_box_.box.border.bottom = try_resolve_to_px(computed_style_.border_bottom_width, base_height).value_or(0);
		block_box_.box.margin.bottom = try_resolve_to_px(computed_style_.margin_bottom, base_height).value_or(0);

		absl::optional<float> height = try_resolve_to_px(computed_style_.height, base_height);
		if (height.has_value()) {
			block_box_.box.content.height = *height;
		} else {
			// Compute 'auto' height
			block_box_.box.content.height = layout_height;
		}
	}
}

void Node::layoutInlineChild(Node* node, InlineFormatContext& ifc, int element_depth)
{
	if (node->type() == NodeType::NODE_TEXT) {
		node->layoutText(ifc);
		if (element_depth == 0)
			ifc.addBox(&node->text_box_);
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		node->layoutInlineElement(ifc, element_depth + 1);
		if (element_depth == 0) {
			for (InlineBox& box : node->inline_boxes_)
				ifc.addBox(&box);
		}
	} else if (node->type() == NodeType::NODE_COMPONENT) {
		for (auto child : node->children())
			layoutInlineChild(child, ifc, element_depth);
	}
}

void Node::assembleInlineChild(Node* node, std::vector<InlineBox>& boxes)
{
	if (node->type_ == NodeType::NODE_TEXT) {
		InlineBox wrap_box = node->text_box_;
		wrap_box.children.push_back(&node->text_box_);
		boxes.push_back(wrap_box);
	} else if (node->type_ == NodeType::NODE_ELEMENT) {
		for (InlineBox& child_box : node->inline_boxes_) {
			InlineBox wrap_box = child_box;
			wrap_box.children.push_back(&child_box);
			boxes.push_back(wrap_box);
		}
	} else if (node->type_ == NodeType::NODE_COMPONENT) {
		for (Node* child : node->children())
			assembleInlineChild(child, boxes);
	}
}

bool Node::anyBlockChildren() const
{
	for (auto child : children()) {
		if (child->type() == NodeType::NODE_COMPONENT) {
			return child->anyBlockChildren();
		} else if (child->type() == NodeType::NODE_ELEMENT) {
			if (child->computedStyle().display == style::DisplayType::Block)
				return true;
		}
	}
	return false;
}

void Node::layoutBlockChild(BlockFormatContext& bfc, BlockBox& block_box, Node* node, int element_depth)
{
	if (node->type() == NodeType::NODE_TEXT) {
		;
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		node->layoutBlockElement(bfc, element_depth + 1);
		block_box.children.push_back(&node->block_box_);
	} else if (node->type() == NodeType::NODE_COMPONENT) {
		for (auto child : node->children())
			layoutBlockChild(bfc, block_box, child, element_depth);
	}
}

std::unique_ptr<BlockWidthSolverInterface> Node::createBlockWidthSolver(BlockFormatContext& bfc)
{
	const auto& st = computed_style_;
	if (st.position == style::PositionType::Static || st.position == style::PositionType::Relative) {
		float base_width = bfc.contg_block_width;
		float clean_width = base_width
			- try_resolve_to_px(st.border_left_width, base_width).value_or(0)
			- try_resolve_to_px(st.padding_left, base_width).value_or(0)
			- try_resolve_to_px(st.padding_right, base_width).value_or(0)
			- try_resolve_to_px(st.border_right_width, base_width).value_or(0);
		return std::unique_ptr<BlockWidthSolverInterface>(
			new StaticBlockWidthSolver(clean_width,
				try_resolve_to_px(st.margin_left, base_width),
				try_resolve_to_px(st.width, base_width),
				try_resolve_to_px(st.margin_right, base_width))
		);
	} else if (st.position == style::PositionType::Absolute || st.position == style::PositionType::Fixed) {
		LOG(ERROR) << "create BlockWidthSolver with unsupported position type: " << (int)st.position;
		return nullptr;
	} else {
		LOG(ERROR) << "create BlockWidthSolver with invalid position type: " << (int)st.position;
		return nullptr;
	}
}

void Node::arrangeBlockElement(BlockFormatContext& bfc, int element_depth)
{
	CHECK(type_ == NodeType::NODE_ELEMENT) << "arrangeBlockElement(): expect NODE_ELEMENT";

	float borpad_top = block_box_.box.border.top + block_box_.box.padding.top;

	if (borpad_top > 0) {
		float coll_margin = collapse_margin(block_box_.box.margin.top,
			(bfc.margin_bottom - bfc.border_bottom));
		block_box_.pos.y = bfc.border_bottom + coll_margin - block_box_.box.margin.top;
		bfc.border_bottom = block_box_.pos.y + block_box_.box.margin.top + borpad_top;
		bfc.margin_bottom = bfc.border_bottom;
	}

	float bfc_border_bottom = bfc.border_bottom;
	float bfc_margin_bottom = bfc.margin_bottom;

	if (anyBlockChildren()) {
		base::scoped_setter _1(bfc.contg_block_width, block_box_.box.content.width);
		base::scoped_setter _2(bfc.contg_block_height,
			try_resolve_to_px(computed_style_.height, bfc.contg_block_height));
		eachLayoutChild([&](Node* child) {
			if (child->type() == NodeType::NODE_ELEMENT && (child->computed_style_.position == style::PositionType::Static
				|| child->computed_style_.position == style::PositionType::Relative)) {
				child->arrangeBlockElement(bfc, element_depth + 1);
			}
			});
	} else {
		float border_box_width = block_box_.box.border.top
			+ block_box_.box.padding.top
			+ block_box_.box.content.height
			+ block_box_.box.padding.bottom
			+ block_box_.box.border.bottom;
		if (border_box_width > 0) {
			float border_bottom = block_box_.pos.y
				+ block_box_.box.margin.top
				+ block_box_.box.border.top
				+ block_box_.box.padding.top
				+ block_box_.box.content.height
				+ block_box_.box.padding.bottom
				+ block_box_.box.border.bottom;
			float margin_bottom = border_bottom + block_box_.box.margin.bottom;
			if (border_bottom > bfc.border_bottom)
				bfc.border_bottom = border_bottom;
			if (margin_bottom > bfc.margin_bottom)
				bfc.margin_bottom = margin_bottom;
		} else {
			float m1 = collapse_margin(block_box_.box.margin.top, block_box_.box.margin.bottom);
			float coll_margin = collapse_margin(m1, (bfc.margin_bottom - bfc.border_bottom));
			block_box_.pos.y = bfc.border_bottom + coll_margin;
		}
	}

	absl::optional<float> base_height = bfc.contg_block_height;
	absl::optional<float> height = try_resolve_to_px(computed_style_.height, base_height);
	if (height.has_value()) {
		block_box_.box.content.height = *height;
		bfc.border_bottom = bfc_margin_bottom + *height;
		bfc.margin_bottom = bfc.border_bottom;
	} else {
		// Compute 'auto' height
		block_box_.box.content.height = max(0.0f,
			bfc.border_bottom - block_box_.pos.y - block_box_.box.margin.top - borpad_top);
	}

	float borpad_bottom = block_box_.box.border.bottom + block_box_.box.padding.bottom;
	if (borpad_bottom > 0) {
		bfc.border_bottom += borpad_bottom;
		bfc.margin_bottom = bfc.border_bottom;
	} else {
		float coll_margin = collapse_margin(block_box_.box.margin.bottom,
			(bfc.margin_bottom - bfc.border_bottom));
		bfc.margin_bottom = bfc.border_bottom + coll_margin;
	}

	// Layout 'absolute' or 'fixed' children
	if (anyBlockChildren()) {
		for (Node* child : children()) {
			if (child->computed_style_.position == style::PositionType::Absolute
				|| child->computed_style_.position == style::PositionType::Fixed) {

				child->layoutBlockElement(block_box_.box.content.width,
					block_box_.box.content.height);
			}
		}
	}
}

void resolve_style(style::Value& style, const style::Value* parent,
	const style::ValueSpec& spec, const style::Value& default_)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		style = parent ? *parent : default_;
	} else if (spec.type == style::ValueSpecType::Specified) {
		style = *spec.value;
	} else {
		style = default_;
	}
}

void resolve_style(style::DisplayType& style, const style::DisplayType* parent,
	const style::ValueSpec& spec, const style::DisplayType& default_)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		style = parent ? *parent : default_;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("block"))
				style = style::DisplayType::Block;
			else if (spec.value->keyword_val == base::string_intern("inline"))
				style = style::DisplayType::Inline;
			else if (spec.value->keyword_val == base::string_intern("inline-block"))
				style = style::DisplayType::InlineBlock;
		} else {
			style = default_;
		}
	} else {
		style = default_;
	}
}

void resolve_style(style::PositionType& style, const style::PositionType* parent,
	const style::ValueSpec& spec, const style::PositionType& default_)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		style = parent ? *parent : default_;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("static"))
				style = style::PositionType::Static;
			else if (spec.value->keyword_val == base::string_intern("relative"))
				style = style::PositionType::Relative;
			else if (spec.value->keyword_val == base::string_intern("absolute"))
				style = style::PositionType::Absolute;
			else if (spec.value->keyword_val == base::string_intern("fixed"))
				style = style::PositionType::Fixed;
		} else {
			style = default_;
		}
	} else {
		style = default_;
	}
}

}
