#pragma once

#include "raygui.h"
#include "raygui-impl.h"

/**************************************************************************************************
* Global Variables */
extern const int gTextColor;
extern const int gBackgroundColor;
extern const int gFocusedColor;
extern const int gPressedColor;
extern const int gLineColor;

/**************************************************************************************************
* Functions
*/
void GuiSetStyleDefault();

void GuiSetStyleTextDefault();

void GuiSetStyleTextFocusable();

void GuiSetStyleButtonDefault();

void GuiSetStyleButtonAlwaysPressed();

void GuiSetStyleTextboxDefault();

void GuiSetStyleTextboxOutlined();

void GuiSetStyleListViewDefault();

void GuiSetStyleListViewOutlined();
