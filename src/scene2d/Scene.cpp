#include "Scene.h"
#include "Node.h"
#include "script/script.h"
#include "style/style.h"
#include "Layout.h"

namespace scene2d {

Scene::Scene()
{
	ctx_ = std::make_unique<script::Context>();
	root_ = createElementNode(base::string_intern("kml"));
	root_->retain();
	weakptr_ = new base::WeakObjectProxy<Scene>(this);
	weakptr_->retain();
}

Scene::~Scene()
{
	weakptr_->clear();
	weakptr_->release();
	if (root_) {
		root_->release();
		root_ = nullptr;
	}
}

Node* Scene::createTextNode(const std::string& text)
{
	auto actor = new Node(NodeType::NODE_TEXT, text);
	actor->retain();
	return actor;
}

Node* Scene::createElementNode(base::string_atom tag)
{
	auto actor = new Node(NodeType::NODE_ELEMENT, tag);
	actor->retain();
	return actor;
}

Node* Scene::createComponentNode(JSValue comp_data)
{
	JSContext* jctx = ctx_->get();
	if (JS_IsString(comp_data)) {
		std::string text = ctx_->parse<std::string>(comp_data);
		LOG(INFO) << "createTextNode: " << text;
		return createTextNode(text);
	} else if (JS_IsArray(jctx, comp_data)) {
		LOG(INFO) << "createElementNode: " << "fragment";
		Node* node = createElementNode(base::string_intern("fragment"));
		int64_t length = 0;
		JS_GetPropertyLength(jctx, &length, comp_data);
		for (uint32_t i = 0; i < (uint32_t)length; ++i) {
			JSValue child_comp_data = JS_GetPropertyUint32(jctx, comp_data, i);
			Node* child = createComponentNode(child_comp_data);
			node->appendChild(child);
		}
		return node;
	} else if (JS_IsObject(comp_data)) {
		JSValue render = JS_GetPropertyStr(jctx, comp_data, "render");
		if (JS_IsFunction(jctx, render)) {
			LOG(INFO) << "createComponentNode";
			Node* node = createComponentNodeWithState(comp_data);
			JS_SetOpaque(comp_data, node->weakProxy());

			JSValue child_comp_data = JS_Call(jctx, render, comp_data, 0, nullptr);
			node->appendChild(createComponentNode(child_comp_data));
			return node;
		} else {
			JSValue tag = JS_GetPropertyStr(jctx, comp_data, "tag");
			JSValue atts = JS_GetPropertyStr(jctx, comp_data, "atts");
			JSValue kids = JS_GetPropertyStr(jctx, comp_data, "kids");
			if (JS_IsString(tag) && JS_IsObject(atts) && JS_IsArray(jctx, kids)) {
				std::string tagName = ctx_->parse<std::string>(tag);
				LOG(INFO) << "createElementNode: " << tagName;
				Node* node = createElementNode(base::string_intern(tagName));

				// Setup properties
				setupProps(node, atts);

				// Create children
				int64_t length = 0;
				JS_GetPropertyLength(jctx, &length, kids);
				for (uint32_t i = 0; i < (uint32_t)length; ++i) {
					JSValue child_comp_data = JS_GetPropertyUint32(jctx, kids, i);
					Node* child = createComponentNode(child_comp_data);
					node->appendChild(child);
				}

				return node;
			} else {
				auto a = JS_JSONStringify(jctx, comp_data, JS_UNDEFINED, JS_UNDEFINED);
				LOG(WARNING) << "Unknown: " << ctx_->parse<std::string>(a);
				return nullptr;
			}
		}
	}
	LOG(WARNING) << "Unknown: " << ctx_->parse<std::string>(comp_data);
	return nullptr;
}

void Scene::updateComponent(JSValue comp_state)
{

}

Node* Scene::pickNode(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos/* = nullptr */)
{
	if (!node->visible()) {
		return nullptr;
	}
	scene2d::PointF local_pos = pos - node->origin();
	const auto& children = node->children_;
	for (auto it = children.rbegin(); it != children.rend(); ++it) {
		Node* node = pickNode(*it, local_pos, flag_mask, out_local_pos);
		if (node)
			return node;
	}
	return pickSelf(node, pos, flag_mask, out_local_pos);
}

void Scene::appendStyleRule(std::unique_ptr<style::StyleRule>&& rule)
{
	if (style_rules_.empty()) {
		style_rules_.emplace_back(std::move(rule));
		return;
	}
	auto sp = rule->specificity();
	if (style_rules_.back()->specificity() <= sp) {
		style_rules_.emplace_back(std::move(rule));
		return;
	}
	for (auto it = style_rules_.begin(); it != style_rules_.end(); ++it) {
		if (sp < (*it)->specificity()) {
			style_rules_.insert(it, std::move(rule));
			break;
		}
	}
}

void Scene::resolveNodeStyle(Node* node)
{
	for (auto& rule : style_rules_) {
		if (match(node, rule->selector.get()))
			node->resolveStyle(rule->spec);
	}
	node->resolveInlineStyle();
}

void Scene::computeLayout(const scene2d::DimensionF& size)
{
	BlockFormatContext bfc;
	bfc.contg_block_size = size;
	bfc.abs_pos_parent = nullptr;
	layoutChildBlock(root_, bfc);
}

Node* Scene::pickSelf(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos)
{
	if (!node->visible())
		return nullptr;
	if (node->testFlags(flag_mask) && node->hitTestNode(pos)) {
		if (out_local_pos)
			*out_local_pos = pos - node->origin();
		return node;
	} else {
		return nullptr;
	}

}

Node* Scene::createComponentNodeWithState(JSValue comp_state)
{
	return new Node(NodeType::NODE_COMPONENT, comp_state);
}

void Scene::setupProps(Node* node, JSValue props)
{
	ctx_->eachObjectField(props, [&](const char* name_str, JSValue value) {
		JSContext* jctx = ctx_->get();
		auto name = base::string_intern(name_str);
		if (name == base::string_intern("style")) {
			node->setStyle(ctx_->parse<style::StyleSpec>(value));
		} else if (name == base::string_intern("id")) {
			node->setId(ctx_->parse<base::string_atom>(value));
		} else if (name == base::string_intern("class")) {
		} else if (JS_IsFunction(ctx_->get(), value)) {
			node->setEventHandler(name, JS_DupValue(jctx, value));
		} else if (JS_IsNumber(value)) {
			double f64;
			JS_ToFloat64(jctx, &f64, value);
			node->setAttribute(name, (float)f64);
		} else if (JS_IsString(value)) {
			const char* s = JS_ToCString(jctx, value);
			node->setAttribute(name, std::string(s));
			JS_FreeCString(jctx, s);
		} else {
			JSValue jval = JS_ToString(jctx, value);
			const char* s = JS_ToCString(jctx, jval);
			node->setAttribute(name, std::string(s));
			JS_FreeCString(jctx, s);
			JS_FreeValue(jctx, jval);
		}
		});
}

bool Scene::match(Node* node, style::Selector* selector)
{
	if (!node)
		return false;

	if (!node->matchSimple(selector))
		return false;

	if (!selector->dep_selector)
		return true;

	if (selector->dep_type == style::SelectorDependency::DirectParent)
		return match(node->parent_, selector->dep_selector.get());

	if (selector->dep_type == style::SelectorDependency::Ancestor) {
		Node* pnode = node->parent_;
		style::Selector* psel = selector->dep_selector.get();
		while (pnode) {
			if (match(pnode, psel))
				return true;
			pnode = pnode->parent_;
		}
		return false;
	}

	return true;
}

void Scene::layoutBlock(Node* node, BlockFormatContext& bfc)
{
	BlockBox blkb;
	const style::Style& st = node->computed_style_;

	/* Compute box width and margin. */
	style::Value left = st.left;
	style::Value margin_left = st.margin_left;
	style::Value border_left = st.border_left_width;
	style::Value padding_left = st.padding_left;
	style::Value width = st.width;
	style::Value right = st.right;
	style::Value margin_right = st.margin_right;
	style::Value border_right = st.border_right_width;
	style::Value padding_right = st.padding_right;

	float measure_width = bfc.contg_block_size.width;
	if (st.position == style::PositionType::Static) {
		/* 10.3.3 Block-level, non-replaced elements in normal flow */
		left = style::Value::fromPixel(0);
		try_convert_to_px(margin_left, bfc.contg_block_size.width);
		try_convert_to_px(border_left, bfc.contg_block_size.width);
		try_convert_to_px(padding_left, bfc.contg_block_size.width);
		try_convert_to_px(width, bfc.contg_block_size.width);
		try_convert_to_px(padding_right, bfc.contg_block_size.width);
		try_convert_to_px(border_right, bfc.contg_block_size.width);
		try_convert_to_px(margin_right, bfc.contg_block_size.width);
		right = style::Value::fromPixel(0);

		if (border_left.isAuto()) border_left = style::Value::fromPixel(0);
		if (padding_left.isAuto()) padding_left = style::Value::fromPixel(0);
		if (padding_right.isAuto()) padding_right = style::Value::fromPixel(0);
		if (border_right.isAuto()) border_right = style::Value::fromPixel(0);

		measure_width -= margin_left.pixelOrZero();
		measure_width -= border_left.pixelOrZero();
		measure_width -= padding_left.pixelOrZero();
		measure_width -= padding_right.pixelOrZero();
		measure_width -= border_right.pixelOrZero();
		measure_width -= margin_right.pixelOrZero();
	} else if (st.position == style::PositionType::Absolute) {
		/* 10.3.7 Absolutely positioned, non-replaced elements */
		try_convert_to_px(left, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(margin_left, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(border_left, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(padding_left, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(width, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(padding_right, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(border_right, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(margin_right, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(right, bfc.abs_pos_parent_block_size.width);

		if (border_left.isAuto()) border_left = style::Value::fromPixel(0);
		if (padding_left.isAuto()) padding_left = style::Value::fromPixel(0);
		if (padding_right.isAuto()) padding_right = style::Value::fromPixel(0);
		if (border_right.isAuto()) border_right = style::Value::fromPixel(0);

		measure_width -= left.pixelOrZero();
		measure_width -= right.pixelOrZero();
		measure_width -= margin_left.pixelOrZero();
		measure_width -= border_left.pixelOrZero();
		measure_width -= padding_left.pixelOrZero();
		measure_width -= padding_right.pixelOrZero();
		measure_width -= border_right.pixelOrZero();
		measure_width -= margin_right.pixelOrZero();
	} else if (st.position == style::PositionType::Fixed) {
		/* 10.3.7 Absolutely positioned, non-replaced elements */
		try_convert_to_px(left, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(margin_left, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(border_left, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(padding_left, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(width, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(padding_right, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(border_right, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(margin_right, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(right, bfc.abs_pos_parent_block_size.width);

		if (border_left.isAuto()) border_left = style::Value::fromPixel(0);
		if (padding_left.isAuto()) padding_left = style::Value::fromPixel(0);
		if (padding_right.isAuto()) padding_right = style::Value::fromPixel(0);
		if (border_right.isAuto()) border_right = style::Value::fromPixel(0);

		measure_width -= left.pixelOrZero();
		measure_width -= right.pixelOrZero();
		measure_width -= margin_left.pixelOrZero();
		measure_width -= border_left.pixelOrZero();
		measure_width -= padding_left.pixelOrZero();
		measure_width -= padding_right.pixelOrZero();
		measure_width -= border_right.pixelOrZero();
		measure_width -= margin_right.pixelOrZero();
	}
	if (measure_width < 0)
		measure_width = 0;

	// Comput top offset
	style::Value top = st.top;
	style::Value margin_top = st.margin_top;
	if (st.position == style::PositionType::Static) {
		top = style::Value::fromPixel(0);
		try_convert_to_px(margin_top, bfc.contg_block_size.width);
	} else if (st.position == style::PositionType::Absolute) {
		try_convert_to_px(top, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(margin_top, bfc.abs_pos_parent_block_size.width);
	} else if (st.position == style::PositionType::Fixed) {
		try_convert_to_px(top, bfc.abs_pos_parent_block_size.width);
		try_convert_to_px(margin_top, bfc.abs_pos_parent_block_size.width);
	}
	blkb.offset.y = bfc.offset_y + top.pixelOrZero();

	float layout_width = 0;
	float layout_height = 0;
	if (!node->anyBlockChildren()) {
		InlineFormatContext ifc(measure_width);
		for (auto child : node->children())
			layoutInlineChild(child, ifc);
		ifc.layout();
		layout_height = ifc.getLayoutHeight();
		layout_width = ifc.getLayoutWidth();
	} else if (st.position == style::PositionType::Static) {
		layout_width = measure_width;
		for (auto child : node->children()) {
			layoutChildBlock(child, bfc);
		}
	} else {
		BlockFormatContext new_bfc;
		new_bfc.contg_block_size.width = measure_width;
		new_bfc.abs_pos_parent = node;
		new_bfc.abs_pos_parent_block_size.width = measure_width;
		for (auto child : node->children()) {
			layoutChildBlock(child, new_bfc);
		}
	}
	
	//BlockBox blkb;
	//blkb.offset.y = bfc.offset_y;
	//blkb.box.content_size.height = layout_height;
	//node->layoutBlockElement(blkb);

	bfc.offset_y += layout_height;
}

void Scene::layoutChildBlock(Node* node, BlockFormatContext& bfc)
{
	if (node->type() == NodeType::NODE_COMPONENT) {
		for (auto child : node->children())
			layoutChildBlock(child, bfc);
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		if (node->computedStyle().display == style::DisplayType::Block) {
			node->layoutBlockElement(bfc, 0);
			bfc.addBox(&node->block_box_);
		}
	}
}

void Scene::layoutInlineChild(Node* node, InlineFormatContext& ifc)
{
	if (node->type() == NodeType::NODE_TEXT) {
		node->layoutText(ifc);
		ifc.addBox(&node->text_box_);
	} else if (node->type() == NodeType::NODE_COMPONENT) {
		for (auto child : node->children())
			layoutInlineChild(child, ifc);
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		node->layoutInlineElement(ifc, 0);
		for (InlineBox& box : node->inline_boxes_)
			ifc.addBox(&box);
	}
}

} // namespace scene2d
