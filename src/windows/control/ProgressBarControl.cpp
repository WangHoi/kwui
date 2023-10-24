#include "ProgressBarControl.h"
#include "windows/graphics/Painter.h"

namespace windows {
namespace control {

const char* ProgressBarControl::CONTROL_NAME = "progress_bar";

ProgressBarControl::ProgressBarControl()
	: _progress(0.0f)
	, _bg_color(WHITE)
	, _color(BLUE)
	, _border_radius(0.0f) {}
base::string_atom ProgressBarControl::name()
{
	return base::string_intern(CONTROL_NAME);
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

}
}
