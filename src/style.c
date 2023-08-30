#include "style.h"

#if defined(LIGHT_THEME)

/* Light Theme */
const int gTextColor = 0x000000FF;
const int gBackgroundColor = 0xFFFFFFFF;
const int gFocusedColor = 0xccecffFF;
const int gPressedColor = 0x8bd1fcFF;
const int gLineColor = 0x555555FF;

#else

/* Dark Theme */
const int gTextColor = 0xFFFFFFFF;
const int gBackgroundColor = 0x282828FF;
const int gFocusedColor = 0x555555FF;
const int gPressedColor = 0x2B70C8FF;
const int gLineColor = 0x555555FF;

#endif

void GuiSetStyleDefault()
{
	GuiSetStyleTextboxDefault();
	GuiSetStyleButtonDefault();
	GuiSetStyleListViewDefault();
	GuiSetStyleTextDefault();

	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, gBackgroundColor);
	GuiSetStyle(DEFAULT, LINE_COLOR, gLineColor);
}

void GuiSetStyleTextDefault()
{
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, gTextColor);
	GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, gTextColor);
	GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, gTextColor);
	GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, gTextColor);
}

void GuiSetStyleTextFocusable()
{
	Color textColor = GetColor(gTextColor);
	Color secondaryTextColor = ColorBrightness(textColor, (((float)textColor.r + (float)textColor.g
		+ (float)textColor.b) / 3.0f > 128.0f) ? -0.4f : 0.4f);

	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, gTextColor);
	GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(secondaryTextColor));
	GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, gTextColor);
	GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, gTextColor);
}

void GuiSetStyleButtonDefault()
{
	GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, gBackgroundColor);
	GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, gFocusedColor);
	GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, gPressedColor);
	GuiSetStyle(BUTTON, BASE_COLOR_DISABLED, gBackgroundColor);

	GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, gBackgroundColor);
	GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, gFocusedColor);
	GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, gPressedColor);
	GuiSetStyle(BUTTON, BORDER_COLOR_DISABLED, gBackgroundColor);
}

void GuiSetStyleButtonAlwaysPressed()
{
	GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, gPressedColor);
	GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, gPressedColor);
	GuiSetStyle(BUTTON, BASE_COLOR_DISABLED, gPressedColor);
	GuiSetStyle(BUTTON, BASE_COLOR_DISABLED, gPressedColor);

	GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, gPressedColor);
	GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, gPressedColor);
	GuiSetStyle(BUTTON, BORDER_COLOR_DISABLED, gPressedColor);
	GuiSetStyle(BUTTON, BORDER_COLOR_DISABLED, gPressedColor);
}

void GuiSetStyleTextboxDefault()
{
	GuiSetStyle(TEXTBOX, BASE_COLOR_NORMAL, gBackgroundColor);
	GuiSetStyle(TEXTBOX, BASE_COLOR_FOCUSED, gBackgroundColor);
	GuiSetStyle(TEXTBOX, BASE_COLOR_PRESSED, gBackgroundColor);
	GuiSetStyle(TEXTBOX, BASE_COLOR_DISABLED, gBackgroundColor);

	GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL, gBackgroundColor);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, gBackgroundColor);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_PRESSED, gBackgroundColor);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_DISABLED, gBackgroundColor);
}

void GuiSetStyleTextboxOutlined()
{
	GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL, gLineColor);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, gLineColor);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_PRESSED, gLineColor);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_DISABLED, gLineColor);
}

void GuiSetStyleListViewDefault()
{
	GuiSetStyle(LISTVIEW, BASE_COLOR_NORMAL, gBackgroundColor);
	GuiSetStyle(LISTVIEW, BASE_COLOR_FOCUSED, gFocusedColor);
	GuiSetStyle(LISTVIEW, BASE_COLOR_PRESSED, gPressedColor);
	GuiSetStyle(LISTVIEW, BASE_COLOR_DISABLED, gBackgroundColor);

	GuiSetStyle(LISTVIEW, BORDER_COLOR_NORMAL, gBackgroundColor);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, gBackgroundColor);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, gBackgroundColor);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_DISABLED, gBackgroundColor);
}

void GuiSetStyleListViewOutlined()
{
	GuiSetStyle(LISTVIEW, BORDER_COLOR_NORMAL, gLineColor);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, gLineColor);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, gLineColor);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_DISABLED, gLineColor);
}
