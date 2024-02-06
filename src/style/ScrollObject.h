#pragma once
#include "scene2d/geom_types.h"
#include "scene2d/Event.h"
#include "graph2d/Painter.h"

namespace style {

struct ScrollObject {
	enum class SubControl {
		VStartButton,
		VTrackStartPiece,
		VThumb,
		VTrackEndPiece,
		VEndButton,

		HStartButton,
		HTrackStartPiece,
		HThumb,
		HTrackEndPiece,
		HEndButton,
	};
	enum StateFlag: uint32_t {
		State_None = 0,
		State_Active = 1,
		State_MouseOver = 2,
		State_Mask = 0x0000ffff,

		State_SubControl_ShiftBits = 16,
		State_SubControl_VStartButton = (1 << 16),
		State_SubControl_VTrackStartPiece = (1 << 17),
		State_SubControl_VThumb = (1 << 18),
		State_SubControl_VTrackEndPiece = (1 << 19),
		State_SubControl_VEndButton = (1 << 20),
		State_SubControl_HStartButton = (1 << 21),
		State_SubControl_HTrackStartPiece = (1 << 22),
		State_SubControl_HThumb = (1 << 23),
		State_SubControl_HTrackEndPiece = (1 << 24),
		State_SubControl_HEndButton = (1 << 25),

		State_SubControl_Mask = 0xffff0000,
	};
	scene2d::DimensionF content_size;
	scene2d::DimensionF viewport_size;
	scene2d::PointF scroll_offset;
	uint32_t scrollbar_flags = StateFlag::State_None;

	static const float SCROLLBAR_GUTTER_WIDTH;

	static bool hitTest(const ScrollObject* sd, const scene2d::PointF& pos, int flags);
	static absl::optional<SubControl> subControlHitTest(const ScrollObject* sd, const scene2d::PointF& pos);
	static void paintVScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintHScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);

private:
	static void paintScrollbarBackground(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintVScrollbarTopButton(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintVScrollbarBottomButton(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static void paintVScrollbarThumb(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect);
	static uint32_t subControlFlags(SubControl sc, uint32_t flags);
	static const Color& subControlColorForFlags(SubControl sc, uint32_t flags);

	std::shared_ptr<graph2d::BitmapInterface> vtop_button_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vtop_button_hover_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vtop_button_active_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vbottom_button_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vbottom_button_hover_bitmap_;
	std::shared_ptr<graph2d::BitmapInterface> vbottom_button_active_bitmap_;
};

}
