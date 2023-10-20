#pragma once
#include "InlineLayout.h"
#include "graph2d/TextLayout.h"
#include "style/StyleValue.h"
#include <vector>
#include <memory>
#include <string>

namespace style {

struct TextBoxes
{
	std::vector<std::unique_ptr<graph2d::GlyphRunInterface>> glyph_runs;
	std::vector<std::unique_ptr<InlineBox>> inline_boxes;
};

}
