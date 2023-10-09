#pragma once
#include "absl/types/optional.h"

namespace style {

class BlockWidthSolverInterface {
public:
    bool isAbsolute() const
    {
        return !isStatic();
    }
    virtual bool isStatic() const = 0;
    virtual float containingBlockWidth() const = 0;
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

    bool isStatic() const override
    {
        return true;
    }
    float containingBlockWidth() const override;
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

    bool isStatic() const override
    {
        return false;
    }
    float containingBlockWidth() const override;
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

class StaticBlockHeightSolver {
public:
    StaticBlockHeightSolver(float cont_block_height,
        absl::optional<float> margin_top,
        absl::optional<float> height,
        absl::optional<float> margin_bottom);

    float containingBlockHeight() const;
    
    void setLayoutHeight(float layout_height);

    float marginTop();
    float height();
    float marginBottom();

private:
    float cont_block_height_;
    absl::optional<float> margin_top_;
    absl::optional<float> height_;
    absl::optional<float> margin_bottom_;
};

}