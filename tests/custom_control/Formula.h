#pragma once

#include <memory>

class FormulaInterface
{
public:
    virtual float evaluate(float x, float t) = 0;
};

class SimpleFormula : public FormulaInterface
{
public:
    float evaluate(float x, float t) override;

    static std::unique_ptr<FormulaInterface> create();
};

class SineWaveFormula : public FormulaInterface
{
public:
    float evaluate(float x, float t) override;

    static std::unique_ptr<FormulaInterface> create();
};

class SquareWaveFormula : public FormulaInterface
{
public:
    float evaluate(float x, float t) override;

    static std::unique_ptr<FormulaInterface> create();
};

