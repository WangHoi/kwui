#include "theme.h"

namespace windows {
namespace theme {
	const style::Color BACKGROUND_COLOR = style::Color::fromString("#F8F8F8");
	const style::Color ACTION_COLOR = style::Color::fromString("#FF6500");
	const style::Color ACTION_HOVERED_COLOR = style::Color::fromString("#ED6207");
    const style::Color ACTION_DISABLED_COLOR = style::Color::fromString("#CCCCCC");
	const style::Color ACTION_TEXT_COLOR = style::Color::fromString("#F3F3F3");
	const style::Color H1_TEXT_COLOR = style::Color::fromString("#000000");
	const style::Color H2_TEXT_COLOR = style::Color::fromString("#333333");
	const style::Color H3_TEXT_COLOR = style::Color::fromString("#777777");
    const style::Color PROGRESS_TEXT_COLOR = style::Color::fromString("#909090");
	const float H1_FONT_SIZE = 44.0f;
	const float H2_FONT_SIZE = 24.0f;
	const float H3_FONT_SIZE = 14.0f;
    const float PROGRESS_FONT_SIZE = 40.0f;
	const style::Color BORDER_COLOR = style::Color::fromString("#E5E5E5");
	const style::Color CLOSE_BUTTON_HOVER_COLOR = style::Color::fromString("#F45454");

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

	const style::Color TEXT_EDIT_BACKGROUND_COLOR = style::Color::fromString("#ffffff");
	const style::Color TEXT_EDIT_SELECTION_COLOR = style::Color::fromString("#FF6500").makeAlpha(0.25);
	const style::Color TEXT_EDIT_CARET_COLOR = style::Color::fromString("#000000");
	const scene2d::DimensionF TEXT_EDIT_SIZE = { 442, 32 };
	const scene2d::PointF TEXT_EDIT_POSITION = { 16, 396 };
	const float TEXT_EDIT_BORDER_RADIUS = 4.0f;
	const float TEXT_EDIT_PADDING = 7.0f;

	const float CONFIRM_DIALOG_WIDTH = 360;
	const float CONFIRM_DIALOG_HEIGHT = 220;
}
}
