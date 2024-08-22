#pragma once

#include <memory>
#include <vector>
#include "graph2d/graph2d.h"

class ImplicitPlot
{
public:
    ImplicitPlot();
    ~ImplicitPlot();
    void update(float stroke_width, int w, int h,
                std::tuple<float, float> x_range,
                std::tuple<float, float> y_range);
    void dump();
    void computeMaskOffset(int layers);
    void updateMask(float stroke_width);
    const void* imageData() const
    {
        return img_buf_.data();
    }

private:
    std::unique_ptr<graph2d::PaintPathInterface> createMaskPath(uint8_t type, float stroke_width);
    struct Mask;
    void setMaskBit(Mask& mask, int row, int col);
    void computeAllMasks(std::vector<Mask>& masks, const uint8_t* buf, size_t stride);
    Mask computeImageMask(const uint8_t* buf, size_t stride);
    size_t offsetToIndex(int x, int y) const;

    struct Quad
    {
        uint8_t type = 0;
    };

    struct Mask
    {
        std::array<uint32_t, 4> raw;

        Mask()
        {
            raw.fill(0);
        }

        Mask& operator |=(const Mask& o);

        void setBit(int x, int y);

        float coverage() const;
    };

    int width_ = 0;
    int height_ = 0;
    float stroke_width_ = 0;
    std::vector<float> vertices_buf_;
    std::vector<Quad> quad_buf_;
    std::vector<uint32_t> img_buf_;
    std::vector<std::tuple<int, int>> index_offset_;
    std::array<std::vector<Mask>, 16> masks_;
    bool dump_image_ = false;
};
