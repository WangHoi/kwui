#pragma once
#include "geom_types.h"
#include "base/log.h"
#include <vector>
#include <memory>
#include <string>

namespace scene2d
{
class Node;

struct BlockBox {
    BoxF box;
    PointF offset; // set by BFC
};

struct BlockFormatContext {
    // containing block size
    DimensionF contg_block_size;
    // block size of (absolute positioned parent) 
    DimensionF abs_pos_parent_block_size;
    // absolute positioned parent    
    Node* abs_pos_parent = nullptr;

    float offset_y = 0;
};

struct LineBox;

struct InlineBox {
    DimensionF size;
    float baseline = 0; // offset from bottom

    std::vector<InlineBox*> children;

    LineBox* line_box; // set by IFC::setupBox()
    float line_box_offset_x; // set by IFC::setupBox()

    PointF offset; // set by LineBox::layout()
};

class InlineFormatContext {
public:
    InlineFormatContext(float avail_width);
    ~InlineFormatContext();
    float getAvailWidth() const;
    void setupBox(InlineBox* box);
    
    void addBox(InlineBox* box);

    void layout();
    inline float getLayoutHeight() const
    {
        return height_;
    }
    
private:
    LineBox* newLineBox();
    LineBox* getLineBox(float pref_min_width);

    float avail_width_;
    std::vector<std::unique_ptr<LineBox>> line_boxes_;

    float height_;
};

} // namespace scene2d
