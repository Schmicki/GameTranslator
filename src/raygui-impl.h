#pragma once

#include "raygui.h"

/*************************************************************************************************/

#define RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT 36

/*************************************************************************************************/

void GuiDrawCustomIcon(Texture2D icons, int iconSize, int index, int posX, int posY, float scale,
	Color tint);

// Return 0 - not pressed, 1 - left mouse, 2 - right mouse
int GuiButtonEx(Rectangle bounds, const char* text);

int GuiIconButtonEx(Rectangle bounds, const char* tooltip, Texture2D icons, int iconSize,
	int padding, int icon, float iconScale);

// Return 0 - not pressed, 1 - left mouse, 2 - right mouse
int GuiInvisibleButtonEx(Rectangle bounds);

/*************************************************************************************************/

Rectangle ClampBounds(Rectangle child, Rectangle parent);

float ScaleToFit(float childWidth, float childHeight, float parentWidth, float parentHeight);

int IntegerAlignUp(int val, int mod);

int IntegerAlignDown(int val, int mod);