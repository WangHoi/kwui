#include "SpinnerControl.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/graphics/PainterD2D.h"

namespace windows {
namespace control {

const char* SpinnerControl::CONTROL_NAME = "spinner";

SpinnerControl::SpinnerControl()
	: bg_color_(style::named_color::gray)
	, fg_color_(style::named_color::orangered)
{
}

base::string_atom SpinnerControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
void SpinnerControl::onAnimationFrame(scene2d::Node* node, absl::Time timestamp)
{
	timestamp_.emplace(timestamp);
	node->requestPaint();
	node->requestAnimationFrame(node);
}
void SpinnerControl::onAttach(scene2d::Node* node)
{
	node->requestAnimationFrame(node);
}
void SpinnerControl::onPaint(graph2d::PainterInterface& pi, const scene2d::RectF& rect)
{
	graphics::Painter& p = graphics::PainterImpl::unwrap(pi);
	double sec = fmod((double)absl::ToUnixMillis(timestamp_.value_or(absl::Time())) / 1000.0, 1.0);
	p.SetStrokeColor(bg_color_);
	float radius = std::min(rect.width(), rect.height()) * 0.5f * 0.8f;
	p.SetStrokeWidth(std::max(1.0f, radius * 0.3f));
	p.DrawCircle(rect.center(), radius);

	p.SetStrokeColor(fg_color_);
	p.DrawArc(rect.center(), radius, 360 * sec, 120);
}
void SpinnerControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{
	if (name == base::string_intern("backgroundColor")) {
		setBackgroundColor(style::Color::fromString(value.toString()));
	} else if (name == base::string_intern("foregroundColor")) {
		setForegroundColor(style::Color::fromString(value.toString()));
	}
}
void SpinnerControl::setBackgroundColor(const style::Color& color)
{
	bg_color_ = color;
}
void SpinnerControl::setForegroundColor(const style::Color& color)
{
	fg_color_ = color;
}

}
}
