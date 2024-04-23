#pragma once
#include "script/Dialog.h"
#include "xskia/PainterX.h"

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

private:
	std::unique_ptr<scene2d::Scene> scene_;
};

}
