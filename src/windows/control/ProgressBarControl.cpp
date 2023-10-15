#include "ProgressBarControl.h"
#include "windows/graphics/Painter.h"

namespace windows {
namespace control {

ProgressBarControl::ProgressBarControl()
	: _progress(0.0f)
	, _bg_color(WHITE)
	, _color(BLUE)
	, _border_radius(0.0f) {}
base::string_atom ProgressBarControl::name()
{
	return base::string_intern("progress-bar");
}
void ProgressBarControl::onPaint(graphics::Painter& p, const scene2d::RectF& rect) {
	p.SetColor(_bg_color);
	p.DrawRoundedRect(rect.origin(), rect.size(), _border_radius);

	scene2d::DimensionF bar_size = { rect.width() * _progress, rect.height() };
	p.SetColor(_color);
	p.DrawRoundedRect(rect.origin(), bar_size, _border_radius);
}
void ProgressBarControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{
	if (name == base::string_intern("value")) {
		SetProgress(absl::get<float>(value));
	}
}
void ProgressBarControl::SetProgress(float value) {
	_progress = std::clamp(value, 0.0f, 1.0f);
}

void register_progress_bar_control()
{
	scene2d::ControlRegistry::get()
		->registerControl(base::string_intern("progress-bar"),
			[]() -> scene2d::Control* {
				return new ProgressBarControl();
			});
}

}
}
