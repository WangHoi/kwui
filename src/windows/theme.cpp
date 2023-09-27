#include "theme.h"

namespace windows {
namespace theme {
	const graphics::Color BACKGROUND_COLOR = graphics::Color::FromString("#F8F8F8");
	const graphics::Color ACTION_COLOR = graphics::Color::FromString("#FF6500");
	const graphics::Color ACTION_HOVERED_COLOR = graphics::Color::FromString("#ED6207");
    const graphics::Color ACTION_DISABLED_COLOR = graphics::Color::FromString("#CCCCCC");
	const graphics::Color ACTION_TEXT_COLOR = graphics::Color::FromString("#F3F3F3");
	const graphics::Color H1_TEXT_COLOR = graphics::Color::FromString("#000000");
	const graphics::Color H2_TEXT_COLOR = graphics::Color::FromString("#333333");
	const graphics::Color H3_TEXT_COLOR = graphics::Color::FromString("#777777");
    const graphics::Color PROGRESS_TEXT_COLOR = graphics::Color::FromString("#909090");
	const float H1_FONT_SIZE = 44.0f;
	const float H2_FONT_SIZE = 24.0f;
	const float H3_FONT_SIZE = 14.0f;
    const float PROGRESS_FONT_SIZE = 40.0f;
	const graphics::Color BORDER_COLOR = graphics::Color::FromString("#E5E5E5");
	const graphics::Color CLOSE_BUTTON_HOVER_COLOR = graphics::Color::FromString("#F45454");

	const float DIALOG_BORDER_RADIUS = 4.0f;
	const float DIALOG_SHADOW_MARGIN_PIXELS = 24.0f;

	const float TITLE_BAR_HEIGHT = 32.0f;
	const float TITLE_BUTTON_WIDTH = 40.0f;

	const float MAIN_DIALOG_WIDTH = 552.0f;
	const float MAIN_DIALOG_HEIGHT = 408.0f;

	const std::string CLOSE_BUTTON_PNG = "close_button.png";
	const std::string CLOSE_BUTTON_HOVER_PNG = "close_button_hover.png";
	const std::string LOGO_PNG = "logo.png";
	const scene2d::DimensionF LOGO_SIZE = { 80, 80 };
	const scene2d::PointF LOGO_POSITION = { 236, 108 };
	const std::string DIALOG_SHADOW_PNG = "dialog-shadow.png";

	const float MAIN_LABEL_Y = 196;
	const scene2d::PointF MAIN_VERSION_POSITION = { 6, 198 };

	const float PRIMARY_BORDER_RADIUS = 8.0f;
	const float SECONDARY_BORDER_RADIUS = 4.0f;
	const std::string EXPAND_PNG = "expand.png";
	const std::string COLLAPSE_PNG = "collapse.png";
	const scene2d::DimensionF EXPAND_PNG_SIZE = { 24, 24 };
	const scene2d::PointF EXPAND_PNG_POSITION = { 106, 12 };
	const float EXPAND_DIALOG_HEIGHT = 480;

	const graphics::Color TEXT_EDIT_BACKGROUND_COLOR = graphics::Color::FromString("#ffffff");
	const graphics::Color TEXT_EDIT_SELECTION_COLOR = graphics::Color::FromString("#FF6500").MakeAlpha(0.25);
	const graphics::Color TEXT_EDIT_CARET_COLOR = graphics::Color::FromString("#000000");
	const scene2d::DimensionF TEXT_EDIT_SIZE = { 442, 32 };
	const scene2d::PointF TEXT_EDIT_POSITION = { 16, 396 };
	const float TEXT_EDIT_BORDER_RADIUS = 4.0f;
	const float TEXT_EDIT_PADDING = 7.0f;

	const float CONFIRM_DIALOG_WIDTH = 360;
	const float CONFIRM_DIALOG_HEIGHT = 220;
}
}
