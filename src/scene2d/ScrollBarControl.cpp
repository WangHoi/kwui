#include "ScrollbarControl.h"
#include "windows/graphics/Painter.h"

namespace windows {
namespace control {

const char* ScrollbarControl::CONTROL_NAME = "scroll_bar";

ScrollbarControl::ScrollbarControl()
	: bg_color_(WHITE)
	, color_(BLUE)
	, border_radius_(0.0f) {}
base::string_atom ScrollbarControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
void ScrollbarControl::onPaint(graph2d::PainterInterface& pi, const scene2d::RectF& rect) {
	if (rect.width() <= 0.0f && rect.height() <= 0.0f)
		return;

	graphics::Painter& p = graphics::PainterImpl::unwrap(pi);
	p.SetColor(bg_color_);
	p.DrawRect(rect.origin(), rect.size());

	if (client_len_ > 0.0f && scroll_len_ > client_len_) {
		if (orient_ == VERTICAL) {
			float y1 = scroll_offset_;
			float h = scroll_offset_ + (client_len_ / scroll_len_) * rect.height();
			p.DrawRoundedRect(rect.origin() + scene2d::PointF(0.0f, y1), scene2d::DimensionF(rect.width(), h), border_radius_);
		} else {
			float x1 = scroll_offset_;
			float w = scroll_offset_ + (client_len_ / scroll_len_) * rect.width();
			p.DrawRoundedRect(rect.origin() + scene2d::PointF(x1, 0.0f), scene2d::DimensionF(w, rect.height()), border_radius_);
		}
	}
}
void ScrollbarControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{
	LOG(WARNING) << "TODO: ScrollbarControl::onSetAttribute";
	/*
	if (name == base::string_intern("value")) {
		SetProgress(absl::get<float>(value));
	}
	*/
}

}
}
