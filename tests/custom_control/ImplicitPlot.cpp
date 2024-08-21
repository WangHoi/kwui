#include "ImplicitPlot.h"
#include "windows/graphics/GraphicDeviceD2D.h"
#include "windows/graphics/PaintSurfaceD2D.h"
#include "CImg.h"
#include "graph2d/graph2d.h"
#include "graph2d/PaintContextInterface.h"
#include "graph2d/PaintPathInterface.h"
using namespace cimg_library;
#define STB_IMAGE_IMPLEMENTATION4
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static const int RES = 10;
static const int PADDING_PIXELS = 3;
static const float STROKE_WIDTH = 2;

static float imp_func(float x, float y)
{
    return powf(x, 4) + powf(y, 4) - x*y - 8 * 64;
    // return fabsf(x * y) - 5;
    // return x * x + y * y - 36;
}

ImplicitPlot::ImplicitPlot()
{
    precomputeMask();
}

ImplicitPlot::~ImplicitPlot()
{
}

void ImplicitPlot::update(int w, int h)
{
    if (w != width_ || h != height_) {
        width_ = w;
        height_ = h;
        vertices_buf_.resize((w + 1) * (h + 1));
        quad_buf_.resize(w * h);
        img_buf_.resize(w * h);
    }

    // Init height map
    float mid_y = 10.0f;
    float mid_x = mid_y * float(w) / float(h);
    float scale = 20.0f / (float)std::min(w, h);
    for (int row = 0; row <= h; ++row) {
        float y = -(float)row * scale + mid_y;
        for (int col = 0; col <= w; ++col) {
            float x = (float)col * scale - mid_x;
            vertices_buf_[row * (w + 1) + col] = imp_func(x, y);
            // fprintf(stderr, "%d(%.3f), %d(%.3f) : %.3f\n", col, x, row, y, imp_func(x, y));
        }
    }

    // Marching square: https://en.wikipedia.org/wiki/Marching_squares
    for (int row = 0; row < h; ++row) {
        for (int col = 0; col < w; ++col) {
            float verts[4] = {
                vertices_buf_[row * (w + 1) + col],
                vertices_buf_[row * (w + 1) + col + 1],
                vertices_buf_[(row + 1) * (w + 1) + col + 1],
                vertices_buf_[(row + 1) * (w + 1) + col],
            };
            auto& quad = quad_buf_[row * w + col];
            quad.type = (uint8_t(verts[0] >= 0.0f) << 3) | (uint8_t(verts[1] >= 0.0f) << 2)
                | (uint8_t(verts[2] >= 0.0f) << 1) | uint8_t(verts[3] >= 0.0f);

            if (quad.type == 5 || quad.type == 10) {
                float y = -((float)row + 0.5f) * scale - mid_y;
                float x = ((float)col + 0.5f) * scale - mid_x;
                float z = imp_func(x, y);
                if (z < 0.0f) {
                    if (quad.type == 5) quad.type = 10;
                    else if (quad.type == 10) quad.type = 5;
                }
            }
        }
    }

    // Stroke
    {
        for (int row = 1; row + 1 < h; ++row) {
            for (int col = 1; col + 1 < w; ++col) {
                uint8_t quad[9] = {
                    quad_buf_[row * w + col].type, // center
                    /*
                    quad_buf_[row * w + col - 1].type, // left
                    quad_buf_[(row - 1) * w + col - 1].type, // top-left
                    quad_buf_[(row - 1) * w + col].type, // up
                    quad_buf_[(row - 1) * w + col + 1].type, // top-right
                    quad_buf_[row * w + col + 1].type, // right
                    quad_buf_[(row + 1) * w + col + 1].type, // bottom-right
                    quad_buf_[(row + 1) * w].type, // down
                    quad_buf_[(row + 1) * w + col - 1].type, // bottom-left
                    */
                };

                bool fill = false;
                for (auto& q : quad) {
                    if (q != 0 && q != 15) {
                        fill = true;
                        break;
                    }
                }
                img_buf_[row * w + col] = fill ? 0xff : 0;
            }
        }
        stbi_write_png("dump_plot.png", width_, height_, 1, img_buf_.data(), width_);
    }
    {
        for (int row = 0; row < h; ++row) {
            for (int col = 0; col < w; ++col) {
                Mask m;
                for (auto [xoff, yoff] : index_offset_) {
                    int x = col - xoff;
                    int y = row - yoff;
                    if (0 <= y && y < h && 0 <= x && x < w) {
                        const auto& masks = masks_[quad_buf_[y * w + x].type];
                        m |= masks[offsetToIndex(-xoff, -yoff)];
                    }
                }
                auto c =std::clamp<int>(m.coverage() * 255.0f, 0, 255);
                img_buf_[row * w + col] = (uint8_t)std::clamp<int>(m.coverage() * 255.0f, 0, 255);
            }
        }
        stbi_write_png("dump_plot_aa.png", width_, height_, 1, img_buf_.data(), width_);
    }
}

