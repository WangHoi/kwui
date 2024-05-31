#include "DialogAndroid.h"
#include "scene2d/Scene.h"
#include "absl/strings/str_format.h"
#include "include/core/SkSurface.h"
#include "Application_jni.h"
#include "absl/functional/bind_front.h"
#include <algorithm>

namespace android {

static DialogAndroid* g_first_dialog = nullptr;
static std::unordered_map<std::string, DialogAndroid*> g_dialog_map;

DialogAndroid::DialogAndroid()
{
	if (!g_first_dialog) {
		g_first_dialog = this;
	}
	id_ = absl::StrFormat("%p", this);
	g_dialog_map[id_] = this;
	scene_ = std::make_unique<scene2d::Scene>(*this);
	android::create_dialog(id_);
}
DialogAndroid::~DialogAndroid()
{
	if (animation_timer_id_) {
        android::stop_timer(animation_timer_id_);
        animation_timer_id_ = 0;
        animating_nodes_.clear();
    }

	g_dialog_map.erase(id_);
	if (g_first_dialog == this) {
		g_first_dialog = nullptr;
	}
	android::release_dialog(id_);
}

DialogAndroid* DialogAndroid::firstDialog()
{
	return g_first_dialog;
}

std::string DialogAndroid::eventContextId() const
{
	return id_;
}

scene2d::PointF DialogAndroid::GetMousePosition() const
{
	return scene2d::PointF();
}

void DialogAndroid::RequestPaint()
{
	paint();
}

void DialogAndroid::RequestUpdate()
{
	scene_->resolveStyle();
	scene_->computeLayout(size_.width, size_.height);

	//POINT point;
	//if (GetCursorPos(&point) && ScreenToClient(hwnd_, &point)) {
	//	mouse_position_ = scene2d::PointF((float)point.x, (float)point.y);
	//	mouse_position_ /= dpi_scale_;
	//} else {
	//	mouse_position_ = scene2d::PointF::fromAll(-1);
	//}
	updateHoveredNode();
	updateFocusedNode();
}

void DialogAndroid::RequestAnimationFrame(scene2d::Node* node)
{
	auto it = std::find_if(animating_nodes_.begin(), animating_nodes_.end(),
		[&](const auto& link) {
			return link.get() == node;
		});
	if (it == animating_nodes_.end())
		animating_nodes_.push_back(node->weakProxy());
	if (!animating_nodes_.empty() && !animation_timer_id_) {
		animation_timer_id_ = android::start_timer(17, absl::bind_front(&DialogAndroid::handleAnimationFrameEvent, this));
	}
}

scene2d::Scene* DialogAndroid::GetScene() const
{
	return scene_.get();
}

DialogAndroid* DialogAndroid::findDialogById(const std::string& id)
{
    auto it = g_dialog_map.find(id);
	if (it != g_dialog_map.end()) {
		return it->second;
    }
    return nullptr;
}

void DialogAndroid::paint()
{
	scene_->resolveStyle();
	scene_->computeLayout(size_.width, size_.height);

	if (surface_) {
		auto p = surface_->beginPaint();
		const style::Color BACKGROUND_COLOR = style::Color::fromString("#F8F8F8");
		p->clear(BACKGROUND_COLOR);
		scene_->paint(p.get());
		if (surface_->endPaint())
			surface_->swapBuffers();
	}
	scene_->runPostRenderTasks();
}

void DialogAndroid::handleActivityCreated()
{
	auto map = g_dialog_map;
	for (auto& p : map) {
		create_dialog(p.first);
	}
}

void DialogAndroid::handleSurfaceChanged(ANativeWindow* hwnd, float dpi_scale)
{
	dpi_scale_ = dpi_scale;
	pixel_size_.width = ANativeWindow_getWidth(hwnd);
	pixel_size_.height = ANativeWindow_getHeight(hwnd);
	size_ = pixel_size_ / dpi_scale;

	LOG(INFO) << "surface changed: " << pixel_size_;

	surface_ = nullptr;

	xskia::PaintSurfaceX::Configuration cfg;
	cfg.hwnd = hwnd;
	cfg.pixel_size = pixel_size_;
	cfg.dpi_scale = dpi_scale;
	surface_ = xskia::PaintSurfaceX::create(cfg);

	RequestPaint();
}

void DialogAndroid::handleSurfaceDestroyed()
{
	surface_ = nullptr;
}

void DialogAndroid::handleSurfaceRedrawNeeded()
{
	RequestPaint();
}

void DialogAndroid::handleScrollEvent(float x, float y, float dx, float dy)
{
	mouse_position_.x = x / dpi_scale_;
	mouse_position_.y = y / dpi_scale_;
	dx /= dpi_scale_ * 16.0f;
	dy /= -dpi_scale_ * 16.0f;
	updateHoveredNode();

	scene2d::Node* node;
	scene2d::PointF local_pos;
	
	node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_SCROLLABLE, &local_pos);
	if (node) {
		bool hwheel = (abs(dx) > abs(dy));
		float wheel_delta = hwheel ? dx : dy;
		LOG(INFO) << "mouse wheel " << local_pos << ", delta=" << wheel_delta << ", hwheel=" << hwheel;
		const int buttons = 0;
		const int modifiers = 0;
		scene2d::MouseCommand cmd = hwheel ? scene2d::MOUSE_HWHEEL : scene2d::MOUSE_WHEEL;
		scene2d::MouseEvent mouse_wheel(node, cmd, mouse_position_, local_pos, wheel_delta, buttons, modifiers);
		scene_->dispatchEvent(node, mouse_wheel, true);
	}

}
void DialogAndroid::handleShowPressEvent(float x, float y)
{
	mouse_position_.x = x / dpi_scale_;
	mouse_position_.y = y / dpi_scale_;
	updateHoveredNode();
}
void DialogAndroid::handleLongPressEvent(float x, float y)
{
	mouse_position_.x = x / dpi_scale_;
	mouse_position_.y = y / dpi_scale_;
	updateHoveredNode();

	scene2d::Node* node;
	scene2d::PointF local_pos;
	node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_CLICKABLE, &local_pos);
	if (node) {
		//LOG(INFO) << "mouse down " << local_pos;
		//active_node_ = node->weaken();
		//node->state_ |= scene2d::NODE_STATE_ACTIVE;
		int button = scene2d::RIGHT_BUTTON;
		int modifiers = 0;
		scene2d::MouseEvent mouse_down(node, scene2d::MOUSE_DOWN, mouse_position_, local_pos, button, button, modifiers);
		scene_->dispatchEvent(node, mouse_down, true);

		scene2d::MouseEvent mouse_up(node, scene2d::MOUSE_UP, mouse_position_, local_pos, button, 0, modifiers);
		scene_->dispatchEvent(node, mouse_up, true);
	}
}
void DialogAndroid::handleSingleTapConfirmedEvent(float x, float y)
{
	mouse_position_.x = x / dpi_scale_;
	mouse_position_.y = y / dpi_scale_;
	updateHoveredNode();

	scene2d::Node* node;
	scene2d::PointF local_pos;
	node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_CLICKABLE, &local_pos);
	// LOG(INFO) << "handleSingleTapConfirmedEvent pick pos=" << mouse_position_ << ", node=" << node;
	if (node) {
		// LOG(INFO) << "handleSingleTapConfirmedEvent " << local_pos << ", tag=" << node->tag_;
		//active_node_ = node->weaken();
		//node->state_ |= scene2d::NODE_STATE_ACTIVE;
		scene2d::ButtonState button = scene2d::LEFT_BUTTON;
		int modifiers = 0;
		scene2d::MouseEvent mouse_down(node, scene2d::MOUSE_DOWN, mouse_position_, local_pos, button, button, modifiers);
		//mouse_down.button = button;
		//mouse_down.buttons = button;
		LOG(INFO) << "mouse_down button=" << (int)mouse_down.button << ", buttons=" << mouse_down.buttons;
		scene_->dispatchEvent(node, mouse_down, true);

		scene2d::MouseEvent mouse_up(node, scene2d::MOUSE_UP, mouse_position_, local_pos, button, 0, modifiers);
		//mouse_up.button = button;
		//mouse_up.buttons = 0;
		LOG(INFO) << "mouse_up button=" << (int)mouse_up.button << ", buttons=" << mouse_up.buttons;
		scene_->dispatchEvent(node, mouse_up, true);
	}

	node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_FOCUSABLE);
	base::object_refptr<scene2d::Node> old_focused = focused_node_.upgrade();
	if (node) {
		if (node != old_focused.get()) {
			if (old_focused) {
				old_focused->state_ &= ~scene2d::NODE_STATE_FOCUSED;
				scene2d::FocusEvent focus_out(old_focused.get(), scene2d::FOCUS_OUT);
				scene_->dispatchEvent(old_focused.get(), focus_out, true);
			}
		}
		focused_node_ = node->weaken();
		node->state_ |= scene2d::NODE_STATE_FOCUSED;
		scene2d::FocusEvent focus_in(node, scene2d::FOCUS_IN);
		scene_->dispatchEvent(focused_node_.get(), focus_in, true);
	}
}
void DialogAndroid::handleImeComposition(const std::wstring& text, absl::optional<int> caret_pos) {
	base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
	if (node) {
		scene2d::ImeEvent composing(node.get(), scene2d::COMPOSING, text, caret_pos);
		scene_->dispatchEvent(node.get(), composing, true);
	}
}
void DialogAndroid::handleImeCommit(const std::wstring& text) {
	base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
	if (node) {
		scene2d::ImeEvent commit(node.get(), scene2d::COMMIT, text);
		scene_->dispatchEvent(node.get(), commit, true);
	}
}
void DialogAndroid::handleImeStartComposition() {
	// Node2DRef node = _focused_node.lock();
	// if (node) {
	//     node->OnImeStartComposition(*this);
	//     scene2d::PointF origin, size;
	//     if (node->QueryImeCaretRect(origin, size))
	//         UpdateCaretRect(node->MapPointToRoot(origin), size);
	// }
	base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
	if (node) {
		scene2d::ImeEvent start_compose(node.get(), scene2d::START_COMPOSE);
		scene_->dispatchEvent(node.get(), start_compose, true);
		if (start_compose.caret_rect_) {
			scene2d::PointF caret_pos = scene_->mapPointToScene(node.get(), start_compose.caret_rect_->origin());
			scene2d::DimensionF caret_size = start_compose.caret_rect_->size();
			// UpdateCaretRect(scene_->mapPointToScene(node.get(), start_compose.caret_rect_->origin()),
			// 	start_compose.caret_rect_->size());
		}
	}
}
void DialogAndroid::handleImeEndComposition() {
	base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
	if (node) {
		scene2d::ImeEvent end_compose(node.get(), scene2d::END_COMPOSE);
		scene_->dispatchEvent(node.get(), end_compose, true);
	}
}
void DialogAndroid::handleKeyDown(scene2d::VKey key) {
    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::KeyEvent key_down(node.get(), scene2d::KEY_DOWN, key, scene2d::NO_MODIFILER);
        scene_->dispatchEvent(node.get(), key_down, true);
    }
}
void DialogAndroid::handleKeyUp(scene2d::VKey key) {
    base::object_refptr<scene2d::Node> node = focused_node_.upgrade();
    if (node) {
        scene2d::KeyEvent key_up(node.get(), scene2d::KEY_UP, key, scene2d::NO_MODIFILER);
        scene_->dispatchEvent(node.get(), key_up, true);
    }
}
void DialogAndroid::updateHoveredNode() {
	scene2d::PointF local_pos;
	scene2d::Node* node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_HOVERABLE, &local_pos);
	base::object_refptr<scene2d::Node> old_hovered = hovered_node_.upgrade();
	if (node != old_hovered.get()) {
		if (old_hovered) {
			old_hovered->state_ &= ~scene2d::NODE_STATE_HOVER;
			scene2d::MouseEvent hover_leave(old_hovered.get(), scene2d::MOUSE_OUT, mouse_position_, scene2d::PointF());
			scene_->dispatchEvent(old_hovered.get(), hover_leave, true);
		}
		hovered_node_ = node ? node->weaken() : base::object_weakptr<scene2d::Node>();
		if (node) {
			node->state_ |= scene2d::NODE_STATE_HOVER;
			scene2d::MouseEvent hover_enter(node, scene2d::MOUSE_OVER, mouse_position_, local_pos);
			scene_->dispatchEvent(node, hover_enter, true);
		}
		RequestPaint();
	}
}
void DialogAndroid::updateFocusedNode() {
	// Focused node become invisible
	if (auto node = focused_node_.upgrade()) {
		if (!node->visibleInHierarchy()) {
			node->state_ &= ~scene2d::NODE_STATE_FOCUSED;
			scene2d::FocusEvent focus_out(node.get(), scene2d::FOCUS_OUT);
			scene_->dispatchEvent(node.get(), focus_out, true);
			focused_node_ = nullptr;
		}
	}
}
void DialogAndroid::handleAnimationFrameEvent()
{
    absl::Time timestamp = absl::Now();
    auto nodes = std::move(animating_nodes_);
    animating_nodes_.clear();
    for (auto& link : nodes) {
        auto node = link.upgrade();
        if (node)
            node->onAnimationFrame(timestamp);
    }
    if (animating_nodes_.empty() && animation_timer_id_) {
		android::stop_timer(animation_timer_id_);
        animation_timer_id_ = 0;
    }
}

}
