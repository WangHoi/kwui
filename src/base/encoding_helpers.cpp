#include "encoding_helpers.h"

namespace windows
{

static inline uint32_t low_pass(uint32_t x, uint32_t bit) {
    return x & ((1u << bit) - 1u);
}

static inline uint32_t bit_mask(uint32_t bit) {
    return 1u << bit;
}

static inline bool test_bit(uint32_t x, uint32_t bit) {
    return (x & bit_mask(bit)) != 0;
}

uint32_t fetch_unicode_utf8(const uint8_t*& s) {
    uint8_t c0 = *s;
    if (c0 == 0) return 0;
    s++;
    if (!test_bit(c0, 7)) return c0;

    uint8_t c1 = *s;
    if (c1 == 0) return 0;
    s++;
    if (!test_bit(c0, 5)) return low_pass(c1, 6) + (low_pass(c0, 5) << 6);

    uint8_t c2 = *s;
    if (c2 == 0) return 0;
    s++;
    if (!test_bit(c0, 4)) return low_pass(c2, 6) + (low_pass(c1, 6) << 6) + (low_pass(c0, 4) << 12);

    uint8_t c3 = *s;
    if (c3 == 0) return 0;
    s++;
    return low_pass(c3, 6) + (low_pass(c2, 6) << 6) + (low_pass(c1, 6) << 12) + (low_pass(c0, 3) << 18);
}

int get_code_width_utf8(const uint8_t* s) {
    char c0 = *s;
    if (c0 == 0) return 0;
    if (!test_bit(c0, 7)) return 1;
    if (s[1] == 0) return 0;
    if (!test_bit(c0, 5)) return 2;
    if (s[2] == 0) return 0;
    if (!test_bit(c0, 4)) return 3;
    if (s[3] == 0) return 0;
    return 4;
}

int string_length_utf8(const uint8_t* s) {
    int width, length = 0;
    while ((width = get_code_width_utf8(s))) {
        s += width;
        length++;
    }
    return length;
}

extern int put_unicode(uint8_t* s, uint32_t code) {
    if (code <= 0x7f) {
        s[0] = uint8_t(code);
        return 1;
    } else if (code <= 0x7ff) {
        s[0] = (0xC0 | (code >> 6));
        s[1] = (0x80 | (code & 0x3F));
        return 2;
    } else if (code <= 0xffff) {
        s[0] = (0xE0 | (code >> 12));
        s[1] = (0x80 | ((code >> 6) & 0x3F));
        s[2] = (0x80 | (code & 0x3F));
        return 3;
    } else if (code <= 0x1fffff) {
        s[0] = (0xF0 | (code >> 18));
        s[1] = (0x80 | ((code >> 12) & 0x3F));
        s[2] = (0x80 | ((code >> 6) & 0x3F));
        s[3] = (0x80 | (code & 0x3F));
        return 4;
    } else if (code <= 0x3ffffff) {
        s[0] = (0xF8 | (code >> 24));
        s[1] = (0x80 | ((code >> 18) & 0x3F));
        s[2] = (0x80 | ((code >> 12) & 0x3F));
        s[3] = (0x80 | ((code >> 6) & 0x3F));
        s[4] = (0x80 | (code & 0x3F));
        return 5;
    } else if (code <= 0x7fffffff) {
        s[0] = (0xFC | (code >> 30));
        s[1] = (0x80 | ((code >> 24) & 0x3F));
        s[2] = (0x80 | ((code >> 18) & 0x3F));
        s[3] = (0x80 | ((code >> 12) & 0x3F));
        s[4] = (0x80 | ((code >> 6) & 0x3F));
        s[5] = (0x80 | (code & 0x3F));
        return 6;
    } else return 0;
}
int put_unicode(uint16_t* s, uint32_t code) {
    if (code <= 0xffff) {
        s[0] = uint16_t(code & 0xffff);
        return 1;
    } else if (code <= 0x10ffff) {
        code -= 0x10000;
        s[0] = uint16_t(0xd800 | (code >> 10));
        s[1] = uint16_t(0xdc00 | (code & 0x03FF));
        return 2;
    } else return 0;
}

bool is_surrogate(uint16_t ch) {
    // 0xD800 <= ch <= 0xDFFF
    return (ch & 0xF800) == 0xD800;
}
bool is_high_surrogate(uint16_t ch) {
    // 0xD800 <= ch <= 0xDBFF
    return (ch & 0xFC00) == 0xD800;
}
bool is_low_surrogate(uint16_t ch) {
    // 0xDC00 <= ch <= 0xDFFF
    return (ch & 0xFC00) == 0xDC00;
}

} // namespace windows

