#include "StyleValue.h"
#include "graph2d/graph2d.h"

namespace style {
Value Value::auto_()
{
	return Value::fromKeyword(base::string_intern("auto"));
}
Value Value::fromKeyword(base::string_atom k)
{
	Value v;
	v.keyword_val = k;
	v.unit = ValueUnit::Keyword;
	return v;
}

Value Value::fromPixel(float val)
{
	return fromUnit(val, ValueUnit::Pixel);
}

Value Value::fromUnit(float val, ValueUnit u)
{
	Value v;
	v.f32_val = val;
	v.unit = u;
	return v;
}

Value Value::fromHexColor(const std::string& s)
{
	Value v;
	v.string_val = s;
	v.unit = ValueUnit::HexColor;
	return v;
}

Value Value::fromUrl(const std::string& url)
{
	Value v;
	v.string_val = url;
	v.unit = ValueUnit::Url;
	return v;
}

void StyleSpec::set(base::string_atom prop, const ValueSpec& val)
{
#define CHECK_VALUE(x) if (prop == base::string_intern(#x)) { this->x = val; }
#define CHECK_VALUE2(x, name) if (prop == base::string_intern(name)) { this->x = val; }
	CHECK_VALUE(display);
	CHECK_VALUE(position);

	CHECK_VALUE2(margin_left, "margin-left");
	CHECK_VALUE2(margin_top, "margin-top");
	CHECK_VALUE2(margin_right, "margin-right");
	CHECK_VALUE2(margin_bottom, "margin-bottom");

	CHECK_VALUE2(border_left_width, "border-left-width");
	CHECK_VALUE2(border_top_width, "border-top-width");
	CHECK_VALUE2(border_right_width, "border-right-width");
	CHECK_VALUE2(border_bottom_width, "border-bottom-width");

	CHECK_VALUE2(border_top_left_radius, "border-top-left-radius");
	CHECK_VALUE2(border_top_right_radius, "border-top-right-radius");
	CHECK_VALUE2(border_bottom_right_radius, "border-bottom-right-radius");
	CHECK_VALUE2(border_bottom_left_radius, "border-bottom-left-radius");

	CHECK_VALUE2(padding_left, "padding-left");
	CHECK_VALUE2(padding_top, "padding-top");
	CHECK_VALUE2(padding_right, "padding-right");
	CHECK_VALUE2(padding_bottom, "padding-bottom");

	CHECK_VALUE(left);
	CHECK_VALUE(top);
	CHECK_VALUE(right);
	CHECK_VALUE(bottom);

	CHECK_VALUE(width);
	CHECK_VALUE(height);

	CHECK_VALUE2(border_color, "border-color");
	CHECK_VALUE2(background_color, "background-color");
	CHECK_VALUE(color);
	CHECK_VALUE2(background_image, "background-image");

	CHECK_VALUE2(line_height, "line-height");
	CHECK_VALUE2(font_family, "font-family");
	CHECK_VALUE2(font_size, "font-size");
	CHECK_VALUE2(font_style, "font-style");
	CHECK_VALUE2(font_weight, "font-weight");
	CHECK_VALUE2(text_align, "text-align");
	CHECK_VALUE2(vertical_align, "vertical-align");

	CHECK_VALUE2(overflow_x, "overflow-x");
	CHECK_VALUE2(overflow_y, "overflow-y");

	CHECK_VALUE2(box_sizing, "box-sizing");

	CHECK_VALUE(cursor);
#undef CHECK_VALUE
#undef CHECK_VALUE2
}

void Style::resolveDefault(const Style* parent)
{
#define RESOLVE_STYLE_DEFAULT(x, def) \
    this->x = def

	RESOLVE_STYLE_DEFAULT(margin_left, style::Value::fromPixel(0));
	RESOLVE_STYLE_DEFAULT(margin_top, style::Value::fromPixel(0));
	RESOLVE_STYLE_DEFAULT(margin_right, style::Value::fromPixel(0));
	RESOLVE_STYLE_DEFAULT(margin_bottom, style::Value::fromPixel(0));
	RESOLVE_STYLE_DEFAULT(border_left_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_right_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_left_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_top_right_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_right_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(border_bottom_left_radius, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_left, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_top, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_right, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(padding_bottom, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(left, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(top, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(right, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(bottom, style::Value::auto_());

	RESOLVE_STYLE_DEFAULT(min_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(min_height, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(max_width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(max_height, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(width, style::Value::auto_());
	RESOLVE_STYLE_DEFAULT(height, style::Value::auto_());

	RESOLVE_STYLE_DEFAULT(border_color, style::Color());
	RESOLVE_STYLE_DEFAULT(background_color, style::Color());
	RESOLVE_STYLE_DEFAULT(background_image, nullptr);

	RESOLVE_STYLE_DEFAULT(overflow_x, style::OverflowType::Visible);
	RESOLVE_STYLE_DEFAULT(overflow_y, style::OverflowType::Visible);
#undef RESOLVE_STYLE_DEFAULT

#define RESOLVE_STYLE_DEFAULT_INHERIT(x, def) \
    this->x = (parent ? parent->x : def)

	RESOLVE_STYLE_DEFAULT_INHERIT(color, style::named_color::black);
	RESOLVE_STYLE_DEFAULT_INHERIT(line_height, style::Value::fromKeyword(base::string_intern("normal")));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_family, style::Value::fromKeyword(base::string_intern("Microsoft YaHei")));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_size, style::Value::fromPixel(12));
	RESOLVE_STYLE_DEFAULT_INHERIT(font_style, style::FontStyle::Normal);
	RESOLVE_STYLE_DEFAULT_INHERIT(font_weight, style::FontWeight());
	RESOLVE_STYLE_DEFAULT_INHERIT(text_align, style::TextAlign::Left);
	RESOLVE_STYLE_DEFAULT_INHERIT(cursor, style::CursorType::Auto);
#undef RESOLVE_STYLE_DEFAULT_INHERIT
}

float Style::fontSizeInPixels() const
{
	return font_size.pixelOrZero();
}

float Style::lineHeightInPixels() const
{
	if (line_height.isRaw()) {
		return fontSizeInPixels() * line_height.f32_val;
	} else if (line_height.isPercent()) {
		return fontSizeInPixels() * line_height.f32_val / 100.0f;
	} else if (line_height.isPixel()) {
		return line_height.f32_val;
	} else {
		auto metric = graph2d::getFontMetrics(fontFamily().c_str(), fontSizeInPixels());
		return metric.lineHeight();
	}
}

std::string Style::fontFamily() const
{
	if (font_family.unit == ValueUnit::Keyword) {
		return font_family.isAuto() ? std::string() : font_family.keyword_val.c_str();
	}
	return std::string();
}

}
