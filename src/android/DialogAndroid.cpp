#include "DialogAndroid.h"

namespace android {

std::string DialogAndroid::eventContextId() const
{
	return std::string();
}

scene2d::PointF DialogAndroid::GetMousePosition() const
{
	return scene2d::PointF();
}

void DialogAndroid::RequestPaint()
{
}

void DialogAndroid::RequestUpdate()
{
}

void DialogAndroid::RequestAnimationFrame(scene2d::Node* node)
{
}

scene2d::Scene* DialogAndroid::GetScene() const
{
	return nullptr;
}

}
