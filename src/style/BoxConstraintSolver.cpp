#include "BoxConstraintSolver.h"
#include "base/log.h"
#include <utility>

namespace style {

StaticBlockWidthSolver::StaticBlockWidthSolver(float cont_block_width, absl::optional<float> margin_left, absl::optional<float> width, absl::optional<float> margin_right)
	: cont_block_width_(cont_block_width)
	, margin_left_(margin_left)
	, width_(width)
	, margin_right_(margin_right)
{
}
float StaticBlockWidthSolver::containingBlockWidth() const
{
	return cont_block_width_;
}
float StaticBlockWidthSolver::measureWidth()
{
	if (width_.has_value())
		return std::max(*width_, 0.0f);
	
	float w = cont_block_width_
		- margin_left_.value_or(0)
		- margin_right_.value_or(0);
	return std::max(w, 0.0f);
}
void StaticBlockWidthSolver::setLayoutWidth(float layout_width)
{
	if (!width_.has_value())
		width_ = layout_width;
	
	if (!margin_left_.has_value() && !margin_right_.has_value()) {
		float remain = cont_block_width_ - *width_;
		if (remain >= 0) {
			margin_left_ = margin_right_ = 0.5f * remain;
		} else {
			margin_left_ = 0.0f;
			margin_right_ = cont_block_width_ - *width_;
		}
	} else if (!margin_left_.has_value()) {
		margin_left_ = cont_block_width_ - *width_ - margin_right_.value_or(0);
	} else if (!margin_right_.has_value()) {
		margin_right_ = cont_block_width_ - margin_left_.value_or(0) - *width_;
	} else {
		// may over-constrained
		margin_right_ = cont_block_width_ - margin_left_.value_or(0) - *width_;
	}
}
float StaticBlockWidthSolver::marginLeft()
{
	return margin_left_.value_or(0);
}
float StaticBlockWidthSolver::width()
{
	return width_.value_or(0);
}
float StaticBlockWidthSolver::marginRight()
{
	return margin_right_.value_or(0);
}
AbsoluteBlockWidthSolver::AbsoluteBlockWidthSolver(float cont_block_width, absl::optional<float> left, absl::optional<float> margin_left, absl::optional<float> width, absl::optional<float> margin_right, absl::optional<float> right)
	: contg_block_width_(cont_block_width)
	, left_(left)
	, margin_left_(margin_left)
	, width_(width)
	, margin_right_(margin_right)
	, right_(right)
{
}
float AbsoluteBlockWidthSolver::containingBlockWidth() const
{
	return contg_block_width_;
}
WidthConstraint AbsoluteBlockWidthSolver::measureWidth()
{
	WidthConstraint c;
	if (width_.has_value()) {
		c.type = WidthConstraintType::Fixed;
		c.value = std::max(0.0f, *width_);
	} else {
		if (left_.has_value() && right_.has_value()) {
			c.type = WidthConstraintType::Fixed;
			float w = contg_block_width_
				- left_.value_or(0)
				- margin_left_.value_or(0)
				- margin_right_.value_or(0)
				- right_.value_or(0);
			if (w < 0) {
				// over-constrained, ignore right value
				w += right_.value_or(0);
			}
			c.value = std::max(0.0f, w);
		} else {
			c.type = WidthConstraintType::FitContent;
			float w = contg_block_width_
				- left_.value_or(0)
				- margin_left_.value_or(0)
				- margin_right_.value_or(0)
				- right_.value_or(0);
			c.value = std::max(0.0f, w);
		}
	}
	return c;
}
void AbsoluteBlockWidthSolver::setLayoutWidth(float layout_width)
{
	bool skip1_6 = false;
	bool skip1_2 = false;
	if (!left_.has_value() && !width_.has_value() && !right_.has_value()) {
		if (!margin_left_.has_value())
			margin_left_ = 0.0f;
		if (!margin_right_.has_value())
			margin_right_ = 0.0f;
		left_ = 0.0f; // static position left
		skip1_2 = true;
	} else if (left_.has_value() && width_.has_value() && right_.has_value()) {
		float w = width_.value_or(layout_width);
		if (!margin_left_.has_value() && !margin_right_.has_value()) {
			float remain = contg_block_width_ - *left_ - w - *right_;
			if (remain >= 0) {
				margin_left_ = *margin_right_ = 0.5f * remain;
			} else {
				margin_left_ = 0.0f;
				margin_right_ = contg_block_width_ - *left_ - w - *right_;
			}
		} else if (!margin_left_.has_value()) {
			margin_left_ = contg_block_width_ - *left_ - w - margin_right_.value_or(0) - *right_;
		} else if (!margin_right_.has_value()) {
			margin_right_ = contg_block_width_ - *left_ - margin_left_.value_or(0) - w - *right_;
		} else {
			// may over-constrained
			right_ = contg_block_width_
				- *left_
				- *margin_left_
				- w
				- *margin_right_;
		}
		skip1_6 = true;
	} else {
		if (!margin_left_.has_value())
			margin_left_ = 0.0f;
		if (!margin_right_.has_value())
			margin_right_ = 0.0f;
	}

	CHECK(margin_left_.has_value()) << "margin_left must be set.";
	CHECK(margin_right_.has_value()) << "margin_right must be set.";

	if (!skip1_6) {
		if (!skip1_2) {
			/* 1. 'left' and 'width' are 'auto' and 'right' is not 'auto',
			 * then the width is shrink-to-fit. Then solve for 'left' */
			if (!left_.has_value() && !width_.has_value() && right_.has_value()) {
				width_ = layout_width;
				left_ = contg_block_width_
					- *margin_left_
					- *width_
					- *margin_right_
					- *right_;
			}
			/* 2.'left' and 'right' are 'auto' and 'width' is not 'auto',
			 * then if the 'direction' property of the element establishing
			 * the static-position containing block is 'ltr' set 'left' to
			 * the static position, otherwise set 'right' to the static
			 * position. Then solve for 'left' (if 'direction is 'rtl') or
			 * 'right' (if 'direction' is 'ltr'). */
			if (!left_.has_value() && !right_.has_value() && width_.has_value()) {
				left_ = 0.0f; // static-position left
				right_ = contg_block_width_
					- *left_
					- *margin_left_
					- *width_
					- *margin_right_;
			}
		}
		/* 3. 'width' and 'right' are 'auto' and 'left' is not 'auto',
		 * then the width is shrink-to-fit . Then solve for 'right'*/
		if (!width_.has_value() && !right_.has_value() && left_.has_value()) {
			width_ = layout_width;
			right_ = contg_block_width_
				- *left_
				- *margin_left_
				- *width_
				- *margin_right_;
		}
		/* 4. 'left' is 'auto', 'width' and 'right' are not 'auto', then solve for 'left' */
		if (!left_.has_value() && width_.has_value() && right_.has_value()) {
			left_ = contg_block_width_
				- *margin_left_
				- *width_
				- *margin_right_
				- *right_;
		}
		/* 5. 'width' is 'auto', 'left' and 'right' are not 'auto', then solve for 'width' */
		if (!width_.has_value() && left_.has_value() && right_.has_value()) {
			width_ = contg_block_width_
				- *left_
				- *margin_left_
				- *margin_right_
				- *right_;
		}
		/* 6. 'right' is 'auto', 'left' and 'width' are not 'auto', then solve for 'right' */
		if (!right_.has_value() && left_.has_value() && width_.has_value()) {
			right_ = contg_block_width_
				- *left_
				- *margin_left_
				- *width_
				- *margin_right_;
		}
	}
}
float AbsoluteBlockWidthSolver::left()
{
	return left_.value_or(0);
}
float AbsoluteBlockWidthSolver::marginLeft()
{
	return margin_left_.value_or(0);
}
float AbsoluteBlockWidthSolver::width()
{
	return width_.value_or(0);
}
float AbsoluteBlockWidthSolver::marginRight()
{
	return margin_right_.value_or(0);
}
float AbsoluteBlockWidthSolver::right()
{
	return right_.value_or(0);
}

float AbsoluteBlockWidthSolver::availWidth()
{
	if (width_.has_value())
		return std::max(*width_, 0.0f);

	float w;
	if (left_.has_value() && right_.has_value()) {
		w = contg_block_width_
			- left_.value_or(0)
			- margin_left_.value_or(0)
			- margin_right_.value_or(0)
			- right_.value_or(0);
		if (w < 0) {
			w = contg_block_width_
				- margin_left_.value_or(0)
				- margin_right_.value_or(0);
		}
	} else if (left_.has_value()) {
		w = contg_block_width_
			- *left_
			- margin_left_.value_or(0)
			- margin_right_.value_or(0);
	} else if (right_.has_value()) {
		w = contg_block_width_
			- margin_left_.value_or(0)
			- margin_right_.value_or(0)
			- *right_;
	} else {
		w = contg_block_width_
			- margin_left_.value_or(0)
			- margin_right_.value_or(0);
	}
	return std::max(w, 0.0f);
}

AbsoluteBlockPositionSolver::AbsoluteBlockPositionSolver(float cont_block_height,
	absl::optional<float> margin_top, absl::optional<float> height, absl::optional<float> margin_bottom)
	: cont_block_height_(cont_block_height)
	, top_(margin_top)
	, height_(height)
	, bottom_(margin_bottom)
{}
void AbsoluteBlockPositionSolver::setLayoutHeight(float layout_height)
{
	if (!height_.has_value())
		height_ = layout_height;

	if (!top_.has_value() && !bottom_.has_value()) {
		float remain = cont_block_height_ - *height_;
		if (remain >= 0) {
			top_ = bottom_ = 0.5f * remain;
		} else {
			top_ = 0.0f;
			bottom_ = cont_block_height_ - *height_;
		}
	} else if (!top_.has_value()) {
		top_ = cont_block_height_ - *height_ - bottom_.value_or(0);
	} else if (!bottom_.has_value()) {
		bottom_ = cont_block_height_ - top_.value_or(0) - *height_;
	} else {
		// may over-constrained
		bottom_ = cont_block_height_ - top_.value_or(0) - *height_;
	}
}
float AbsoluteBlockPositionSolver::top()
{
	return top_.value_or(0);
}
float AbsoluteBlockPositionSolver::height()
{
	return height_.value_or(0);
}
float AbsoluteBlockPositionSolver::bottom()
{
	return bottom_.value_or(0);
}

}