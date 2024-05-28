#pragma once
#include "windows/windows_header.h"
#include "scene2d/geom_types.h"
#include "scene2d/TextLayout.h"
#include "style/TextFlow.h"
#include <memory>
#include <string>

namespace windows {
namespace graphics {

enum TextAlignment {
	TEXT_ALIGN_LEFT = 1,
	TEXT_ALIGN_HCENTER = 2,
	TEXT_ALIGN_RIGHT = 4,
	TEXT_ALIGN_TOP = 8,
	TEXT_ALIGN_VCENTER = 16,
	TEXT_ALIGN_BOTTOM = 32,
	
	TEXT_ALIGN_TOP_LEFT = TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP,
	TEXT_ALIGN_CENTER = TEXT_ALIGN_HCENTER | TEXT_ALIGN_VCENTER,
};

class TextLayoutD2D : public scene2d::TextLayoutInterface
{
public:
	TextLayoutD2D(const std::wstring& text, const std::string& font_family,
			      float font_size, ComPtr<IDWriteTextLayout> layout);
	float lineHeight() const override { return _line_height; }
	float baseline() const override  { return _baseline; }
	scene2d::RectF rect() const override { return _rect; }
	scene2d::RectF caretRect(int idx) const override;
	scene2d::RectF rangeRect(int start_idx, int end_idx) const override;
	int hitTest(const scene2d::PointF& pos, scene2d::RectF* out_caret_rect = nullptr) const override;

	inline IDWriteTextLayout *GetRaw() const { return _layout.Get(); }

private:
	void UpdateTextMetrics();
	scene2d::RectF MakeCaretRect(const DWRITE_HIT_TEST_METRICS* htm) const;
	std::wstring _text;
	std::string _font_family;
	float _font_size;
	ComPtr<IDWriteTextLayout> _layout;
	float _line_height;
	float _baseline;
	scene2d::RectF _rect;
};

} // namespace graphics
} // namespace windows
