#include "ScrollObject.h"
#include "scene2d/Node.h"
#include "graph2d/graph2d.h"

namespace style {

const float ScrollObject::SCROLLBAR_GUTTER_WIDTH = 16.0f;
static const char* VSCROLL_TOP_BUTTON_PNG = "D:/projects/kwui/vscroll_top_button.png";
static const char* VSCROLL_BOTTOM_BUTTON_PNG = "D:/projects/kwui/vscroll_bottom_button.png";
static const Color SCROLLBAR_THUMB_NORMAL_COLOR(0xcc, 0xcc, 0xcc); // "#ccc"
static const Color SCROLLBAR_THUMB_HOVER_COLOR(0xbb, 0xbb, 0xbb); // "#bbb"
static const Color SCROLLBAR_THUMB_ACTIVE_COLOR(0xaa, 0xaa, 0xaa); // "#aaa"
static const Color SCROLLBAR_BUTTON_NORMAL_COLOR;
static const Color SCROLLBAR_BUTTON_HOVER_COLOR(0xbb, 0xbb, 0xbb, 0x60);
static const Color SCROLLBAR_BUTTON_ACTIVE_COLOR(0xaa, 0xaa, 0xaa, 0x80);

bool ScrollObject::hitTest(const ScrollObject* sd, const scene2d::PointF& pos, int flags)
{
	if (flags & scene2d::NODE_FLAG_SCROLLABLE)
		return true;
	return (pos.x >= sd->viewport_size.width || pos.y >= sd->viewport_size.height);
}

absl::optional<ScrollObject::SubControl> ScrollObject::subControlHitTest(const ScrollObject* sd, const scene2d::PointF& pos)
{
	if (pos.x < 0 || pos.y < 0) {
		return absl::nullopt;
	} else if (sd->viewport_size.width >= sd->content_size.width
		&& sd->viewport_size.height >= sd->content_size.height) {
		// Client area
		return absl::nullopt; 
	} else if (pos.x >= sd->viewport_size.width && pos.y >= sd->viewport_size.height) {
		// Bottom right resize corner
		return absl::nullopt;
	} else if (pos.x >= sd->viewport_size.width) {
		if (pos.y < SCROLLBAR_GUTTER_WIDTH) {
			return SubControl::VStartButton;
		} else {
			float bottom_btn_top = (sd->viewport_size.width < sd->content_size.width) // has h-scrollbar
				? sd->viewport_size.height
				: (sd->viewport_size.height - SCROLLBAR_GUTTER_WIDTH);
			if (pos.y >= bottom_btn_top) {
				return SubControl::VEndButton;
			} else {
				float factor = (sd->viewport_size.height - 2.0f * SCROLLBAR_GUTTER_WIDTH) / sd->content_size.height;
				float y1 = sd->scroll_offset.y * factor;
				float y2 = (sd->scroll_offset.y + sd->viewport_size.height) * factor;
				if ((pos.y - SCROLLBAR_GUTTER_WIDTH) < y1) {
					return SubControl::VTrackStartPiece;
				} else if (y1 <= (pos.y - SCROLLBAR_GUTTER_WIDTH) && (pos.y - SCROLLBAR_GUTTER_WIDTH) < y2) {
					//LOG(INFO) << "viewport_size=" << sd->viewport_size << ", content_size=" << sd->content_size;
					//LOG(INFO) << "VScrollbarThumb y1=" << y1 << ", pos.y=" << pos.y << ", y2=" << y2;
					return SubControl::VThumb;
				} else {
					return SubControl::VTrackEndPiece;
				}
			}
		}
	} else if (pos.y >= sd->viewport_size.height) {
		float factor = sd->viewport_size.width / sd->content_size.width;
		float x1 = sd->scroll_offset.x * factor;
		float x2 = (sd->scroll_offset.x + sd->viewport_size.width) * factor;
		if (pos.x < x1) {
			return SubControl::HTrackStartPiece;
		} else if (x1 <= pos.x && pos.x < x2) {
			return SubControl::HThumb;
		} else {
			return SubControl::HTrackEndPiece;
		}
	}
	return absl::nullopt;
}

void ScrollObject::paintVScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	if (sd->viewport_size.height >= sd->content_size.height) {
		paintScrollbarBackground(sd, painter, rect);
		return;
	}

