#pragma once
#include "windows/windows_header.h"
#include "scene2d/geom_types.h"
#include "graph2d/TextLayout.h"
#include <memory>
#include <string>

namespace windows {
namespace graphics {

enum class FontStyle {
	REGULAR,
	ITALIC,
};

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

class FontWeight {
public:
	FontWeight();
	FontWeight(uint16_t raw) : _raw(raw) {}
	inline uint16_t GetRaw() const {
		return _raw;
	}
	inline bool operator==(const FontWeight& rhs) {
		return rhs._raw == this->_raw;
	}
	inline bool operator<(const FontWeight& rhs) {
		return rhs._raw < this->_raw;
	}
	static FontWeight THIN;
	static FontWeight EXTRA_LIGHT;
	static FontWeight LIGHT;
	static FontWeight SEMI_LIGHT;
	static FontWeight NORMAL;
	static FontWeight MEDIUM;
	static FontWeight SEMI_BOLD;
	static FontWeight BOLD;
	static FontWeight HEAVY;
private:
	uint16_t _raw;
};

class TextLayout;
class TextLayoutBuilder {
public:
	TextLayoutBuilder(const std::string& text);
	TextLayoutBuilder(const std::wstring& text);
	inline TextLayoutBuilder& MaxWidth(float w) {
		_max_width = w;
		return *this;
	}
	inline TextLayoutBuilder& SetFontFamily(const std::string& name) {
		_font_family = name;
		return *this;
	}
	inline TextLayoutBuilder& FontSize(float s) {
		_font_size = s;
		return *this;
	}
	inline TextLayoutBuilder& FontStyle(FontStyle s) {
		_font_style = s;
		return *this;
	}
	inline TextLayoutBuilder& FontWeight(FontWeight w) {
		_font_weight = w;
		return *this;
	}
	inline TextLayoutBuilder& Align(TextAlignment align) {
		_align = align;
		return *this;
	}
	std::unique_ptr<TextLayout> Build();
private:
	void Init(const std::wstring& text);
	std::wstring _text;
	float _max_width;
	std::string _font_family;
	float _font_size;
	::windows::graphics::FontStyle _font_style;
	::windows::graphics::FontWeight _font_weight;
	TextAlignment _align;
};

class TextLayout : public graph2d::TextLayoutInterface {
public:
	TextLayout(const std::wstring& text, const std::string& font_family,
			   float font_size, ComPtr<IDWriteTextLayout> layout);
	float lineHeight() const override { return _line_height; }
	float baseline() const override { return _baseline; }
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
extern const float DEFAULT_FONT_SIZE;

} // namespace graphics
} // namespace windows
