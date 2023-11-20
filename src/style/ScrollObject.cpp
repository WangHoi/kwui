#include "ScrollObject.h"
#include "scene2d/Node.h"

namespace style {

static const float SCROLLBAR_WHEEL_FACTOR = 16.0f;

bool ScrollObject::hitTest(const ScrollObject* sd, const scene2d::PointF& pos, int flags)
{
	if (flags & scene2d::NODE_FLAG_SCROLLABLE)
		return true;
	return (pos.x < 0 || pos.x >= sd->viewport_rect.width()
		|| pos.y < 0 || pos.y >= sd->viewport_rect.height());
}

void ScrollObject::onEvent(ScrollObject* sd, scene2d::MouseEvent& event, scene2d::Node* node)
{
	if (event.cmd == scene2d::MOUSE_WHEEL) {
		float d = -event.wheel_delta * SCROLLBAR_WHEEL_FACTOR;
		if (event.modifiers == scene2d::LSHIFT_MODIFIER || event.modifiers == scene2d::RSHIFT_MODIFIER) {
			float w = sd->viewport_rect.width();
			float x = std::clamp(sd->viewport_rect.left + d, 0.0f, sd->content_size.width - w);
			if (sd->viewport_rect.left != x) {
				sd->viewport_rect.left = x;
				sd->viewport_rect.right = x + w;
				node->requestPaint();
			}
		} else {
			float h = sd->viewport_rect.height();
			float y = std::clamp(sd->viewport_rect.top + d, 0.0f, sd->content_size.height - h);
			if (sd->viewport_rect.top != y) {
				sd->viewport_rect.top = y;
				sd->viewport_rect.bottom = y + h;
				node->requestPaint();
			}
		}
	}
}

void ScrollObject::paintVScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	const auto color = Value::fromHexColor("#ccc");
	if (sd->viewport_rect.height() >= sd->content_size.height) {
		painter->drawBox(rect, 0.0f, color, color);
		return;
	}
	float factor = rect.height() / sd->content_size.height;
	float y1 = sd->viewport_rect.top * factor;
	float y2 = sd->viewport_rect.bottom * factor;
	painter->drawBox(scene2d::RectF::fromLTRB(rect.left, rect.top + y1, rect.right, rect.top + y2), 0.0f, color, color);
}

void ScrollObject::paintHScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	const auto color = Value::fromHexColor("#ccc");
	if (sd->viewport_rect.width() >= sd->content_size.width) {
		painter->drawBox(rect, 0.0f, color, color);
		return;
	}
	float factor = rect.width() / sd->content_size.width;
	float x1 = sd->viewport_rect.left * factor;
	float x2 = sd->viewport_rect.right * factor;
	painter->drawBox(scene2d::RectF::fromLTRB(rect.left + x1, rect.top, rect.left + x2, rect.bottom), 0.0f, color, color);
}

}