	paintScrollbarBackground(sd, painter, rect);
	paintVScrollbarThumb(sd, painter, rect);
	paintVScrollbarTopButton(sd, painter, rect);
	paintVScrollbarBottomButton(sd, painter, rect);
}

void ScrollObject::paintHScrollbar(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	const auto color = Color::fromString("#ccc");
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

void ScrollObject::paintScrollbarBackground(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	Color color(0xf0, 0xf0, 0xf0);
	painter->drawBox(rect, EdgeOffsetF(), CornerRadiusF(), color, color);
}

void ScrollObject::paintVScrollbarTopButton(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	if (!sd->vtop_button_bitmap_) {
		sd->vtop_button_bitmap_ = graph2d::createBitmap(VSCROLL_TOP_BUTTON_PNG);
	}
	const auto& color = subControlColorForFlags(SubControl::VStartButton, sd->scrollbar_flags);
	auto brect = scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.top + rect.width());
	painter->drawBox(brect, EdgeOffsetF(), CornerRadiusF(), color, color, sd->vtop_button_bitmap_.get());
}

void ScrollObject::paintVScrollbarBottomButton(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	if (!sd->vbottom_button_bitmap_) {
		sd->vbottom_button_bitmap_ = graph2d::createBitmap(VSCROLL_BOTTOM_BUTTON_PNG);
	}
	const auto& color = subControlColorForFlags(SubControl::VEndButton, sd->scrollbar_flags);
	auto brect = scene2d::RectF::fromLTRB(rect.left, rect.bottom - rect.width(), rect.right, rect.bottom);
	painter->drawBox(brect, EdgeOffsetF(), CornerRadiusF(), color, color, sd->vbottom_button_bitmap_.get());
}

void ScrollObject::paintVScrollbarThumb(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	const auto& color = subControlColorForFlags(SubControl::VThumb, sd->scrollbar_flags);
	float factor = (rect.height() - 2.0f * SCROLLBAR_GUTTER_WIDTH) / sd->content_size.height;
	float y1 = SCROLLBAR_GUTTER_WIDTH + sd->scroll_offset.y * factor;
	float y2 = SCROLLBAR_GUTTER_WIDTH + (sd->scroll_offset.y + sd->viewport_size.height) * factor;
	painter->drawBox(scene2d::RectF::fromLTRB(rect.left + 1, rect.top + y1, rect.right - 1, rect.top + y2),
		EdgeOffsetF(), CornerRadiusF(), color, color);
}

const Color& ScrollObject::subControlColorForFlags(SubControl sc, uint32_t flags_)
{
	static const Color NO_COLOR(0, 0, 0, 0);
	const auto flags = subControlFlags(sc, flags_);
	if (sc == SubControl::VThumb || sc == SubControl::HThumb) {
		return (flags & StateFlag::State_Active)
			? SCROLLBAR_THUMB_ACTIVE_COLOR
			: ((flags & StateFlag::State_MouseOver) ? SCROLLBAR_THUMB_HOVER_COLOR : SCROLLBAR_THUMB_NORMAL_COLOR);
	} else if (sc == SubControl::VStartButton || sc == SubControl::VEndButton
		|| sc == SubControl::HStartButton || sc == SubControl::HEndButton) {
		return (flags & StateFlag::State_Active)
			? SCROLLBAR_BUTTON_ACTIVE_COLOR
			: ((flags & StateFlag::State_MouseOver) ? SCROLLBAR_BUTTON_HOVER_COLOR : SCROLLBAR_BUTTON_NORMAL_COLOR);
	} else {
		return NO_COLOR;
	}
}

uint32_t ScrollObject::subControlFlags(ScrollObject::SubControl sc, uint32_t flags)
{
	uint32_t shift = State_SubControl_ShiftBits + (uint32_t)sc;
	if ((flags & (1 << shift)) != 0) {
		return (flags & State_Mask);
	} else {
		return State_None;
	}
}

}
