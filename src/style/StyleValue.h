#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <optional>
#include "base/base.h"

namespace style {

enum class ValueUnit {
	Undefined,

	Raw,

	Pixel,
	Point,
	Percent,

	HexColor,
	Url,

	Keyword,
};

struct Value {
	float f32_val = 0.0f;
	base::string_atom keyword_val;
	ValueUnit unit = ValueUnit::Undefined;
	std::string string_val;

	static Value fromKeyword(base::string_atom k)
	{
		Value v;
		v.keyword_val = k;
		v.unit = ValueUnit::Keyword;
	}
	static Value fromPixel(float val)
	{
		return fromUnit(val, ValueUnit::Pixel);
	}
	static Value fromUnit(float val, ValueUnit u)
	{
		Value v;
		v.f32_val = val;
		v.unit = u;
		return v;
	}
	static Value fromHexColor(const std::string &s)
	{
		Value v;
		v.string_val = s;
		v.unit = ValueUnit::HexColor;
	}
	static Value fromUrl(const std::string &url)
	{
		Value v;
		v.string_val = url;
		v.unit = ValueUnit::Url;
	}
};

enum class ValueSpecType {
	Inherit,
	Unset,
	Initial,
	Specified,
};

struct ValueSpec {
	ValueSpecType type = ValueSpecType::Unset;
	std::optional<Value> value;
};

struct StyleSpec {
	ValueSpec display;
	ValueSpec position;

	ValueSpec margin_left;
	ValueSpec margin_top;
	ValueSpec margin_right;
	ValueSpec margin_bottom;
	ValueSpec border_left_width;
	ValueSpec border_top_width;
	ValueSpec border_right_width;
	ValueSpec border_bottom_width;
	ValueSpec padding_left;
	ValueSpec padding_top;
	ValueSpec padding_right;
	ValueSpec padding_bottom;

	ValueSpec left;
	ValueSpec top;
	ValueSpec right;
	ValueSpec bottom;
	ValueSpec min_width;
	ValueSpec min_height;
	ValueSpec max_width;
	ValueSpec max_height;
	
	ValueSpec width;
	ValueSpec height;

	ValueSpec border_top_left_radius;
	ValueSpec border_top_right_radius;
	ValueSpec border_bottom_right_radius;
	ValueSpec border_bottom_left_radius;

	ValueSpec background_color;
	ValueSpec background_image;
	ValueSpec border_color;
};

enum class DisplayType {
	Block,
	Inline,
	InlineBlock,
	None,
};

enum class PositionType {
	Static,
	Relative,
	Absolute,
	Fixed,
};

struct Style {
	DisplayType display = DisplayType::Block;
	PositionType position = PositionType::Static;
	Value left;
	Value top;
	Value right;
	Value bottom;
	Value min_width;
	Value min_height;
	Value max_width;
	Value max_height;

	Value width;
	Value height;

	Value margin_left;
	Value margin_top;
	Value margin_right;
	Value margin_bottom;

	Value border_left_width;
	Value border_top_width;
	Value border_right_width;
	Value border_bottom_width;

	Value padding_left;
	Value padding_top;
	Value padding_right;
	Value padding_bottom;

	Value border_top_left_radius;
	Value border_top_right_radius;
	Value border_bottom_right_radius;
	Value border_bottom_left_radius;
};

}
