#pragma once
#include "absl/types/optional.h"

namespace scene2d {

class BlockWidthSolverInterface {
public:
    virtual float measureWidth() = 0;
    virtual void setLayoutWidth(float layout_width) = 0;

    virtual float marginLeft() = 0;
    virtual float width() = 0;
    virtual float marginRight() = 0;

    virtual absl::optional<float> left() = 0;
    virtual absl::optional<float> right() = 0;
};

class StaticBlockWidthSolver : public BlockWidthSolverInterface {
public:
    StaticBlockWidthSolver(float cont_block_width,
        absl::optional<float> margin_left,
        absl::optional<float> width,
        absl::optional<float> margin_right);

    float measureWidth() override;
    void setLayoutWidth(float layout_width) override;

    float marginLeft() override;
    float width() override;
    float marginRight() override;

    absl::optional<float> left() override
    {
        return absl::nullopt;
    }
    absl::optional<float> right() override
    {
        return absl::nullopt;
    }

private:
    float cont_block_width_;
    absl::optional<float> margin_left_;
    absl::optional<float> width_;
    absl::optional<float> margin_right_;
};

class AbsoluteBlockWidthSolver : public BlockWidthSolverInterface {
public:
    AbsoluteBlockWidthSolver(float cont_block_width,
        absl::optional<float> left,
        absl::optional<float> margin_left,
        absl::optional<float> width,
        absl::optional<float> margin_right,
        absl::optional<float> right);

    float measureWidth() override;
    void setLayoutWidth(float layout_width) override;

    absl::optional<float> left() override;
    float marginLeft() override;
    float width() override;
    float marginRight() override;
    absl::optional<float> right() override;

private:
    float cont_block_width_;
    absl::optional<float> left_;
    absl::optional<float> margin_left_;
    absl::optional<float> width_;
    absl::optional<float> margin_right_;
    absl::optional<float> right_;
};

}