void ImplicitPlot::dump()
{
    stbi_write_png("dump_plot.png", width_, height_, 1, img_buf_.data(), width_);
}

void ImplicitPlot::computeMaskOffset(int layers)
{
    /* Indices:
     * 12 13 14 15 16
     * 11  2  3  4 17
     * 10  1  0  5 18
     *  9  8  7  6 19
     * 24 23 22 21 20
     */
    index_offset_.reserve(layers * layers);
    index_offset_.emplace_back(0, 0);
    for (int l = 1; l < layers; ++l) {
        int n = l * 2;
        // left: bottom-up: 1, 2; 9, 10, 11, 12
        for (int i = 0; i < n; ++i) {
            index_offset_.emplace_back(-l, l - 1 - i);
        }
        // top: left-right: 3, 4; 13, 14, 15, 16
        for (int i = 0; i < n; ++i) {
            index_offset_.emplace_back(-l + 1 + i, -l);
        }
        // right: top-down: 5, 6; 17, 18, 19, 20
        for (int i = 0; i < n; ++i) {
            index_offset_.emplace_back(l, 1 - l + i);
        }
        // bottom: right-left: 7, 8; 21, 22, 23, 24
        for (int i = 0; i < n; ++i) {
            index_offset_.emplace_back(l - 1 - i, l);
        }
    }
}

void ImplicitPlot::computeAllMasks(std::vector<Mask>& masks, const uint8_t* buf, size_t stride)
{
    masks.clear();
    masks.reserve(index_offset_.size());

    for (const auto& [xoff, yoff] : index_offset_) {
        int sx = (PADDING_PIXELS - xoff) * RES;
        int sy = (PADDING_PIXELS - yoff) * RES;
        masks.push_back(computeImageMask(buf + sy * stride + sx, stride));
    }
    int kk = 1;
}

void ImplicitPlot::precomputeMask()
{
    const int layers = 1 + (int)ceilf(0.5f * STROKE_WIDTH);
    computeMaskOffset(layers);

    int size = (PADDING_PIXELS * 2 + 1) * RES;
    auto pixmap = windows::graphics::PaintSurfaceBitmapD2D::create(
        {DXGI_FORMAT_A8_UNORM, {(float)size, (float)size}, 1.0f});

    graph2d::PaintBrush brush;
    brush.setColor(style::Color(0xff, 0xff, 0xff, 0xff));

    for (uint8_t type = 0; type < 16; ++type) {
        auto p = createMaskPath(type, STROKE_WIDTH);
        if (!p) {
            masks_[type].resize(index_offset_.size());
            continue;
        }

        auto ctx = pixmap->beginPaint();
        ctx->clear(style::Color(0, 0, 0, 0));
        ctx->drawPath(p.get(), brush);
        bool ok = pixmap->endPaint();

        if (auto data = pixmap->map()) {
            UINT buf_size;
            uint8_t* buf = nullptr;
            UINT stride;
            data->GetDataPointer(&buf_size, &buf);
            data->GetStride(&stride);

            // dump to file
            char fname[128] = {};
            snprintf(fname, _countof(fname) - 1, "dump_mask_%d.png", (int)type);
            stbi_write_png(fname, size, size, 1, buf, stride);

            // compute mask
            computeAllMasks(masks_[type], buf, stride);
        }
    }


}

