#include "StyleValue.h"

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

	CHECK_VALUE2(line_height, "line-height");
	CHECK_VALUE2(font_family, "font-family");
	CHECK_VALUE2(font_size, "font-size");
	CHECK_VALUE2(font_style, "font-style");
	CHECK_VALUE2(font_weight, "font-weight");
	CHECK_VALUE2(text_align, "text-align");

	CHECK_VALUE2(overflow_x, "overflow-x");
	CHECK_VALUE2(overflow_y, "overflow-y");
#undef CHECK_VALUE
#undef CHECK_VALUE2
}

}
