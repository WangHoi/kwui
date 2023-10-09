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

}
