#pragma once
#include "scene2d/geom_types.h"
#include "scene2d/Event.h"
#include "graph2d/Painter.h"

namespace style {

struct ScrollObject {
	scene2d::DimensionF content_size;
	scene2d::DimensionF viewport_size;
	scene2d::PointF scroll_offset;
	bool mouse_down_v_scrollbar = false;
	bool mouse_down_h_scrollbar = false;

	enum HitTestResult {
		None,
		Client,
		HScrollBar,
		HScrollTrack,
		VScrollBar,
		VScrollTrack,
		ResizeCorner,
	};

	static bool hitTest(const ScrollObject* sd, const scene2d::PointF& pos, int flags);
	static HitTestResult hitTestPart(const ScrollObject* sd, const scene2d::PointF& pos);
	static void paintVScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintHScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
};

}
