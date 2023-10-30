#include "ScrollBarControl.h"
#include "windows/graphics/Painter.h"

namespace windows {
namespace control {

const char* ScrollBarControl::CONTROL_NAME = "scroll_bar";

ScrollBarControl::ScrollBarControl()
	: bg_color_(WHITE)
	, color_(BLUE)
	, border_radius_(0.0f) {}
base::string_atom ScrollBarControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
void ScrollBarControl::onPaint(graph2d::PainterInterface& pi, const scene2d::RectF& rect) {
	if (rect.width() <= 0.0f && rect.height() <= 0.0f)
		return;

	graphics::Painter& p = graphics::PainterImpl::unwrap(pi);
	p.SetColor(bg_color_);
	p.DrawRect(rect.origin(), rect.size());

	if (viewport_len_ > 0.0f && inner_len_ > viewport_len_) {
		if (orient_ == VERTICAL) {
			float y1 = scroll_offset_;
			float h = scroll_offset_ + (viewport_len_ / inner_len_) * rect.height();
			p.DrawRoundedRect(rect.origin() + scene2d::PointF(0.0f, y1), scene2d::DimensionF(rect.width(), h), border_radius_);
		} else {
			float x1 = scroll_offset_;
			float w = scroll_offset_ + (viewport_len_ / inner_len_) * rect.width();
			p.DrawRoundedRect(rect.origin() + scene2d::PointF(x1, 0.0f), scene2d::DimensionF(w, rect.height()), border_radius_);
		}
	}
}
void ScrollBarControl::onSetAttribute(base::string_atom name, const scene2d::NodeAttributeValue& value)
{
	LOG(WARNING) << "TODO: ScrollBarControl::onSetAttribute";
	/*
	if (name == base::string_intern("value")) {
		SetProgress(absl::get<float>(value));
	}
	*/
}

}
}
