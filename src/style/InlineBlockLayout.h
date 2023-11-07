#pragma once
#include "Layout.h"
#include "StyleValue.h"
#include "InlineLayout.h"
#include "BlockLayout.h"
#include "scene2d/geom_types.h"
#include "base/log.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "graph2d/TextLayout.h"
#include <vector>
#include <memory>
#include <string>

namespace scene2d {
class Node;
}

namespace style {

struct LayoutObject;

struct InlineBlockBox {
	std::vector<InlineBox> inline_boxes;
	BlockBox block_box;

	template <typename Sink>
	friend void AbslStringify(Sink& sink, const InlineBlockBox& o) {
		absl::Format(&sink, "InlineBlockBox { ");
		absl::Format(&sink, "inlines=[ ");
		for (auto& ib : o.inline_boxes) {
			absl::Format(&sink, "%v, ", ib);
		}
		absl::Format(&sink, "], block=%v ", o.block_box);
		absl::Format(&sink, "}");
	}
};

} // namespace style
