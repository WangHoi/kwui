#pragma once
#include "scene2d/geom_types.h"
#include "scene2d/Event.h"
#include "graph2d/Painter.h"

namespace style {

struct ScrollObject {
	enum StateFlag {
		State_None = 0,
		State_Active = 1,
		State_MouseOver = 2,
		
		SubControl_StartButton = (1 << 8),
		SubControl_TrackStartPiece = (1 << 9),
		SubControl_Thumb = (1 << 10),
		SubControl_TrackEndPiece = (1 << 11),
		SubControl_EndButton = (1 << 12),
	};
	scene2d::DimensionF content_size;
	scene2d::DimensionF viewport_size;
	scene2d::PointF scroll_offset;
	int v_scrollbar_flags = StateFlag::State_None;
	int h_scrollbar_flags = StateFlag::State_None;

	enum class HitTestResult {
		None,
		Client,
		HScrollbarLeftButton,
		HScrollbarTrackStartPiece,
		HScrollbarThumb,
		HScrollbarTrackEndPiece,
		HScrollbarRightButton,
		VScrollbarTopButton,
		VScrollbarTrackStartPiece,
		VScrollbarThumb,
		VScrollbarTrackEndPiece,
		VScrollbarBottomButton,
		ResizeCorner,
	};

	static bool hitTest(const ScrollObject* sd, const scene2d::PointF& pos, int flags);
	static HitTestResult hitTestPart(const ScrollObject* sd, const scene2d::PointF& pos);
	static void paintVScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintHScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);

private:
	static void paintScrollbarBackground(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintVScrollbarTopButton(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintVScrollbarBottomButton(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintVScrollbarThumb(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);

	std::shared_ptr<graph2d::BitmapInterface> vtop_button_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vtop_button_hover_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vtop_button_active_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vbottom_button_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vbottom_button_hover_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vbottom_button_active_bitmap_;
};

}
