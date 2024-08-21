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

static const int RES = 100;
static const int PADDING_PIXELS = 1;

static float imp_func(float x, float y)
{
    return x * x + y * y - 36;
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

            if (quad.type != 0 && quad.type != 15) {
                int ikk = 1;
            }

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
    for (int row = 1; row + 1 < h; ++row) {
        for (int col = 1; col + 1 < w; ++col) {
            uint8_t quad[9] = {
                quad_buf_[row * w + col].type, // center
                quad_buf_[row * w + col - 1].type, // left
                quad_buf_[(row - 1) * w + col - 1].type, // top-left
                quad_buf_[(row - 1) * w + col].type, // up
                quad_buf_[(row - 1) * w + col + 1].type, // top-right
                quad_buf_[row * w + col + 1].type, // right
                quad_buf_[(row + 1) * w + col + 1].type, // bottom-right
                quad_buf_[(row + 1) * w].type, // down
                quad_buf_[(row + 1) * w + col - 1].type, // bottom-left
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
}

void ImplicitPlot::dump()
{
    CImg<uint8_t> img(width_, height_, 1, 1, 0);
    memcpy(img.data(), img_buf_.data(), img_buf_.size());
    img.save("dump_plot.ppm");
}

void ImplicitPlot::precomputeMask()
{
    int size = (PADDING_PIXELS * 2 + 1) * RES;
    auto pixmap = windows::graphics::PaintSurfaceBitmapD2D::create(
        {DXGI_FORMAT_A8_UNORM, {(float)size, (float)size}, 1.0f});

    auto ctx = pixmap->beginPaint();
    ctx->clear(style::Color(0, 0, 0, 0));

    graph2d::PaintBrush brush;
    brush.setColor(style::Color(0xff, 0xff, 0xff, 0xff));
    auto p = createMaskPath(1, 1);
    ctx->drawPath(p.get(), brush);

    bool ok = pixmap->endPaint();

    if (auto data = pixmap->map()) {
        UINT buf_size;
        uint8_t* buf = nullptr;
        UINT stride;
        data->GetDataPointer(&buf_size, &buf);
        data->GetStride(&stride);
        stbi_write_png("dump_mask.png", size, size, 1, buf, stride);
    }
}

std::unique_ptr<graph2d::PaintPathInterface> ImplicitPlot::createMaskPath(uint8_t type, float stroke_width)
{
    float hp = 0.5f * float(RES);
    float d = stroke_width * float(RES);
    float r = 0.5f * d;
    float cx = 0.5f * float((PADDING_PIXELS * 2 + 1) * RES);
    const float rr = 0.5f * sqrtf(2.0f) * r;
    if (type == 1) {
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
    }
    return nullptr;
}
