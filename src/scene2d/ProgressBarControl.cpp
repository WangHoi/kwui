#include "ProgressBarControl.h"

namespace scene2d {

const char* ProgressBarControl::CONTROL_NAME = "progress_bar";

ProgressBarControl::ProgressBarControl()
	: progress_(0.0f)
	, bg_color_(style::named_color::white)
	, fg_color_(style::named_color::lightblue)
	, border_radius_(0.0f) {}
base::string_atom ProgressBarControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
void ProgressBarControl::onPaint(graph2d::PainterInterface& pi, const scene2d::RectF& rect) {
	style::CornerRadiusF radius({
		border_radius_, border_radius_, border_radius_, border_radius_,
	});
	pi.drawRoundedRect(rect, radius, bg_color_);

	scene2d::DimensionF bar_size = { rect.width() * progress_, rect.height() };
	pi.drawRoundedRect(scene2d::RectF::fromOriginSize(rect.origin(), bar_size) , radius, fg_color_);
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
	progress_ = std::clamp(value, 0.0f, 1.0f);
}

}
