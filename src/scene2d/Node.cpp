#include "Node.h"
#include "graph2d/graph2d.h"
#include "style/style.h"
#include "control.h"
#include "Scene.h"
#include "style/BoxConstraintSolver.h"
#include "base/scoped_setter.h"
#include "style/BlockLayout.h"
#include "style/InlineLayout.h"
#include "absl/functional/bind_front.h"

namespace scene2d {

static inline void resolve_style(style::Value& style,
	const style::Value* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		style = *spec.value;
	}
}
static void resolve_style(style::DisplayType& style, const style::DisplayType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::PositionType& style, const style::PositionType* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::FontStyle& style, const style::FontStyle* parent,
	const style::ValueSpec& spec);
static void resolve_style(style::FontWeight& style, const style::FontWeight* parent,
	const style::ValueSpec& spec);

Node::Node(Scene* scene, NodeType type)
	: scene_(scene)
	, type_(type)
	, origin_(PointF::fromZeros())
	, size_(DimensionF::fromZeros())
{
	block_box_.style = &computed_style_;
	weakptr_ = new base::WeakObjectProxy<Node>(this);
	weakptr_->retain();
}

Node::Node(Scene* scene, NodeType type, const std::string& text)
	: Node(scene, type)
{
	text_ = text;
}

Node::Node(Scene* scene, NodeType type, base::string_atom tag)
	: Node(scene, type)
{
	tag_ = tag;
	control_.reset(ControlRegistry::get()->createControl(tag));
	if (control_)
		control_->onAttach(this);
}

