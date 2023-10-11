#include "Layout.h"
#include "StyleValue.h"
#include "scene2d/Node.h"
#include <utility>

namespace style {

void try_convert_to_px(style::Value& v, float percent_base)
{
    v = style::Value::fromPixel(try_resolve_to_px(v, percent_base).value_or(0));
}

absl::optional<float> try_resolve_to_px(const style::Value& v, float percent_base)
{
    if (v.unit == style::ValueUnit::Percent) {
        return v.f32_val / 100.0f * percent_base;
    } else if (v.unit == style::ValueUnit::Pixel) {
        return v.f32_val;
    } else if (v.unit == style::ValueUnit::Raw) {
        return v.f32_val;
    }
    return absl::nullopt;
}

absl::optional<float> try_resolve_to_px(const style::Value& v, absl::optional<float> percent_base)
{
    if (v.unit == style::ValueUnit::Percent) {
        return percent_base.has_value()
            ? absl::optional<float>(v.f32_val / 100.0f * percent_base.value())
            : absl::nullopt;
    } else if (v.unit == style::ValueUnit::Pixel) {
        return v.f32_val;
    } else if (v.unit == style::ValueUnit::Raw) {
        return v.f32_val;
    }
    return absl::nullopt;
}

float collapse_margin(float m1, float m2)
{
    if (m1 >= 0 && m2 >= 0)
        return std::max(m1, m2);
    else if (m1 < 0 && m2 < 0)
        return std::min(m1, m2);
    else
        return m1 + m2;
}

}
