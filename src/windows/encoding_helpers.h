#pragma once
#include <stdint.h>

namespace windows {

uint32_t fetch_unicode_utf8(const uint8_t*& s);
int get_code_width_utf8(const uint8_t* s);
int string_length_utf8(const uint8_t* s);
int put_unicode(uint8_t* s, uint32_t code);
int put_unicode(uint16_t* s, uint32_t code);

bool is_surrogate(uint16_t ch);
bool is_high_surrogate(uint16_t ch);
bool is_low_surrogate(uint16_t ch);

}
