#include "platform/filesystem.h"
#include "filter-overlay.h"

/*************************************************************************************************/

typedef struct FilterOverlayState
{
	unsigned int unused;
} FilterOverlayState;

/*************************************************************************************************/

void CloseFilterOverlay(FileManagerState* state, FilterOverlayState* filter)
{
	free(filter);

	state->overlay.Draw = NULL;
	state->overlay.data = NULL;
}

int GuiFilterOverlay(FileManagerState* state, FilterOverlayState* trim, Rectangle bounds)
{
	const int textSize = (int)(16.0f * gScale);
	const int sliderWidth = 200;
	const int sliderHeight = textSize + gPadding * 2;
	const int windowWidth = sliderWidth + gPadding * 2;
	const int windowHeight = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT + sliderHeight * 3 + gPadding * 4;

	Rectangle window = {
		bounds.x = bounds.width / 2 - (float)windowWidth / 2,
		bounds.y = bounds.height / 2 - (float)windowHeight / 2,
		bounds.width = (float)windowWidth,
		bounds.height = (float)windowHeight
	};

	Rectangle inputBox = {
		window.x + gPadding,
		window.y + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT + gPadding,
		(float)sliderWidth,
		(float)sliderHeight
	};

	if (GuiWindowBox(window, "Trimming"))
	{
		CloseFilterOverlay(state, trim);
		return 1;
	}

	GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xFFFFFFFF);
	GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xEEEEEEEE);
	GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0xEEEEEEEE);
	GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xEEEEEEEE);
	GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0xEEEEEEEE);
	GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0x00000000);

	if (GuiButtonEx(inputBox, "Trim"))
	{
		CloseFilterOverlay(state, trim);
		return 1;
	}

	if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_CONTROL))
	{
		CloseFilterOverlay(state, trim);
		return 1;
	}

	return 0;
}

Overlay OpenFilterOverlay(FileManagerState* fileManager)
{
	FilterOverlayState* state;
	Overlay overlay = { NULL, NULL };

	if ((state = malloc(sizeof(FilterOverlayState))) == NULL)
		return overlay;

	overlay.Draw = &GuiFilterOverlay;
	overlay.data = state;
	return overlay;
}