#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>
#include <optional>
#include "base/base.h"
#include "absl/strings/str_format.h"

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

	static Value auto_();
	static Value fromKeyword(base::string_atom k);
	static Value fromPixel(float val);
	static Value fromUnit(float val, ValueUnit u);
	static Value fromHexColor(const std::string& s);
	static Value fromUrl(const std::string& url);

	inline bool isPixel() const
	{
		return unit == ValueUnit::Pixel;
	}
	inline bool isRaw() const
	{
		return unit == ValueUnit::Raw;
	}
	inline bool isAuto() const
	{
		return isKeyword("auto");
	}
	inline bool isKeyword(absl::string_view key) const
	{
		return unit == ValueUnit::Keyword && keyword_val == base::string_intern(key);
	}
	inline float pixelOrZero() const
	{
		return (unit == ValueUnit::Pixel || unit == ValueUnit::Raw)
			? f32_val : 0.0f;
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

	ValueSpec border_color;
	ValueSpec background_color;
	ValueSpec color;
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

	Value border_color;
	Value background_color;
	Value color;
};

template <typename Sink>
void AbslStringify(Sink& sink, DisplayType p) {
	switch (p) {
	case DisplayType::None:
		absl::Format(&sink, "None");
		break;
	case DisplayType::Block:
		absl::Format(&sink, "Block");
		break;
	case DisplayType::Inline:
		absl::Format(&sink, "Inline");
		break;
	case DisplayType::InlineBlock:
		absl::Format(&sink, "InlineBox");
		break;
	default:
		absl::Format(&sink, "Custom(%d)", (int)p);
	}
}

template <typename Sink>
void AbslStringify(Sink& sink, PositionType p) {
	switch (p) {
	case PositionType::Static:
		absl::Format(&sink, "Static");
		break;
	case PositionType::Relative:
		absl::Format(&sink, "Relative");
		break;
	case PositionType::Absolute:
		absl::Format(&sink, "Absolute");
		break;
	case PositionType::Fixed:
		absl::Format(&sink, "Fixed");
		break;
	default:
		absl::Format(&sink, "Custom(%d)", (int)p);
	}
}

}
