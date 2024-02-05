#include "ScrollObject.h"
#include "scene2d/Node.h"
#include "graph2d/graph2d.h"

static const float SCROLLBAR_GUTTER_WIDTH = 16.0f;
static const char* VSCROLL_TOP_BUTTON_PNG = "D:/projects/kwui/vscroll_top_button.png";
static const char* VSCROLL_BOTTOM_BUTTON_PNG = "D:/projects/kwui/vscroll_bottom_button.png";
static const char* SCROLLBAR_NORMAL_COLOR = "#ccc";
static const char* SCROLLBAR_HOVER_COLOR = "#bbb";
static const char* SCROLLBAR_ACTIVE_COLOR = "#aaa";

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
		if (pos.y < SCROLLBAR_GUTTER_WIDTH) {
			return HitTestResult::VScrollbarTopButton;
		} else {
			float bottom_btn_top = (sd->viewport_size.width < sd->content_size.width) // has h-scrollbar
				? sd->viewport_size.height
				: (sd->viewport_size.height - SCROLLBAR_GUTTER_WIDTH);
			if (pos.y >= bottom_btn_top) {
				return HitTestResult::VScrollbarBottomButton;
			} else {
				float factor = (sd->viewport_size.height - 2.0f * SCROLLBAR_GUTTER_WIDTH) / sd->content_size.height;
				float y1 = sd->scroll_offset.y * factor;
				float y2 = (sd->scroll_offset.y + sd->viewport_size.height) * factor;
				if ((pos.y - SCROLLBAR_GUTTER_WIDTH) < y1) {
					return HitTestResult::VScrollbarTrackStartPiece;
				} else if (y1 <= (pos.y - SCROLLBAR_GUTTER_WIDTH) && (pos.y - SCROLLBAR_GUTTER_WIDTH) < y2) {
					//LOG(INFO) << "viewport_size=" << sd->viewport_size << ", content_size=" << sd->content_size;
					//LOG(INFO) << "VScrollbarThumb y1=" << y1 << ", pos.y=" << pos.y << ", y2=" << y2;
					return HitTestResult::VScrollbarThumb;
				} else {
					return HitTestResult::VScrollbarTrackEndPiece;
				}
			}
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
	auto color = Color::fromString("#f0f0f0");
	painter->drawBox(rect, EdgeOffsetF(), CornerRadiusF(), color, color);
}

void ScrollObject::paintVScrollbarTopButton(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	if (!sd->vtop_button_bitmap_) {
		sd->vtop_button_bitmap_ = graph2d::createBitmap(VSCROLL_TOP_BUTTON_PNG);
	}
	auto brect = scene2d::RectF::fromLTRB(rect.left, rect.top, rect.right, rect.top + rect.width());
	painter->drawBox(brect, EdgeOffsetF(), CornerRadiusF(), Color(), Color(), sd->vtop_button_bitmap_.get());
}

void ScrollObject::paintVScrollbarBottomButton(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	if (!sd->vbottom_button_bitmap_) {
		sd->vbottom_button_bitmap_ = graph2d::createBitmap(VSCROLL_BOTTOM_BUTTON_PNG);
	}
	auto brect = scene2d::RectF::fromLTRB(rect.left, rect.bottom - rect.width(), rect.right, rect.bottom);
	painter->drawBox(brect, EdgeOffsetF(), CornerRadiusF(), Color(), Color(), sd->vbottom_button_bitmap_.get());
}

void ScrollObject::paintVScrollbarThumb(ScrollObject* sd, graph2d::PainterInterface* painter, const scene2d::RectF& rect)
{
	const auto color = (sd->v_scrollbar_flags & StateFlag::SubControl_Thumb)
		? ((sd->v_scrollbar_flags & StateFlag::State_Active)
			? Color::fromString(SCROLLBAR_ACTIVE_COLOR)
			: ((sd->v_scrollbar_flags & StateFlag::State_MouseOver) ? Color::fromString(SCROLLBAR_HOVER_COLOR) : Color::fromString(SCROLLBAR_NORMAL_COLOR)))
		: Color::fromString(SCROLLBAR_NORMAL_COLOR);

	float factor = (rect.height() - 2.0f * SCROLLBAR_GUTTER_WIDTH) / sd->content_size.height;
	float y1 = SCROLLBAR_GUTTER_WIDTH + sd->scroll_offset.y * factor;
	float y2 = SCROLLBAR_GUTTER_WIDTH + (sd->scroll_offset.y + sd->viewport_size.height) * factor;
	painter->drawBox(scene2d::RectF::fromLTRB(rect.left, rect.top + y1, rect.right - 1, rect.top + y2),
		EdgeOffsetF(), CornerRadiusF(), color, color);

}

}
