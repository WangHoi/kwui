#pragma once
#include "geom_types.h"

namespace scene2d
{

class Node;
struct BlockFormatContext {
    // containing block size
    DimensionF contg_block_size;
    // block size of (absolute positioned parent) 
    DimensionF abs_pos_parent_block_size;
    // absolute positioned parent    
    Node* abs_pos_parent = nullptr;
};

} // namespace scene2d
