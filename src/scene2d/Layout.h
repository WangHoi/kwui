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
    //std::optional<RectF> abs_pos;
    
    std::vector<BlockBox*> children;
    /* Static: relative to BFC origin
     * Absolute: zero */ 
    PointF pos;
};

struct BlockFormatContext {
    // Owner of this BFC
    Node* owner = nullptr;
    // containing block width
    float contg_block_width;
    // containing block height
    absl::optional<float> contg_block_height;

    float border_bottom = 0;
    float margin_bottom = 0;

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
float collapse_margin(float m1, float m2);

} // namespace scene2d
