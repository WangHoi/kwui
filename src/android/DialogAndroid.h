#pragma once
#include "script/Dialog.h"

namespace android {

class DialogAndroid : public script::DialogInterface {
public:
	// Í¨¹ý DialogInterface ¼Ì³Ð
	std::string eventContextId() const override;
	scene2d::PointF GetMousePosition() const override;
	void RequestPaint() override;
	void RequestUpdate() override;
	void RequestAnimationFrame(scene2d::Node* node) override;
	scene2d::Scene* GetScene() const override;
};

}
