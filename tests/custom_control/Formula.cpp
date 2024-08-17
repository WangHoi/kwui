#include "Formula.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

float cellnoise(float x)
{
    int32_t n = static_cast<int32_t>(floor(x)) | 0;
    n = (n << 13) ^ n;
    n &= 0xffffffff;
    int32_t m = n;
    n = n * 15731;
    n &= 0xffffffff;
    n = n * m;
    n &= 0xffffffff;
    n = n + 789221;
    n &= 0xffffffff;
    n = n * m;
    n &= 0xffffffff;
    n = n + 1376312589;
    n &= 0xffffffff;
    n = (n >> 14) & 65535;
    return n / 65535.0;
}

float voronoi(float x)
{
    int32_t i = static_cast<int32_t>(floor(x));
    float f = x - i;
    float x0 = cellnoise(i - 1);
    float d0 = std::abs(f - (-1 + x0));
    float x1 = cellnoise(i);
    float d1 = std::abs(f - x1);
    float x2 = cellnoise(i + 1);
    float d2 = std::abs(f - (1 + x2));
    float r = d0;
    r = d1 < r ? d1 : r;
    r = d2 < r ? d2 : r;
    return r;
}

float noise(float x)
{
    int32_t i = static_cast<int32_t>(floor(x)) | 0;
    float f = x - i;
    float w = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    float a = (2.0 * cellnoise(i + 0) - 1.0) * (f + 0.0);
    float b = (2.0 * cellnoise(i + 1) - 1.0) * (f - 1.0);
    return 2.0 * (a + (b - a) * w);
}

float sinc(float x)
{
    return (fabs(x) < 1e-3f) ? 1.0f : (sinf(x) / x);
}

float SimpleFormula::evaluate(float x, float t)
{
    return 30 * sin(x * 0.04) + 20 * sin(x * 0.1) + 5 * sin(x * 0.3);
}

std::unique_ptr<FormulaInterface> SimpleFormula::create()
{
    return std::make_unique<SimpleFormula>();
}

float SineWaveFormula::evaluate(float x, float t)
{
    return 2.0f * noise(3.0f * x + t) + 3.0f * sinc(x);
}

std::unique_ptr<FormulaInterface> SineWaveFormula::create()
{
    return std::make_unique<SineWaveFormula>();
}

float SquareWaveFormula::evaluate(float x, float t)
{
    return 2.0f + 2.0f * sin(floor(x + t) * 4321);
}

std::unique_ptr<FormulaInterface> SquareWaveFormula::create()
{
    return std::make_unique<SquareWaveFormula>();
}
