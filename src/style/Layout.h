#pragma once
#include "scene2d/geom_types.h"
#include "base/log.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include <vector>
#include <memory>
#include <string>

namespace scene2d {
class Node;
}

namespace style {
struct Value;
struct BlockFormatContext;
class InlineFormatContext;
class InlineBox;
class BlockBox;
class BlockBoxBuilder;
struct LayoutObject;

struct EdgeOffsetF {
	float left = 0;
	float top = 0;
	float right = 0;
	float bottom = 0;
};

struct BoxF {
	// margin edges
	EdgeOffsetF margin;
	// border edges
	EdgeOffsetF border;
	// padding edges
	EdgeOffsetF padding;
	// content size
	scene2d::DimensionF content;
};

void try_convert_to_px(style::Value& v, float percent_base);
absl::optional<float> try_resolve_to_px(const style::Value& v, float percent_base);
absl::optional<float> try_resolve_to_px(const style::Value& v, absl::optional<float> percent_base);
float collapse_margin(float m1, float m2);

} // namespace style
