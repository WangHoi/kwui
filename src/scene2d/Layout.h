#pragma once
#include "geom_types.h"
#include "base/log.h"
#include "style/style.h"
#include "absl/types/optional.h"
#include <vector>
#include <memory>
#include <string>

namespace scene2d
{
class Node;

struct BlockBox {
    BoxF box;
    std::optional<RectF> abs_pos;
    
    std::vector<BlockBox*> children;
    /* Static: relative to BFC origin
     * Absolute: zero */ 
    PointF offset;

    inline bool isStatic() const
    {
        return !abs_pos.has_value();
    }
    inline bool isAbsolute() const
    {
        return abs_pos.has_value();
    }
};

struct BlockFormatContext {
    // Owner of this BFC
    Node* owner = nullptr;
    // containing block width
    float contg_block_width;
    // containing block width
    float contg_block_height;
    // absolute positioned parent block width
    float abs_pos_parent_block_width;
    // absolute positioned parent block height
    float abs_pos_parent_block_height;
    // absolute positioned parent    
    Node* abs_pos_parent = nullptr;

    float offset_y = 0; // normal flow y offset

    void addBox(BlockBox* box);

private:
    std::vector<BlockBox*> block_boxes_;
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
    float getLayoutWidth() const;

private:
    LineBox* newLineBox();
    LineBox* getLineBox(float pref_min_width);

    float avail_width_;
    std::vector<std::unique_ptr<LineBox>> line_boxes_;

    float height_;
};

void try_convert_to_px(style::Value& v, float percent_base);
absl::optional<float> try_resolve_to_px(const style::Value& v, float percent_base);
absl::optional<float> try_resolve_to_px(const style::Value& v, absl::optional<float> percent_base);

} // namespace scene2d
