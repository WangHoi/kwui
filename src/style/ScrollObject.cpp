#include "ScrollObject.h"
#include "scene2d/Node.h"

namespace style {

bool ScrollObject::hitTest(const ScrollObject* sd, const scene2d::PointF& pos, int flags)
{
	if (flags & scene2d::NODE_FLAG_SCROLLABLE)
		return true;
	return (pos.x < 0 || pos.x >= sd->viewport_size.width
		|| pos.y < 0 || pos.y >= sd->viewport_size.height);
}

ScrollObject::HitTestResult ScrollObject::hitTestPart(const ScrollObject* sd, const scene2d::PointF& pos)
{
	if (pos.x < 0 || pos.y < 0) {
		return HitTestResult::None;
	} else if (sd->viewport_size.width >= sd->content_size.width
		&& sd->viewport_size.height >= sd->content_size.height) {
		return HitTestResult::Client;
	} else if (pos.x >= sd->viewport_size.width && pos.y >= sd->viewport_size.height) {
		return HitTestResult::ResizeCorner;
	} else if (pos.x >= sd->viewport_size.width) {
		float factor = sd->viewport_size.height / sd->content_size.height;
		float y1 = sd->scroll_offset.y * factor;
		float y2 = (sd->scroll_offset.y + sd->viewport_size.height) * factor;
		if (pos.y < y1) {
			return HitTestResult::VScrollbarTrackStartPiece;
		} else if (y1 <= pos.y && pos.y < y2) {
			return HitTestResult::VScrollbarThumb;
		} else {
			return HitTestResult::VScrollbarTrackEndPiece;
		}
	} else if (pos.y >= sd->viewport_size.height) {
		float factor = sd->viewport_size.width / sd->content_size.width;
		float x1 = sd->scroll_offset.x * factor;
		float x2 = (sd->scroll_offset.x + sd->viewport_size.width) * factor;
		if (pos.x < x1) {
			return HitTestResult::HScrollbarTrackStartPiece;
		} else if (x1 <= pos.x && pos.x < x2) {
			return HitTestResult::HScrollbarThumb;
		} else {
			return HitTestResult::HScrollbarTrackEndPiece;
		}
	}
	return HitTestResult::None;
}

void ScrollObject::paintVScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	const auto color = sd->v_scrollbar_active
		? Color::fromString("#aaa")
		: (sd->v_scrollbar_hover ? Color::fromString("#bbb") : Color::fromString("#ccc"));
	if (sd->viewport_size.height >= sd->content_size.height) {
		painter->drawBox(rect, EdgeOffsetF(), CornerRadiusF(), color, color);
		return;
	}
	float factor = rect.height() / sd->content_size.height;
	float y1 = sd->scroll_offset.y * factor;
	float y2 = (sd->scroll_offset.y + sd->viewport_size.height) * factor;
	painter->drawBox(scene2d::RectF::fromLTRB(rect.left, rect.top + y1, rect.right, rect.top + y2),
		EdgeOffsetF(), CornerRadiusF(), color, color);
}

void ScrollObject::paintHScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	const auto color = sd->h_scrollbar_active
		? Color::fromString("#aaa")
		: (sd->h_scrollbar_hover ? Color::fromString("#bbb") : Color::fromString("#ccc"));
	if (sd->viewport_size.width >= sd->content_size.width) {
		painter->drawBox(rect, EdgeOffsetF(), CornerRadiusF(), color, color);
		return;
	}
	float factor = rect.width() / sd->content_size.width;
	float x1 = sd->scroll_offset.x * factor;
	float x2 = (sd->scroll_offset.x + sd->viewport_size.width) * factor;
	painter->drawBox(scene2d::RectF::fromLTRB(rect.left + x1, rect.top, rect.left + x2, rect.bottom),
		EdgeOffsetF(), CornerRadiusF(), color, color);
}

}
