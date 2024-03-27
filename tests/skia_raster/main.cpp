#include "include/c/sk_canvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkFont.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "tgaimage.h"

std::vector<uint8_t> raster_direct(int width, int height,
                                void (*draw)(SkCanvas*)) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    size_t rowBytes = info.minRowBytes();
    size_t size = info.computeByteSize(rowBytes);
    std::vector<uint8_t> pixelMemory(size);  // allocate memory
    sk_sp<SkSurface> surface =
            SkSurface::MakeRasterDirect(
                    info, &pixelMemory[0], rowBytes);
    SkCanvas* canvas = surface->getCanvas();
    draw(canvas);
    return pixelMemory;
}

int main()
{
    auto pixels = raster_direct(100, 200, [](SkCanvas* canvas) {
        canvas->clear(SK_ColorCYAN);
        SkPaint brush;
        brush.setColor(SK_ColorWHITE);
        canvas->drawCircle(50, 100, 36, brush);
        brush.setColor(SK_ColorBLACK);
        SkFont fnt(SkTypeface::MakeFromName("Microsoft YaHei", SkFontStyle()));
        fnt.setSize(32);
        canvas->drawString(u8"中国", 20, 100, fnt, brush);
    });
    TGAImage tga(100, 200, 4);
    tga.setData(pixels);
    tga.write_tga_file("test.tga", false, false);
}