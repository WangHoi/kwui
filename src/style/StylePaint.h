#pragma once
#include "scene2d/geom_types.h"
#include <vector>

namespace scene2d {
class Node;
}

namespace style {

struct BlockPaintContext {
	scene2d::PointF root_scene_pos;
	scene2d::PointF contg_pos;	// relative to BFC
	scene2d::PointF positioned_contg_pos;	// relative to BFC
};

}
