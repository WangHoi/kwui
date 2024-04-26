#include "DialogAndroid.h"
#include "scene2d/Scene.h"
#include "absl/strings/str_format.h"
#include "include/core/SkSurface.h"
#include "Application_jni.h"

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
}
DialogAndroid::~DialogAndroid()
{
	g_dialog_map.erase(id_);
	if (g_first_dialog == this) {
		g_first_dialog = nullptr;
	}
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
	kwui_request_paint();
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
	//UpdateFocusedNode();
}

void DialogAndroid::RequestAnimationFrame(scene2d::Node* node)
{
}

scene2d::Scene* DialogAndroid::GetScene() const
{
	return scene_.get();
}

DialogAndroid* DialogAndroid::findDialogById(const std::string& id)
{
    return nullptr;
}

void DialogAndroid::paint(SkCanvas* canvas, float dpi_scale)
{
	dpi_scale_ = dpi_scale;
	pixel_size_.width = canvas->getSurface()->width();
	pixel_size_.height = canvas->getSurface()->height();
	size_ = pixel_size_ / dpi_scale;

	scene_->resolveStyle();
	float width = canvas->getSurface()->width() / dpi_scale;
	float height = canvas->getSurface()->height() / dpi_scale;
	scene_->computeLayout(width, height);

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

void DialogAndroid::handleSurfaceChanged(ANativeWindow* hwnd, float dpi_scale)
{
}

void DialogAndroid::handleSurfaceDestroyed()
{
}

void DialogAndroid::handleSurfaceRedrawNeeded()
{
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
	scene2d::Node* node;
	scene2d::PointF local_pos;
	node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_CLICKABLE, &local_pos);
	if (node) {
		//LOG(INFO) << "mouse down " << local_pos;
		//active_node_ = node->weaken();
		//node->state_ |= scene2d::NODE_STATE_ACTIVE;
		int button = scene2d::LEFT_BUTTON;
		int modifiers = 0;
		scene2d::MouseEvent mouse_down(node, scene2d::MOUSE_DOWN, mouse_position_, local_pos, button, button, modifiers);
		scene_->dispatchEvent(node, mouse_down, true);

		scene2d::MouseEvent mouse_up(node, scene2d::MOUSE_UP, mouse_position_, local_pos, button, 0, modifiers);
		scene_->dispatchEvent(node, mouse_up, true);
	}
}
void DialogAndroid::handleSingleTapConfirmedEvent(float x, float y)
{
	scene2d::Node* node;
	scene2d::PointF local_pos;
	node = scene_->pickNode(mouse_position_, scene2d::NODE_FLAG_CLICKABLE, &local_pos);
	if (node) {
		//LOG(INFO) << "mouse down " << local_pos;
		//active_node_ = node->weaken();
		//node->state_ |= scene2d::NODE_STATE_ACTIVE;
		int button = scene2d::LEFT_BUTTON;
		int modifiers = 0;
		scene2d::MouseEvent mouse_down(node, scene2d::MOUSE_DOWN, mouse_position_, local_pos, button, button, modifiers);
		scene_->dispatchEvent(node, mouse_down, true);

		scene2d::MouseEvent mouse_up(node, scene2d::MOUSE_UP, mouse_position_, local_pos, button, 0, modifiers);
		scene_->dispatchEvent(node, mouse_up, true);
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

}