Node::Node(Scene* scene, NodeType type, JSValue comp_state)
	: Node(scene, type)
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
	for (Node* child : children_) {
		child->release();
	}
	children_.clear();
	if (comp_state_ != JS_UNINITIALIZED) {
		JS_FreeValue(scene_->script_ctx_->get(), comp_state_);
		comp_state_ = JS_UNINITIALIZED;
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

absl::optional<PointF> Node::hitTestNode(const PointF& p) {
	if (type_ == NodeType::NODE_ELEMENT && computed_style_.display == style::DisplayType::Block) {
		RectF border_rect = block_box_.borderRect();
		PointF pos = p - border_rect.origin();
		if (0.0f <= pos.x && pos.x < border_rect.width()
			&& 0.0f <= pos.y && pos.y < border_rect.height()) {
			return pos;
		}
	}
	return absl::nullopt;
}

void Node::onEvent(MouseEvent& event)
{
	if (control_)
		control_->onMouseEvent(this, event);
}

void Node::onEvent(KeyEvent& event)
{
	if (control_)
		control_->onKeyEvent(this, event);
}

void Node::onEvent(FocusEvent& event)
{
	if (control_)
		control_->onFocusEvent(this, event);
}

void Node::onEvent(ImeEvent& event)
{
	if (control_)
		control_->onImeEvent(this, event);
}

void Node::onAnimationFrame(absl::Time timestamp)
{
	if (control_)
		control_->onAnimationFrame(this, timestamp);
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

void Node::resolveDefaultStyle()
{
	CHECK(type_ == NodeType::NODE_ELEMENT);
	if (tag_ == base::string_intern("img")
		|| tag_ == base::string_intern("span")
		|| tag_ == base::string_intern("strong")
		|| tag_ == base::string_intern("em")
		|| tag_ == base::string_intern("b")) {
		computed_style_.display = style::DisplayType::Inline;
	} else {
		computed_style_.display = style::DisplayType::Block;
	}
	computed_style_.position = style::PositionType::Static;
	CHECK(type_ == NodeType::NODE_ELEMENT);
#define RESOLVE_STYLE_DEFAULT(x, def) \
    computed_style_.x = def

	RESOLVE_STYLE_DEFAULT(margin_left, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(margin_top, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(margin_right, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(margin_bottom, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_left_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_right_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_left_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_right_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_right_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_left_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_left, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_top, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_right, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_bottom, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(left, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(top, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(right, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(bottom, style::Value::auto_());

	RESOLVE_STYLE_DEFAULT(min_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(min_height, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(max_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(max_height, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(height, style::Value::auto_());

	RESOLVE_STYLE_DEFAULT(border_color, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(background_color, style::Value::auto_());
#undef RESOLVE_STYLE_DEFAULT

#define RESOLVE_STYLE_DEFAULT_INHERIT(x, def) \
    computed_style_.x = (parent_ ? parent_->computed_style_.x : def)
	
	RESOLVE_STYLE_DEFAULT_INHERIT(color, style::Value::fromHexColor("#000000"));
	RESOLVE_STYLE_DEFAULT_INHERIT(line_height, style::Value::fromPixel(18));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_family, style::Value::fromKeyword(base::string_intern("Microsoft YaHei")));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_size, style::Value::fromPixel(12));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_style, style::FontStyle::Normal);
	RESOLVE_STYLE_DEFAULT_INHERIT(font_weight, style::FontWeight());
#undef RESOLVE_STYLE_DEFAULT_INHERIT
}

void Node::resolveStyle(const style::StyleSpec& spec)
{
	CHECK(type_ == NodeType::NODE_ELEMENT);
#define RESOLVE_STYLE(x) \
    resolve_style(computed_style_.x, \
        parent_ ? &parent_->computed_style_.x : nullptr, \
        spec.x)
	RESOLVE_STYLE(display);
	RESOLVE_STYLE(position);
	RESOLVE_STYLE(margin_left);
	RESOLVE_STYLE(margin_top);
	RESOLVE_STYLE(margin_right);
	RESOLVE_STYLE(margin_bottom);
	RESOLVE_STYLE(border_left_width);
	RESOLVE_STYLE(border_top_width);
	RESOLVE_STYLE(border_right_width);
	RESOLVE_STYLE(border_bottom_width);
	RESOLVE_STYLE(border_top_left_radius);
	RESOLVE_STYLE(border_top_right_radius);
	RESOLVE_STYLE(border_bottom_right_radius);
	RESOLVE_STYLE(border_bottom_left_radius);
	RESOLVE_STYLE(padding_left);
	RESOLVE_STYLE(padding_top);
	RESOLVE_STYLE(padding_right);
	RESOLVE_STYLE(padding_bottom);
	RESOLVE_STYLE(left);
	RESOLVE_STYLE(top);
	RESOLVE_STYLE(right);
	RESOLVE_STYLE(bottom);

	RESOLVE_STYLE(min_width);
	RESOLVE_STYLE(min_height);
	RESOLVE_STYLE(max_width);
	RESOLVE_STYLE(max_height);
	RESOLVE_STYLE(width);
	RESOLVE_STYLE(height);

	RESOLVE_STYLE(border_color);
	RESOLVE_STYLE(background_color);
	RESOLVE_STYLE(color);
	RESOLVE_STYLE(line_height);
	RESOLVE_STYLE(font_family);
	RESOLVE_STYLE(font_size);
	RESOLVE_STYLE(font_style);
	RESOLVE_STYLE(font_weight);
#undef RESOLVE_STYLE
}

void Node::resolveInlineStyle()
{
	resolveStyle(specStyle_);
	if (type_ == NodeType::NODE_ELEMENT && absolutelyPositioned()
		&& computed_style_.display != style::DisplayType::Block) {
		computed_style_.display = style::DisplayType::Block;
	}
}

bool Node::matchSimple(style::Selector* selector) const
{
	if (type_ != NodeType::NODE_ELEMENT)
		return false;
	if (selector->id != base::string_atom() && selector->id != id_)
		return false;
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

void Node::reflow(float contg_blk_width, float contg_blk_height)
{
	CHECK(type_ == NodeType::NODE_ELEMENT) << "reflow(): expect NODE_ELEMENT";
	CHECK(computed_style_.position == style::PositionType::Absolute || computed_style_.position == style::PositionType::Fixed)
		<< "reflow(): invalid position " << computed_style_.position;

	bfc_.emplace(this);
	bfc_->owner = this;

	LOG(INFO)
		<< "<" << tag_ << "> " << this << " "
		<< "new BFC size " << DimensionF(contg_blk_width, contg_blk_height);

	// Box generation
	auto& b = block_box_;
	style::BlockBoxBuilder bbb(&b);
	eachChild(absl::bind_front(&Node::layoutPrepare, std::ref(*bfc_), std::ref(bbb)));

	const auto& st = computed_style_;
	float clean_contg_width = contg_blk_width
		- try_resolve_to_px(st.border_left_width, contg_blk_width).value_or(0)
		- try_resolve_to_px(st.padding_left, contg_blk_width).value_or(0)
		- try_resolve_to_px(st.padding_right, contg_blk_width).value_or(0)
		- try_resolve_to_px(st.border_right_width, contg_blk_width).value_or(0);
	style::AbsoluteBlockWidthSolver width_solver(
		clean_contg_width,
		try_resolve_to_px(st.left, contg_blk_width),
		try_resolve_to_px(st.margin_left, contg_blk_width),
		try_resolve_to_px(st.width, contg_blk_width),
		try_resolve_to_px(st.margin_right, contg_blk_width),
		try_resolve_to_px(st.right, contg_blk_width));

	// Compute width, left and right margins
	style::WidthConstraint mw = width_solver.measureWidth();
	if (mw.type == style::WidthConstraintType::Fixed) {
		b.content.width = mw.value;
	} else {
		auto [min_content_width, max_content_width] = layoutMeasureWidth(b);
		if (mw.type == style::WidthConstraintType::MinContent) {
			b.content.width = min_content_width;
		} else if (mw.type == style::WidthConstraintType::MaxContent) {
			b.content.width = max_content_width;
		} else if (mw.type == style::WidthConstraintType::FitContent) {
			b.content.width = std::min(max_content_width, std::max(min_content_width, mw.value));
		} else {
			LOG(ERROR) << "BUG: unexpected width constraint type.";
			b.content.width = 0.0f;
		}
	}
	width_solver.setLayoutWidth(b.content.width);

	b.margin.left = width_solver.marginLeft();
	b.border.left = try_resolve_to_px(st.border_left_width, contg_blk_width).value_or(0);
	b.padding.left = try_resolve_to_px(st.padding_left, contg_blk_width).value_or(0);
	b.padding.right = try_resolve_to_px(st.padding_right, contg_blk_width).value_or(0);
	b.border.right = try_resolve_to_px(st.border_right_width, contg_blk_width).value_or(0);
	b.margin.right = width_solver.marginRight();

	// Compute top and bottom margins
	b.margin.top = try_resolve_to_px(st.margin_top, contg_blk_width).value_or(0);
	b.border.top = try_resolve_to_px(st.border_top_width, contg_blk_width).value_or(0);
	b.padding.top = try_resolve_to_px(st.padding_top, contg_blk_width).value_or(0);

	b.prefer_height = try_resolve_to_px(st.height, contg_blk_height);

	b.padding.bottom = try_resolve_to_px(st.padding_bottom, contg_blk_width).value_or(0);
	b.border.bottom = try_resolve_to_px(st.border_bottom_width, contg_blk_width).value_or(0);
	b.margin.bottom = try_resolve_to_px(st.margin_bottom, contg_blk_width).value_or(0);

	eachLayoutChild([this, &bbb](Node* child) {
		layoutMeasure(*bfc_, bbb, child);
		});

	layoutArrange(*bfc_, b);
	b.pos.x = width_solver.left();

	// for absolutely positioned block
	// TODO: improve AbsoluteBlockPositionSolver
	style::AbsoluteBlockPositionSolver height_solver(contg_blk_height,
		try_resolve_to_px(st.top, contg_blk_height),
		try_resolve_to_px(st.height, contg_blk_height),
		try_resolve_to_px(st.bottom, contg_blk_height));
	height_solver.setLayoutHeight(b.marginRect().size().height);
	b.pos.y = height_solver.top();
	//b.content.height = height_solver.height();
	b.content.height = std::max(0.0f, height_solver.height() - b.margin.top - b.margin.bottom);

	LOG(INFO) << "reflow <" << tag_ << "> border box" << b.borderRect();

	// layout out-of-flow
	for (Node* node : bfc_->abs_pos_nodes) {
		// find containing block
		Node* pn = node->parent();
		while (pn) {
			if (pn->type() == NodeType::NODE_ELEMENT && pn->computed_style_.position != style::PositionType::Static) {
				//abs_contg_width = pn->block_box_.padding.left + pn->block_box_.content.width + pn->block_box_.padding.right;
				//abs_contg_height = pn->block_box_.padding.top + pn->block_box_.content.height + pn->block_box_.padding.bottom;
				break;
			}
			pn = pn->parent();
		}

		if (!pn) {
			RectF r = b.paddingRect();
			node->reflow(r.width(), r.height());
		} else if (pn->computed_style_.display == style::DisplayType::Block) {
			RectF r = pn->block_box_.paddingRect();
			node->reflow(r.width(), r.height());
			if (pn->computed_style_.position == style::PositionType::Relative) {
				node->block_box_.pos += pn->block_box_.pos + r.origin();
			}
		} else if (pn->computed_style_.display == style::DisplayType::Inline) {
			RectF r = pn->inline_box_.boundingRect();
			node->reflow(r.width(), r.height());
			if (pn->computed_style_.position == style::PositionType::Relative) {
				node->block_box_.pos += pn->inline_box_.pos + r.origin();
			}
		} else {
			LOG(WARNING) << "TODO: not implemented: absolutely element reflow for parent with display " << pn->computed_style_.display;
		}
	}
}
/*
void Node::layoutText(style::InlineFormatContext& ifc)
{
	CHECK(type_ == NodeType::NODE_TEXT) << "layoutText(): expect NODE_TEXT";
	text_box_.size = text_layout_->rect().size();
	text_box_.baseline = text_layout_->baseline();
	ifc.setupBox(&text_box_);
}
*/
bool Node::positioned() const
{
	return computed_style_.position != style::PositionType::Static;
}

bool Node::relativePositioned() const
{
	return computed_style_.position == style::PositionType::Relative;
}

bool Node::absolutelyPositioned() const
{
	return computed_style_.position != style::PositionType::Static
		&& computed_style_.position != style::PositionType::Relative;
}

Node* Node::absolutelyPositionedParent() const
{
	Node* p = parent_;
	while (p) {
		if (p->positioned() && p->bfc_)
			return p;
		p = p->parent_;
	}
	return nullptr;
}

scene2d::PointF Node::getMousePosition() const
{
	return scene_->getMousePosition();
}

void Node::requestPaint()
{
	scene_->requestPaint();
}

void Node::requestUpdate()
{
	scene_->requestUpdate();
}

void Node::requestAnimationFrame(scene2d::Node* node)
{
	scene_->requestAnimationFrame(node);
}

void Node::updateTextLayout()
{
	text_layout_ = graph2d::createTextLayout(
		text_,
		computed_style_.font_family.keyword_val.c_str(),
		computed_style_.font_style,
		computed_style_.font_weight,
		computed_style_.font_size.pixelOrZero());
}

void Node::layoutPrepare(style::BlockFormatContext& bfc, style::BlockBoxBuilder& bbb, Node* node)
{
	if (node->type() == NodeType::NODE_TEXT) {
		bbb.addText(node);
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		const auto& st = node->computedStyle();
		if (node->absolutelyPositioned()) {
			bfc.abs_pos_nodes.push_back(node);
			return;
		}
		// 'static' or 'relative' positioned
		if (st.display == style::DisplayType::Block) {
			bbb.beginBlock(&node->block_box_);
			Node::eachChild(node, absl::bind_front(&Node::layoutPrepare, std::ref(bfc), std::ref(bbb)));
			bbb.endBlock();
		} else if (st.display == style::DisplayType::Inline) {
			bbb.beginInline(node);
			//Node::eachChild(node, absl::bind_front(&Node::layoutPrepare, std::ref(bbb)));
			bbb.endInline();
		}
	} else {
		Node::eachChild(node, absl::bind_front(&Node::layoutPrepare, std::ref(bfc), std::ref(bbb)));
	}
}

std::tuple<float, float> Node::layoutMeasureWidth(style::BlockBox& box)
{
	if (box.type == style::BlockBoxType::WithBlockChildren) {
		float min_width = 0.0f, max_width = 0.0f;
		box.eachChild([&](style::BlockBox* child) {
			const style::Style& st = *child->style;
			float margin_left = try_resolve_to_px(st.margin_left, absl::nullopt).value_or(0);
			float border_left = try_resolve_to_px(st.border_left_width, absl::nullopt).value_or(0);
			float padding_left = try_resolve_to_px(st.padding_left, absl::nullopt).value_or(0);
			float width = try_resolve_to_px(st.width, absl::nullopt).value_or(0);
			float padding_right = try_resolve_to_px(st.padding_right, absl::nullopt).value_or(0);
			float border_right = try_resolve_to_px(st.border_right_width, absl::nullopt).value_or(0);
			float margin_right = try_resolve_to_px(st.margin_right, absl::nullopt).value_or(0);
			float mbp_width = margin_left + border_left + padding_left + padding_right + border_right + margin_right;
			float child_min_w, child_max_w;
			if (st.width.isPixel() || st.width.isRaw()) {
				min_width = std::max(min_width, mbp_width + width);
				max_width = std::max(max_width, mbp_width + width);
			} else {
				auto [ child_min_w, child_max_w ] = layoutMeasureWidth(*child);
				min_width = std::max(min_width, mbp_width + child_min_w);
				max_width = std::max(max_width, mbp_width + child_max_w);
			}
			});
		return { min_width, max_width };
	} else if (box.type == style::BlockBoxType::WithInlineChildren) {
		Node* contg_node = std::get<Node*>(box.payload);
		float min_width = 0.0f, max_width = 0.0f;
		contg_node->eachLayoutChild([&](Node* child) {
			auto [child_min_w, child_max_w] = layoutMeasureInlineWidth(child);
			min_width = std::max(min_width, child_min_w);
			max_width += child_max_w;
			});
		return { min_width, max_width };
	}
	return { 0.0f, 0.0f };
}

std::tuple<float, float> Node::layoutMeasureInlineWidth(Node* node)
{
	const auto& st = node->computedStyle();
	if (node->type() == NodeType::NODE_TEXT) {
		float w = node->text_layout_->rect().width();
		return { w, w };
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		float min_width = 0.0f, max_width = 0.0f;
		node->eachLayoutChild([&](Node* child) {
			auto [child_min_w, child_max_w] = layoutMeasureInlineWidth(child);
			min_width = std::max(min_width, child_min_w);
			max_width += child_max_w;
			});
		return { min_width, max_width };
	}
	return { 0.0f, 0.0f };
}

void Node::layoutMeasure(style::InlineFormatContext& ifc, style::InlineBoxBuilder& ibb, Node* node)
{
	if (node->type() == NodeType::NODE_TEXT) {
		node->inline_box_.size = node->text_layout_->rect().size();
		node->inline_box_.baseline = node->text_layout_->baseline();
		ibb.addText(node);
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		if (node->absolutelyPositioned()) {
			ifc.bfc().abs_pos_nodes.push_back(node);
		} else {
			ibb.beginInline(&node->inline_box_);
			eachLayoutChild(node, [&](Node* child) {
				layoutMeasure(ifc, ibb, child);
				});
			ibb.endInline();
		}
	} else if (node->type() == NodeType::NODE_COMPONENT) {
		LOG(WARNING) << "BUG: layoutMeasure(NODE_COMPONENT)";
	}
}

void Node::layoutMeasure(style::BlockFormatContext& bfc, style::BlockBoxBuilder& bbb, Node* node)
{
	if (node->type() == NodeType::NODE_TEXT) {
		//bbb.addText(node);
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		const auto& st = node->computedStyle();
		if (node->absolutelyPositioned()) {
			return;
		}
		// 'static' or 'relative' positioned
		if (st.display == style::DisplayType::Block) {

			float contg_width = bbb.containingBlockWidth();
			float clean_contg_width = contg_width
				- try_resolve_to_px(st.border_left_width, contg_width).value_or(0)
				- try_resolve_to_px(st.padding_left, contg_width).value_or(0)
				- try_resolve_to_px(st.padding_right, contg_width).value_or(0)
				- try_resolve_to_px(st.border_right_width, contg_width).value_or(0);
			style::StaticBlockWidthSolver solver(
				clean_contg_width,
				try_resolve_to_px(st.margin_left, contg_width),
				try_resolve_to_px(st.width, contg_width),
				try_resolve_to_px(st.margin_right, contg_width));
			float avail_width = solver.measureWidth();
			solver.setLayoutWidth(avail_width);

			// Compute width, left and right margins
			auto& b = node->block_box_;
			b.content.width = solver.width();
			b.margin.left = solver.marginLeft();
			b.border.left = try_resolve_to_px(st.border_left_width, contg_width).value_or(0);
			b.padding.left = try_resolve_to_px(st.padding_left, contg_width).value_or(0);
			b.padding.right = try_resolve_to_px(st.padding_right, contg_width).value_or(0);
			b.border.right = try_resolve_to_px(st.border_right_width, contg_width).value_or(0);
			b.margin.right = solver.marginRight();

			// Compute height, top and bottom margins
			absl::optional<float> base_height = bbb.containingBlockHeight();
			b.margin.top = try_resolve_to_px(st.margin_top, contg_width).value_or(0);
			b.border.top = try_resolve_to_px(st.border_top_width, contg_width).value_or(0);
			b.padding.top = try_resolve_to_px(st.padding_top, contg_width).value_or(0);

			b.prefer_height = try_resolve_to_px(st.height, base_height);

			b.padding.bottom = try_resolve_to_px(st.padding_bottom, contg_width).value_or(0);
			b.border.bottom = try_resolve_to_px(st.border_bottom_width, contg_width).value_or(0);
			b.margin.bottom = try_resolve_to_px(st.margin_bottom, contg_width).value_or(0);

			bbb.beginBlock(&node->block_box_);
			Node::eachLayoutChild(node, [&bfc, &bbb](Node* child) {
				layoutMeasure(bfc, bbb, child);
				});
			bbb.endBlock();
		} else if (st.display == style::DisplayType::Inline) {
			bbb.beginInline(node);
			/*
			Node::eachLayoutChild(node, [&bfc, &bbb](Node* child) {
				layoutMeasure(bfc, bbb, child);
				});
			*/
			bbb.endInline();
		}
	}
}

void Node::layoutArrange(style::BlockFormatContext& bfc, style::BlockBox& box)
{
	float borpad_top = box.border.top + box.padding.top;
	float borpad_bottom = box.border.bottom + box.padding.bottom;

	box.pos.x = bfc.content_left;
	if (borpad_top > 0) {
		float coll_margin = style::collapse_margin(box.margin.top,
			(bfc.margin_bottom - bfc.border_bottom));
		box.pos.y = bfc.border_bottom + coll_margin - box.margin.top;
		bfc.border_bottom = bfc.border_bottom + coll_margin + borpad_top;
		bfc.margin_bottom = bfc.border_bottom;
	} else {
		float coll_margin = style::collapse_margin(box.margin.top,
			(bfc.margin_bottom - bfc.border_bottom));
		box.pos.y = bfc.border_bottom + coll_margin - box.margin.top;
		//bfc.border_bottom += coll_margin + borpad_top;
		bfc.margin_bottom = bfc.border_bottom + coll_margin;
	}

	base::scoped_setter _(bfc.content_left,
		bfc.content_left + box.margin.left + box.border.left + box.padding.left);
	if (box.type == style::BlockBoxType::WithBlockChildren || box.type == style::BlockBoxType::Empty) {
		float saved_bfc_margin_bottom = bfc.margin_bottom;
		box.eachChild([&](style::BlockBox* child) {
			Node::layoutArrange(bfc, *child);
			});
		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom = saved_bfc_margin_bottom + *box.prefer_height;
			bfc.margin_bottom = bfc.border_bottom;
		} else {
			// Compute 'auto' height
			if (borpad_bottom > 0) {
				box.content.height = std::max(0.0f,
					bfc.margin_bottom - box.pos.y - box.margin.top - borpad_top);
			} else {
				box.content.height = std::max(0.0f,
					bfc.border_bottom - box.pos.y - box.margin.top - borpad_top);
			}
		}
	} else if (box.type == style::BlockBoxType::WithInlineChildren) {
		Node* contg_node = std::get<Node*>(box.payload);
		contg_node->ifc_.emplace(bfc, bfc.content_left, box.content.width);
		LOG(INFO)
			<< "<" << contg_node->tag_ << "> "
			<< "begin IFC pos=" << PointF(bfc.content_left, bfc.margin_bottom)
			<< ", bfc_bottom=" << bfc.border_bottom << ", " << bfc.margin_bottom;
		style::InlineBoxBuilder ibb(*contg_node->ifc_, &contg_node->inline_box_);
		contg_node->eachLayoutChild([&](Node* child) {
			layoutMeasure(*contg_node->ifc_, ibb, child);
			});
		contg_node->ifc_->layoutArrange();
		if (box.prefer_height.has_value()) {
			box.content.height = *box.prefer_height;
			bfc.border_bottom = bfc.margin_bottom + *box.prefer_height;
			bfc.margin_bottom = bfc.border_bottom;
		} else {
			box.content.height = contg_node->ifc_->getLayoutHeight();
			if (box.content.height > 0) {
				bfc.border_bottom = bfc.margin_bottom + box.content.height;
				bfc.margin_bottom = bfc.border_bottom;
			} else {
				// check top and bottom margin collapse, below
			}
		}
		LOG(INFO)
			<< "<" << contg_node->tag_ << "> "
			<< "end IFC size=" << box.content
			<< ", bfc_bottom=" << bfc.border_bottom << ", " << bfc.margin_bottom;
	}

	LOG(INFO) << "layout arrange box pos " << box.pos << ", border-rect: " << box.borderRect();

	if (borpad_bottom > 0) {
		bfc.border_bottom = bfc.margin_bottom + borpad_bottom;
		bfc.margin_bottom = bfc.border_bottom + box.margin.bottom;
	} else {
		float coll_margin = style::collapse_margin(box.margin.bottom,
			(bfc.margin_bottom - bfc.border_bottom));
		bfc.margin_bottom = bfc.border_bottom + coll_margin;
	}
}

void resolve_style(style::DisplayType& style, const style::DisplayType* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("block"))
				style = style::DisplayType::Block;
			else if (spec.value->keyword_val == base::string_intern("inline"))
				style = style::DisplayType::Inline;
			else if (spec.value->keyword_val == base::string_intern("inline-block"))
				style = style::DisplayType::InlineBlock;
		}
	}
}

void resolve_style(style::PositionType& style, const style::PositionType* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
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
		}
	}
}

void resolve_style(style::FontStyle& style, const style::FontStyle* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("normal"))
				style = style::FontStyle::Normal;
			else if (spec.value->keyword_val == base::string_intern("italic"))
				style = style::FontStyle::Italic;
		}
	}
}

void resolve_style(style::FontWeight& style, const style::FontWeight* parent,
	const style::ValueSpec& spec)
{
	if (spec.type == style::ValueSpecType::Inherit) {
		if (parent)
			style = *parent;
	} else if (spec.type == style::ValueSpecType::Specified) {
		if (spec.value->unit == style::ValueUnit::Keyword) {
			if (spec.value->keyword_val == base::string_intern("normal")) {
				style = style::FontWeight(400);
			} else if (spec.value->keyword_val == base::string_intern("bold")) {
				style = style::FontWeight(600);
			} else if (spec.value->keyword_val == base::string_intern("lighter")) {
				if (style.raw() <= 500) {
					style = style::FontWeight(100);
				} else if (style.raw() <= 700) {
					style = style::FontWeight(400);
				} else {
					style = style::FontWeight(400);
				}
			} else if (spec.value->keyword_val == base::string_intern("bolder")) {
				if (style.raw() <= 300) {
					style = style::FontWeight(400);
				} else if (style.raw() <= 500) {
					style = style::FontWeight(700);
				} else {
					style = style::FontWeight(900);
				}
			}
		}
	}
}

}
