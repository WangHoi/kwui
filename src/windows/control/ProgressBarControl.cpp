#include "ProgressBarControl.h"
#include "windows/graphics/PainterD2D.h"

namespace windows {
namespace control {

const char* ProgressBarControl::CONTROL_NAME = "progress_bar";

ProgressBarControl::ProgressBarControl()
	: _progress(0.0f)
	, _bg_color(style::named_color::white)
	, _color(style::named_color::lightblue)
	, _border_radius(0.0f) {}
base::string_atom ProgressBarControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
void ProgressBarControl::onPaint(graph2d::PainterInterface& pi, const scene2d::RectF& rect) {
	graphics::Painter& p = graphics::PainterImpl::unwrap(pi);
	p.SetColor(_bg_color);
	p.DrawRoundedRect(rect.origin(), rect.size(), _border_radius);

	scene2d::DimensionF bar_size = { rect.width() * _progress, rect.height() };
	p.SetColor(_color);
	p.DrawRoundedRect(rect.origin(), bar_size, _border_radius);
}
void ProgressBarControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{
	if (name == base::string_intern("value")) {
		SetProgress((float)value.toDouble());
	} else if (name == base::string_intern("color")) {
		SetColor(style::Color::fromString(value.toString()));
	} else if (name == base::string_intern("backgroundColor")) {
		SetBackgroundColor(style::Color::fromString(value.toString()));
	} else if (name == base::string_intern("borderRadius")) {
		if (value.isNumber()) {
			SetBorderRadius((float)value.toDouble());
		} else {
			LOG(INFO) << "progress_bar: borderRadius must be number.";
		}
	}
}
void ProgressBarControl::SetProgress(float value) {
	_progress = std::clamp(value, 0.0f, 1.0f);
}

}
}
