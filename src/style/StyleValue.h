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

	Auto,
	Keyword,
};

struct Value {
	float f32_val = 0.0f;
	base::string_atom keyword_val;
	ValueUnit unit = ValueUnit::Undefined;
};

enum class ValueSpecType {
	Unset,
	Inherit,
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
	ValueSpec border_left;
	ValueSpec border_top;
	ValueSpec border_right;
	ValueSpec border_bottom;
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
	Value border_left;
	Value border_top;
	Value border_right;
	Value border_bottom;
	Value padding_left;
	Value padding_top;
	Value padding_right;
	Value padding_bottom;
};

}
