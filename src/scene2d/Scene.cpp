#include "Scene.h"
#include "Node.h"
#include "script/script.h"
#include "style/style.h"
#include "style/Layout.h"
#include "style/StylePaint.h"
#include "style/LayoutObject.h"
#include "absl/functional/bind_front.h"
#include "absl/cleanup/cleanup.h"
#include "graph2d/Painter.h"

namespace scene2d {

Scene::Scene(EventContext& ctx)
	: event_ctx_(ctx)
{
	script_ctx_ = std::make_unique<script::Context>();
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
	auto actor = new Node(this, NodeType::NODE_TEXT, text);
	return actor;
}

Node* Scene::createElementNode(base::string_atom tag)
{
	auto actor = new Node(this, NodeType::NODE_ELEMENT, tag);
	return actor;
}

Node* Scene::createComponentNode(JSValue comp_data)
{
	JSContext* jctx = script_ctx_->get();
	if (JS_IsString(comp_data)) {
		std::string text = script_ctx_->parse<std::string>(comp_data);
		//LOG(INFO) << "createTextNode: " << text;
		return createTextNode(text);
	} else if (JS_IsArray(jctx, comp_data)) {
		//LOG(INFO) << "createElementNode: " << "fragment";
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
		absl::Cleanup _ = [&]() { JS_FreeValue(jctx, render); };
		if (JS_IsFunction(jctx, render)) {
			//LOG(INFO) << "createComponentNode";
			Node* node = createComponentNodeWithState(comp_data);
			JS_SetOpaque(comp_data, node->weakProxy());

			JSValue child_comp_data = JS_Call(jctx, render, comp_data, 0, nullptr);
			node->appendChild(createComponentNode(child_comp_data));
			return node;
		} else {
			JSValue tag = JS_GetPropertyStr(jctx, comp_data, "tag");
			JSValue atts = JS_GetPropertyStr(jctx, comp_data, "atts");
			JSValue kids = JS_GetPropertyStr(jctx, comp_data, "kids");
			absl::Cleanup _ = [&]() {
				JS_FreeValue(jctx, tag);
				JS_FreeValue(jctx, atts);
				JS_FreeValue(jctx, kids);
				};
			if (JS_IsString(tag) && JS_IsObject(atts) && JS_IsArray(jctx, kids)) {
				std::string tagName = script_ctx_->parse<std::string>(tag);
				//LOG(INFO) << "createElementNode: " << tagName;
				Node* node = createElementNode(base::string_intern(tagName));

				// Setup properties
				setupProps(node, atts);

				// Create children
				int64_t length = 0;
				JS_GetPropertyLength(jctx, &length, kids);
				for (uint32_t i = 0; i < (uint32_t)length; ++i) {
					JSValue child_comp_data = JS_GetPropertyUint32(jctx, kids, i);
					Node* child = createComponentNode(child_comp_data);
					JS_FreeValue(jctx, child_comp_data);

					node->appendChild(child);
				}

				return node;
			} else {
				auto a = JS_JSONStringify(jctx, comp_data, JS_UNDEFINED, JS_UNDEFINED);
				LOG(WARNING) << "Unknown: " << script_ctx_->parse<std::string>(a);
				JS_FreeValue(jctx, a);
				return nullptr;
			}
		}
	}
	LOG(WARNING) << "Unknown: " << script_ctx_->parse<std::string>(comp_data);
	return nullptr;
}

void Scene::updateComponent(JSValue comp_state)
{

}

Node* Scene::pickNode(const PointF& pos, int flag_mask, PointF* out_local_pos)
{
	//return pickNode(root_, pos, flag_mask, out_local_pos);
	for (auto it = flow_roots_.rbegin(); it != flow_roots_.rend(); ++it) {
		style::LayoutObject* o = style::LayoutObject::pick(*it, pos, flag_mask, out_local_pos);
		if (o)
			return o->node;
	}
	return nullptr;
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

void Scene::resolveStyle()
{
	resolveNodeStyle(root_);
}

void Scene::computeLayout(const scene2d::DimensionF& size)
{
	/*
	root_->computed_style_.display = style::DisplayType::Block;
	root_->computed_style_.position = style::PositionType::Absolute;
	root_->computed_style_.width = style::Value::fromPixel(size.width);
	root_->computed_style_.height = style::Value::fromPixel(size.height);
	root_->reflowAbsolutelyPositioned(size.width, size.height);
	*/
	style::LayoutTreeBuilder b(root_);
	flow_roots_ = b.build();
	for (auto& flow_root : flow_roots_) {
		style::LayoutObject::reflow(flow_root, size);
	}
}

void Scene::paint(graph2d::PainterInterface* painter)
{
	// positioned_layout -> (offset, parent_positioned_layout)
	std::map<style::LayoutObject*, std::tuple<PointF, style::LayoutObject*>> offsets;
	for (auto& fl : flow_roots_) {
		if (fl.positioned_parent) {
			PointF offset;
			auto it = offsets.find(fl.positioned_parent);
			while (it != offsets.end()) {
				const auto& [off, po] = it->second;
				offset += off;
				it = offsets.find(po);
			}
			painter->setTranslation(offset, false);
		}
		
		style::LayoutObject::paint(fl.root, painter);
		
		offsets[fl.root] = { style::LayoutObject::getOffset(fl.root), fl.positioned_parent };
		for (style::LayoutObject*o : fl.relatives_) {
			offsets[o] = { style::LayoutObject::getOffset(o), fl.root };
		}

		if (fl.positioned_parent) {
			painter->restore();
		}
	}
}

scene2d::PointF Scene::getMousePosition() const
{
	return event_ctx_.GetMousePosition();
}

void Scene::requestPaint()
{
	event_ctx_.RequestPaint();
}

void Scene::requestUpdate()
{
	event_ctx_.RequestUpdate();
}

void Scene::requestAnimationFrame(scene2d::Node* node)
{
	event_ctx_.RequestAnimationFrame(node);
}

PointF Scene::mapPointToScene(Node* node, const PointF& pos) const
{
	if (node->scene_ != this) {
		LOG(WARNING) << "failed to map point to this scene.";
		return pos;
	}
	PointF p = pos;
	if (node->computed_style_.display == style::DisplayType::Block) {
		p += node->block_box_.pos + node->block_box_.borderRect().origin();
	} else if (node->computed_style_.display == style::DisplayType::Inline) {
		p += node->inline_box_.boundingRect().origin();
	}

	Node* pn = node->positionedAncestor();
	while (pn) {
		if (pn->computed_style_.display == style::DisplayType::Block) {
			p += pn->block_box_.pos;
		} else if (node->computed_style_.display == style::DisplayType::Inline) {
			p += pn->inline_box_.boundingRect().origin();
		}
		pn = pn->positionedAncestor();
	}
	return p;
}

Node* Scene::pickSelf(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos)
{
	if (!node->visible())
		return nullptr;
	if (node->testFlags(flag_mask)) {
		auto local_pos = node->hitTestNode(pos);
		if (local_pos) {
			if (out_local_pos) {
				auto kk = node->hitTestNode(pos);
				*out_local_pos = *local_pos;
			}
			return node;
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}

Node* Scene::createComponentNodeWithState(JSValue comp_state)
{
	return new Node(this, NodeType::NODE_COMPONENT, comp_state);
}

void Scene::setupProps(Node* node, JSValue props)
{
	script_ctx_->eachObjectField(props, [&](const char* name_str, JSValue value) {
		JSContext* jctx = script_ctx_->get();
		auto name = base::string_intern(name_str);
		if (name == base::string_intern("style")) {
			node->setStyle(script_ctx_->parse<style::StyleSpec>(value));
		} else if (name == base::string_intern("id")) {
			node->setId(script_ctx_->parse<base::string_atom>(value));
		} else if (name == base::string_intern("class")) {
			const char* s = JS_ToCString(jctx, value);
			if (s) {
				node->setClass(style::Classes::parse(s));
			}
			JS_FreeCString(jctx, s);
		} else if (JS_IsFunction(script_ctx_->get(), value)) {
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

void Scene::resolveNodeStyle(Node* node)
{
	if (node->type() == NodeType::NODE_COMPONENT) {
		node->computed_style_ = node->parent_->computed_style_;
		node->eachChild(absl::bind_front(&Scene::resolveNodeStyle, this));
	} else if (node->type() == NodeType::NODE_ELEMENT) {
		node->resolveDefaultStyle();
		for (auto& rule : style_rules_) {
			if (match(node, rule->selector.get())) {
				node->resolveStyle(rule->spec);
			}
		}
		node->resolveInlineStyle();
		node->eachChild(absl::bind_front(&Scene::resolveNodeStyle, this));
	} else if (node->type() == NodeType::NODE_TEXT) {
		node->resolveDefaultStyle();
		node->updateTextLayout();
	}
}

void Scene::paintNode(Node* node, style::BlockPaintContext& bpc, graph2d::PainterInterface* painter)
{
	if (node->type_ == NodeType::NODE_TEXT) {
		LOG(INFO) << "paintText scene pos=" << node->inline_box_.pos << ", text=" << node->text_;
		size_t n = node->text_boxes_.glyph_runs.size();
		for (size_t i = 0; i < n; ++i) {
			style::InlineBox* ibox = node->text_boxes_.inline_boxes[i].get();
			graph2d::GlyphRunInterface* gr = node->text_boxes_.glyph_runs[i].get();
			PointF baseline_origin = ibox->pos;
			baseline_origin.y += ibox->baseline;
			painter->drawGlyphRun(baseline_origin, gr, node->computed_style_.color);
		}
	} else if (node->type_ == NodeType::NODE_ELEMENT) {
		//PointF contg_block_pos = node->absolutelyPositioned()
		//	? bpc.positioned_contg_pos
		//	: bpc.contg_pos;
		if (node->computed_style_.display == style::DisplayType::Block) {
			LOG(INFO) << "paintBlock scene pos=" << node->block_box_.pos
				<< ", border-rect " << node->block_box_.borderRect();
			RectF border_rect = node->block_box_.borderRect();
			RectF render_rect = RectF::fromXYWH(
				node->block_box_.pos.x + border_rect.left,
				node->block_box_.pos.y + border_rect.top,
				border_rect.width(),
				border_rect.height());
			painter->drawBox(
				render_rect,
				node->computed_style_.border_top_width.pixelOrZero(),
				node->computed_style_.background_color,
				node->computed_style_.border_color);
			if (node->control_)
				painter->drawControl(render_rect, node->control_.get());
		} else if (node->computed_style_.display == style::DisplayType::Inline) {
		} else if (node->computed_style_.display == style::DisplayType::InlineBlock) {
			LOG(INFO) << "paintInlineBlock scene pos=" << node->inline_box_.pos
				<< ", border-rect " << node->block_box_.borderRect();
			RectF border_rect = node->block_box_.borderRect();
			RectF render_rect = RectF::fromXYWH(
				node->inline_box_.pos.x + border_rect.left,
				node->inline_box_.pos.y + border_rect.top,
				border_rect.width(),
				border_rect.height());
			painter->drawBox(
				render_rect,
				node->computed_style_.border_top_width.pixelOrZero(),
				node->computed_style_.background_color,
				node->computed_style_.border_color);
			if (node->control_)
				painter->drawControl(render_rect, node->control_.get());
		} else if (node->computed_style_.display == style::DisplayType::None) {
			;
		} else {
			LOG(WARNING) << "paintNode: " << node->computed_style_.display << " not implemented.";
		}
		
		bool need_restore = false;
		if (node->absolutelyPositioned() && node->computed_style_.display == style::DisplayType::Block) {
			need_restore = true;
			painter->save();
			PointF pos = node->block_box_.pos;
			//LOG(INFO) << "paint abs_pos_block setTranslation " << pos;
			painter->setTranslation(pos, true);
			//offset = node->inline_box_.boundingRect().origin();
		} else if (node->computed_style_.display == style::DisplayType::InlineBlock) {
			need_restore = true;
			painter->save();
			PointF pos = node->inline_box_.pos;
			//LOG(INFO) << "paint inline_block setTranslation " << pos;
			painter->setTranslation(pos, true);
		}
		Node::eachLayoutChild(node, [&](Node* child) {
			paintNode(child, bpc, painter);
			});
		if (need_restore)
			painter->restore();
	} else if (node->type_ == NodeType::NODE_COMPONENT) {
		Node::eachLayoutChild(node, [&](Node* child) {
			paintNode(child, bpc, painter);
			});
	} else {
		LOG(WARNING) << "Unsupported paint node type" << node->type_;
	}
}

void Scene::paintNode(Node* node, graph2d::PainterInterface* painter)
{
	style::LayoutObject::paint(&node->layout_, painter);
}
Node* Scene::pickNode(Node* node, const PointF& pos, int flag_mask, PointF* out_local_pos/* = nullptr */)
{
	if (!node->visible()) {
		return nullptr;
	}
	
	scene2d::PointF local_pos = pos;
	if (node->type() == NodeType::NODE_ELEMENT && node->absolutelyPositioned()
		&& node->computedStyle().display == style::DisplayType::Block) {
		local_pos -= node->block_box_.pos;
	}
	if (node->type() == NodeType::NODE_ELEMENT
		&& node->computedStyle().display == style::DisplayType::InlineBlock) {
		local_pos -= node->inline_box_.pos;
	}
	const auto& children = node->children_;
	for (auto it = children.rbegin(); it != children.rend(); ++it) {
		Node* node = pickNode(*it, local_pos, flag_mask, out_local_pos);
		if (node) {
			if (out_local_pos) {
				int kk = 1;
			}
			return node;
		}
	}
	
	return pickSelf(node, pos, flag_mask, out_local_pos);
}

} // namespace scene2d
