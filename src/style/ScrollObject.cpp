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
		float h = sd->viewport_rect.height();
		float y = std::clamp(sd->viewport_rect.top + d, 0.0f, sd->content_size.height - h);
		if (sd->viewport_rect.top != y) {
			sd->viewport_rect.top = y;
			sd->viewport_rect.bottom = y + h;
			node->requestPaint();
		}
	}
}

void ScrollObject::paintVScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	if (sd->viewport_rect.height() >= sd->content_size.height)
		return;
	float factor = rect.height() / sd->content_size.height;
	float y1 = sd->viewport_rect.top * factor;
	float y2 = sd->viewport_rect.bottom * factor;
	const auto color = Value::fromHexColor("#ccc");
	painter->drawBox(scene2d::RectF::fromLTRB(rect.left, rect.top + y1, rect.right, rect.top + y2), 0.0f, color, color);
}

}
