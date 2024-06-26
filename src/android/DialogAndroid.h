#pragma once
#include "script/Dialog.h"
#include "xskia/PainterX.h"
#include "xskia/PaintSurfaceX.h"
#include "scene2d/geom_types.h"
#include "absl/types/optional.h"
#include <string>
#include <android/native_window.h>
#include <vector>

namespace android {

class DialogAndroid : public script::DialogInterface {
public:
	DialogAndroid();
	~DialogAndroid();

	static DialogAndroid* firstDialog();

	std::string eventContextId() const override;
	scene2d::PointF GetMousePosition() const override;
	void RequestPaint() override;
	void RequestUpdate() override;
	void RequestAnimationFrame(scene2d::Node* node) override;
	scene2d::Scene* GetScene() const override;

	static DialogAndroid* findDialogById(const std::string& id);

	static void handleActivityCreated();
	void handleSurfaceChanged(ANativeWindow* hwnd, float dpi_scale);
	void handleSurfaceDestroyed();
	void handleSurfaceRedrawNeeded();
	void handleScrollEvent(float x, float y, float dx, float dy);
	void handleShowPressEvent(float x, float y);
	void handleLongPressEvent(float x, float y);
	void handleSingleTapConfirmedEvent(float x, float y);
	void handleImeComposition(const std::wstring& text, absl::optional<int> caret_pos);
	void handleImeCommit(const std::wstring& text);
	void handleImeStartComposition();
	void handleImeEndComposition();
    void handleKeyDown(scene2d::VKey key);
    void handleKeyUp(scene2d::VKey key);

private:
	void paint();
	void updateHoveredNode();
	void updateFocusedNode();
	void handleAnimationFrameEvent();

	std::string id_;
	std::unique_ptr<xskia::PaintSurfaceX> surface_;
	std::unique_ptr<scene2d::Scene> scene_;
	scene2d::DimensionF pixel_size_ = { 1.0f, 1.0f };
	scene2d::DimensionF size_ = { 1.0f, 1.0f };
	float dpi_scale_ = 1.0f;
	scene2d::PointF mouse_position_;
	base::object_weakptr<scene2d::Node> hovered_node_;
	base::object_weakptr<scene2d::Node> active_node_;
	base::object_weakptr<scene2d::Node> focused_node_;
	std::vector<base::object_weakptr<scene2d::Node>> animating_nodes_;
	int animation_timer_id_ = 0;
};

}
