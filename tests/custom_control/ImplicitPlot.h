#pragma once

#include <memory>
#include <vector>
#include "graph2d/graph2d.h"

class ImplicitPlot
{
public:
    ImplicitPlot();
    ~ImplicitPlot();
    void update(int w, int h);
    void dump();
    void precomputeMask();

private:
    std::unique_ptr<graph2d::PaintPathInterface> createMaskPath(uint8_t type, float stroke_width);

    struct Quad
    {
        uint8_t type = 0;
    };

    int width_ = 0;
    int height_ = 0;
    std::vector<float> vertices_buf_;
    std::vector<Quad> quad_buf_;
    std::vector<uint8_t> img_buf_;
};
