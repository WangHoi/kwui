#pragma once
#include "scene2d/geom_types.h"
#include "style/TextFlow.h"
#include <memory>
#include <string>

namespace scene2d {

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

class TextLayoutInterface;
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
	inline TextLayoutBuilder& FontStyle(style::FontStyle s) {
		_font_style = s;
		return *this;
	}
	inline TextLayoutBuilder& FontWeight(style::FontWeight w) {
		_font_weight = w;
		return *this;
	}
	inline TextLayoutBuilder& Align(TextAlignment align) {
		_align = align;
		return *this;
	}
	std::unique_ptr<TextLayoutInterface> Build();
private:
	void Init(const std::wstring& text);
	std::wstring _text;
	float _max_width;
	std::string _font_family;
	float _font_size;
	style::FontStyle _font_style;
	style::FontWeight _font_weight;
	TextAlignment _align;
};

class TextLayoutInterface {
public:
	virtual ~TextLayoutInterface() {}
	virtual float lineHeight() const = 0;
	virtual float baseline() const = 0;
	virtual scene2d::RectF rect() const = 0;
	virtual scene2d::RectF caretRect(int idx) const = 0;
	// |start_idx| maybe larger than |end_idx|
	virtual scene2d::RectF rangeRect(int start_idx, int end_idx) const = 0;
	virtual int hitTest(const scene2d::PointF& pos, scene2d::RectF* out_caret_rect = nullptr) const = 0;
	//inline IDWriteTextLayout *GetRaw() const { return _layout.Get(); }
};
extern const float DEFAULT_FONT_SIZE;
extern const char* DEFAULT_FONT_FAMILY;

} // namespace scene2d
