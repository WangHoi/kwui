#include "Color.h"

namespace windows {
namespace graphics {

Color Color::FromString(std::string_view str) {
    if (str.empty()) return Color();
    if (str[0] == '#') {
        uint32_t c = (uint32_t)strtoul(&str[1], nullptr, 16);
        if (str.length() == 4) {
            int r = (c & 0xf00) >> 8;
            int g = (c & 0xf0) >> 4;
            int b = (c & 0xf);
            return Color((r << 4) | r, (g << 4) | g, (b << 4) | b);
        } if (str.length() == 7) {
            return Color((int)((c & 0xff0000) >> 16), (int)((c & 0xff00) >> 8), (int)(c & 0xff));
        } else if (str.length() == 9) {
            return Color((int)((c & 0xff000000) >> 24), (int)((c & 0xff0000) >> 16), (int)((c & 0xff00) >> 8), (int)(c & 0xff));
        }
    }
    return Color();
}

uint32_t Color::GetGdiRgb() const {
    return (uint32_t(_c[0] * 255.0f) & 0xff)
        | ((uint32_t(_c[1] * 255.0f) & 0xff) << 8)
        | ((uint32_t(_c[2] * 255.0f) & 0xff) << 16);
}

}
}
