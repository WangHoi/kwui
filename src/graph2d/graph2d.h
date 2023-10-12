#pragma once

#include "TextLayout.h"
#include "Painter.h"
#include <memory>
#include <string>

namespace graph2d {

std::unique_ptr<TextLayoutInterface> createTextLayout(const std::string &text);

}