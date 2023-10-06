#pragma once
#include "geom_types.h"
#include "base/log.h"
#include <vector>
#include <memory>
#include <string>

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

struct InlineBox {
    PointF offset; // set by parent
    
    DimensionF size;
    float baseline = 0; // offset from bottom
};

struct LineBox {
    PointF offset; // set by parent
    
    float left;
    float avail_width;
    // float line_gap;  // leading
    
    DimensionF used_size;
    float used_baseline; // offset from used_size's bottom

    std::vector<InlineBox> inline_boxes;

    LineBox(float left_, float avail_w)
        : left(left_)
        , avail_width(avail_w)
        , used_baseline(0)
    {
    }
    int addInlineBox(const InlineBox &box)
    {
        int idx = (int)inline_boxes.size();
        inline_boxes.push_back(box);
        return idx;
    }
    void updateInlineBox(int idx, const InlineBox &box)
    {
        CHECK(idx >= 0 && idx < (int)inline_boxes.size())
            << "invalid inline box index.";
        inline_boxes[idx] = box;
    }
    std::tuple<float, float> getAvailWidth() const
    {
        float x = left;
        float w = avail_width;
        for (auto& b : inline_boxes) {
            x += b.size.width;
            w -= b.size.width;
        }
        return std::make_tuple(x, w);
    }
    float remainWidth() const
    {
        float w = avail_width;
        for (auto& b : inline_boxes) {
            w -= b.size.width;
        }
        return w;
    }
    void finish()
    {
        used_size.width = 0;
        float top = 0, bottom = 0;
        for (auto& b : inline_boxes) {
            used_size.width += b.size.width;
            top = std::max(top, b.size.width - b.baseline);
            bottom = std::max(bottom, b.baseline);
        }
        used_size.height = top + bottom;
        used_baseline = bottom;

        float x = left;
        for (auto& b : inline_boxes) {
            b.offset.x = x;
            x += b.size.width;
        }
    }
};

class InlineFormatContext {
public:
private:
    LineBox* newLineBox()
    {
        auto lb = std::make_unique<LineBox>(0, avail_width);
        auto ret = lb.get();
        line_boxes.emplace_back(std::move(lb));
        return ret;
    }
    LineBox* getLineBox(float pref_min_width)
    {
        if (line_boxes.empty())
            return newLineBox();
        LineBox* lb = line_boxes.back().get();
        if (lb->inline_boxes.empty() || lb->remainWidth() >= pref_min_width)
            return lb;
        return newLineBox();
    }

    float avail_width;
    std::vector<std::unique_ptr<LineBox>> line_boxes;
};

} // namespace scene2d
