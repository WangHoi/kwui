#include "DialogAndroid.h"
#include "scene2d/Scene.h"
#include "include/core/SkSurface.h"

namespace android {

static DialogAndroid* g_first_dialog = nullptr;

DialogAndroid::DialogAndroid()
{
	if (!g_first_dialog) {
		g_first_dialog = this;
	}
	scene_ = std::make_unique<scene2d::Scene>(*this);
}
DialogAndroid::~DialogAndroid()
{
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
	return scene_.get();
}

void DialogAndroid::paint(SkCanvas* canvas, float dpi_scale)
{
	scene_->resolveStyle();
	float width = canvas->getSurface()->width() / dpi_scale;
	float height = canvas->getSurface()->width() / dpi_scale;
	scene_->computeLayout(width, height);

	xskia::PainterX p(canvas, dpi_scale);
	const style::Color BACKGROUND_COLOR = style::Color::fromString("#F8F8F8");
	p.clear(BACKGROUND_COLOR);
	scene_->paint(&p);
}

}
