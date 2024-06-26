#pragma once
#include "scene2d/geom_types.h"
#include "base/log.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "absl/strings/str_format.h"
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

	template <typename Sink>
	friend void AbslStringify(Sink& sink, const EdgeOffsetF& o) {
		absl::Format(&sink, "EdgeOffsetF { ");
		if (o.left != 0.0f)
			absl::Format(&sink, "left=%.0f, ", o.left);
		if (o.top != 0.0f)
			absl::Format(&sink, "top=%.0f, ", o.top);
		if (o.right != 0.0f)
			absl::Format(&sink, "right=%.0f, ", o.right);
		if (o.bottom != 0.0f)
			absl::Format(&sink, "bottom=%.0f, ", o.bottom);
		absl::Format(&sink, "}");
	}
};

struct CornerRadiusF {
	float top_left = 0;
	float top_right = 0;
	float bottom_right = 0;
	float bottom_left = 0;

	CornerRadiusF() = default;
	explicit CornerRadiusF(float f)
		: top_left(f), top_right(f), bottom_right(f), bottom_left(f) {}

	template <typename Sink>
	friend void AbslStringify(Sink& sink, const CornerRadiusF& o) {
		absl::Format(&sink, "CornerRadiusF { ");
		if (o.top_left != 0.0f)
			absl::Format(&sink, "top_left=%.0f, ", o.top_left);
		if (o.top_right != 0.0f)
			absl::Format(&sink, "top_right=%.0f, ", o.top_right);
		if (o.bottom_right != 0.0f)
			absl::Format(&sink, "bottom_right=%.0f, ", o.bottom_right);
		if (o.bottom_left != 0.0f)
			absl::Format(&sink, "bottom_left=%.0f, ", o.bottom_left);
		absl::Format(&sink, "}");
	}
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