std::unique_ptr<graph2d::PaintPathInterface> ImplicitPlot::createMaskPath(uint8_t type, float stroke_width)
{
    float hp = 0.5f * float(RES);
    float d = stroke_width * float(RES);
    float r = 0.5f * d;
    float cx = 0.5f * float((PADDING_PIXELS * 2 + 1) * RES);
    const float rr = 0.5f * sqrtf(2.0f) * r;
    if (type == 1 || type == 14) {
        auto p = graph2d::createPath();
        auto x1 = cx - hp;
        auto y1 = cx;
        auto x2 = cx;
        auto y2 = cx + hp;
        p->moveTo(x1 - rr, y1 + rr);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x1 + rr, y1 - rr);
        p->lineTo(x2 + rr, y2 - rr);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x2 - rr, y2 + rr);
        p->close();
        return p;
    } else if (type == 2 || type == 13) {
        auto p = graph2d::createPath();
        auto x1 = cx + hp;
        auto y1 = cx;
        auto x2 = cx;
        auto y2 = cx + hp;
        p->moveTo(x1 - rr, y1 - rr);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x1 + rr, y1 + rr);
        p->lineTo(x2 + rr, y2 + rr);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x2 - rr, y2 - rr);
        p->close();
        return p;
    } else if (type == 3 || type == 12) {
        auto p = graph2d::createPath();
        auto x1 = cx - hp;
        auto y1 = cx;
        auto x2 = cx + hp;
        auto y2 = cx;
        p->moveTo(x1, y1 + r);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x1, y1 - r);
        p->lineTo(x2, y2 - r);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x2, y2 + r);
        p->close();
        return p;
    } else if (type == 4 || type == 11) {
        auto p = graph2d::createPath();
        auto x1 = cx;
        auto y1 = cx - hp;
        auto x2 = cx + hp;
        auto y2 = cx;
        p->moveTo(x1 - rr, y1 + rr);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x1 + rr, y1 - rr);
        p->lineTo(x2 + rr, y2 - rr);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x2 - rr, y2 + rr);
        p->close();
        return p;
    } else if (type == 5 || type == 10) {
        auto p = graph2d::createPath();

        auto x1 = cx - hp;
        auto y1 = cx;
        auto x2 = cx;
        auto y2 = cx - hp;
        auto x3 = cx + hp;
        auto y3 = cx;
        auto x4 = cx;
        auto y4 = cx + hp;
        p->moveTo(x1 - rr, y1 + rr);
        p->arcTo(r, r, 90,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x1 - rr, y1 - rr);
        p->lineTo(x2 - rr, y2 - rr);
        p->arcTo(r, r, 90,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x2 + rr, y2 - rr);
        p->lineTo(x3 + rr, y3 - rr);
        p->arcTo(r, r, 90,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x3 + rr, y3 + rr);
        p->lineTo(x4 + rr, y4 + rr);
        p->arcTo(r, r, 90,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x4 - rr, y4 + rr);
        p->close();

        return p;
    } else if (type == 6 || type == 9) {
        auto p = graph2d::createPath();
        auto x1 = cx;
        auto y1 = cx - hp;
        auto x2 = cx;
        auto y2 = cx + hp;
        p->moveTo(x1 - r, y1);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x1 + r, y1);
        p->lineTo(x2 + r, y2);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x2 - r, y2);
        p->close();
        return p;
    } else if (type == 7 || type == 8) {
        auto p = graph2d::createPath();
        auto x1 = cx;
        auto y1 = cx - hp;
        auto x2 = cx - hp;
        auto y2 = cx;
        p->moveTo(x1 - rr, y1 - rr);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x1 + rr, y1 + rr);
        p->lineTo(x2 + rr, y2 + rr);
        p->arcTo(r, r, 180,
                 kwui::SWEEP_DIRECTION_CLOCKWISE, kwui::ARC_SIZE_SMALL,
                 x2 - rr, y2 - rr);
        p->close();
        return p;
    }
    return nullptr;
}

ImplicitPlot::Mask ImplicitPlot::computeImageMask(const uint8_t* buf, size_t stride)
{
    Mask m;
    for (int row = 0; row < RES; ++row) {
        for (int col = 0; col < RES; ++col) {
            if (buf[col] >= 128) {
                m.setBit(col, row);
            }
        }
        buf += stride;
    }
    return m;
}

size_t ImplicitPlot::offsetToIndex(int x, int y) const
{
    for (size_t i = 0; i < index_offset_.size(); ++i) {
        auto [ix, iy] = index_offset_[i];
        if (ix == x && iy == y)
            return i;
    }
    assert(0 && "offsetToIndex(); failed");
    return 0;
}

ImplicitPlot::Mask& ImplicitPlot::Mask::operator|=(const Mask& o)
{
    raw[0] |= o.raw[0];
    raw[1] |= o.raw[1];
    raw[2] |= o.raw[2];
    raw[3] |= o.raw[3];
    return *this;
}

void ImplicitPlot::Mask::setBit(int x, int y)
{
    int n = y * RES + x;
    int i = n / 32;
    unsigned j = unsigned(n) % 32;
    raw[i] |= (1u << j);
}

float ImplicitPlot::Mask::coverage() const
{
    int n = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            if (raw[i] & (1 << j))
                ++n;
        }
    }
    return float(n) / float(RES * RES);
}
