#pragma once
#include "script/Dialog.h"
#include "xskia/PainterX.h"
#include "scene2d//geom_types.h"

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

	void paint(SkCanvas* canvas, float dpi_scale);
	void handleScrollEvent(float x, float y, float dx, float dy);
	void handleShowPressEvent(float x, float y);
	void handleLongPressEvent(float x, float y);
	void handleSingleTapConfirmedEvent(float x, float y);

private:
	void updateHoveredNode();

	std::unique_ptr<scene2d::Scene> scene_;
	scene2d::DimensionF pixel_size_ = { 1.0f, 1.0f };
	scene2d::DimensionF size_ = { 1.0f, 1.0f };
	float dpi_scale_ = 1.0f;
	scene2d::PointF mouse_position_;
	base::object_weakptr<scene2d::Node> hovered_node_;
	base::object_weakptr<scene2d::Node> active_node_;
	base::object_weakptr<scene2d::Node> focused_node_;
};

}
