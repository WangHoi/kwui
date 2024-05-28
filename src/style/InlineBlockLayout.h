#pragma once
#include "Layout.h"
#include "StyleValue.h"
#include "InlineLayout.h"
#include "BlockLayout.h"
#include "TextFlow.h"
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

struct LayoutObject;

struct InlineBlockBox {
	std::unique_ptr<InlineBox> inline_box;
	BlockBox block_box;

	template <typename Sink>
	friend void AbslStringify(Sink& sink, const InlineBlockBox& o) {
		absl::Format(&sink, "InlineBlockBox { ");
		absl::Format(&sink, "inline=%v, ", *o.inline_box);
		absl::Format(&sink, "block=%v ", o.block_box);
		absl::Format(&sink, "}");
	}

	InlineBlockBox()
	{
		inline_box = std::make_unique<InlineBox>();
	}
	InlineBlockBox(const InlineBlockBox&) = delete;
	InlineBlockBox& operator=(const InlineBlockBox&) = delete;
};

} // namespace style
