#pragma once
#include "scene2d/geom_types.h"
#include "scene2d/Event.h"
#include "graph2d/Painter.h"

namespace style {

struct ScrollObject {
	scene2d::DimensionF content_size;
	scene2d::RectF viewport_rect;

	static bool hitTest(const ScrollObject* sd, const scene2d::PointF& pos, int flags);
	static void onEvent(ScrollObject* sd, scene2d::MouseEvent& event, scene2d::Node* node);
	static void paintVScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
};

}
