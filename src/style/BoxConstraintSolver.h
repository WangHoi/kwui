#pragma once
#include "absl/types/optional.h"
#include "absl/types/variant.h"

namespace style {

enum class WidthConstraintType {
    MinContent,
    MaxContent,
    FitContent,
    Fixed,
};
struct WidthConstraint {
    WidthConstraintType type = WidthConstraintType::Fixed;
    float value;
};

class StaticBlockWidthSolver {
public:
    StaticBlockWidthSolver(float cont_block_width,
        absl::optional<float> margin_left,
        absl::optional<float> width,
        absl::optional<float> margin_right);

    bool isStatic() const
    {
        return true;
    }
    float containingBlockWidth() const;
    float measureWidth();
    void setLayoutWidth(float layout_width);

    float marginLeft();
    float width();
    float marginRight();

private:
    float cont_block_width_;
    absl::optional<float> margin_left_;
    absl::optional<float> width_;
    absl::optional<float> margin_right_;
};

class AbsoluteBlockWidthSolver {
public:
    AbsoluteBlockWidthSolver(float cont_block_width,
        absl::optional<float> left,
        absl::optional<float> margin_left,
        absl::optional<float> width,
        absl::optional<float> margin_right,
        absl::optional<float> right);

    bool isStatic() const
    {
        return false;
    }
    float containingBlockWidth() const;
    WidthConstraint measureWidth();
    void setLayoutWidth(float layout_width);

    float left();
    float marginLeft();
    float width();
    float marginRight();
    float right();

private:
    float availWidth();

    float contg_block_width_;
    absl::optional<float> left_;
    absl::optional<float> margin_left_;
    absl::optional<float> width_;
    absl::optional<float> margin_right_;
    absl::optional<float> right_;
};

class AbsoluteBlockPositionSolver {
public:
    AbsoluteBlockPositionSolver(float cont_block_height, absl::optional<float> top,
        absl::optional<float> height, absl::optional<float> bottom);

    void setLayoutHeight(float layout_height);

    float top();
    float height();
    float bottom();

private:
    float cont_block_height_;
    absl::optional<float> top_;
    absl::optional<float> height_;
    absl::optional<float> bottom_;
};

}