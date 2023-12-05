#pragma once
#include "style/StyleColor.h"
#include <string>
#include "scene2d/geom_types.h"

namespace windows {
namespace theme {
	extern const style::Color BACKGROUND_COLOR;
	extern const style::Color ACTION_COLOR;
	extern const style::Color ACTION_HOVERED_COLOR;
    extern const style::Color ACTION_DISABLED_COLOR;
	extern const style::Color ACTION_TEXT_COLOR;
	extern const style::Color H1_TEXT_COLOR;
	extern const style::Color H2_TEXT_COLOR;
	extern const style::Color H3_TEXT_COLOR;
    extern const style::Color PROGRESS_TEXT_COLOR;
	extern const float H1_FONT_SIZE;
	extern const float H2_FONT_SIZE;
	extern const float H3_FONT_SIZE;
    extern const float PROGRESS_FONT_SIZE;
	extern const style::Color BORDER_COLOR;
	extern const style::Color CLOSE_BUTTON_HOVER_COLOR;
	
	extern const float DIALOG_BORDER_RADIUS;
	extern const float DIALOG_SHADOW_MARGIN_PIXELS;

	extern const float TITLE_BAR_HEIGHT;
	extern const float TITLE_BUTTON_WIDTH;
	extern const float MAIN_DIALOG_WIDTH;
	extern const float MAIN_DIALOG_HEIGHT;

	extern const std::string CLOSE_BUTTON_PNG;
	extern const std::string CLOSE_BUTTON_HOVER_PNG;
	extern const std::string LOGO_PNG;
	extern const scene2d::DimensionF LOGO_SIZE;
	extern const scene2d::PointF LOGO_POSITION;
	extern const std::string DIALOG_SHADOW_PNG;

	extern const float MAIN_LABEL_Y;
	extern const scene2d::PointF MAIN_VERSION_POSITION;
	
	extern const float PRIMARY_BORDER_RADIUS;
	extern const float SECONDARY_BORDER_RADIUS;
	extern const std::string EXPAND_PNG;
	extern const std::string COLLAPSE_PNG;
	extern const scene2d::DimensionF EXPAND_PNG_SIZE;
	extern const scene2d::PointF EXPAND_PNG_POSITION;
	extern const float EXPAND_DIALOG_HEIGHT;

	extern const style::Color TEXT_EDIT_BACKGROUND_COLOR;
	extern const style::Color TEXT_EDIT_SELECTION_COLOR;
	extern const style::Color TEXT_EDIT_CARET_COLOR;
	extern const scene2d::DimensionF TEXT_EDIT_SIZE;
	extern const scene2d::PointF TEXT_EDIT_POSITION;
	extern const float TEXT_EDIT_BORDER_RADIUS;
	extern const float TEXT_EDIT_PADDING;

	extern const float CONFIRM_DIALOG_WIDTH;
	extern const float CONFIRM_DIALOG_HEIGHT;
}
}
