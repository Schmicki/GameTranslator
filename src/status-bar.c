#include "status-bar.h"
#include "tools/explorer/explorer.h"

/*************************************************************************************************/

static int windowX;
static int windowY;
static int mouseOffsetX;
static int mouseOffsetY;
static int mouseX;
static int mouseY;

/*************************************************************************************************/

static int GuiActivityTab(Rectangle bounds, const char* text, int index)
{
	const int iconSize = (int)(gScale * gIconSize);
	const int buttonSize = iconSize + gPadding * 2;
	Rectangle item;

	if (index == gCurrentActivity)
	{
		GuiSetStyleButtonAlwaysPressed();
	}
	else
	{
		GuiSetStyleButtonDefault();
	}

	/*
	* Button
	*/

	item.x = bounds.x;
	item.y = bounds.y;
	item.width = (float)((int)bounds.width - buttonSize - gPadding);
	item.height = bounds.height;

	GuiSetTooltip(NULL);
	if (GuiButtonEx(item, NULL))
	{
		gCurrentActivity = index;
	}

	/*
	* Label
	*/

	item.x = (float)((int)item.x + gPadding);
	item.y = (float)((int)item.y + gPadding);
	item.width = (float)((int)item.width - gPadding * 2);
	item.height = (float)((int)item.height - gPadding * 2);

	GuiLabel(item, text);

	/*
	* Close Button
	*/

	item.x = (float)((int)item.x + (int)item.width + gPadding);
	item.y = bounds.y;
	item.width = (float)buttonSize;
	item.height = (float)buttonSize;

	if (GuiIconButtonEx(item, "Close Tab", gIcons, gIconSize, gPadding, ICON_CUSTOM_CLOSE, gScale))
	{
		return 1;
	}

	return 0;
}

static void GuiActivityTabs(Rectangle bounds, int* scroll, int* rest)
{
	const int iconSize = (int)(gScale * gIconSize);
	const int buttonSize = iconSize + gPadding * 2;

	const int minTabWith = buttonSize * 3;
	const int maxTabWith = buttonSize * 10;

	const int tabWidth = (int)((float)((int)bounds.width - (gActivityCount - 1) * 1) / (float)gActivityCount);
	const int fixedTabWidth = (int)Clamp((float)tabWidth, (float)minTabWith, (float)maxTabWith);

	int tmp = 0;

	Rectangle item;

	*rest = 0;

	GuiEnableTooltip();

	if (tabWidth < minTabWith && gActivityCount > 0)
	{
		/*
		* Scroll Buttons
		*/

		/* Remove buttons from width */
		bounds.width = (float)((int)bounds.width - buttonSize * 2 - gPadding * 2);

		/*
		* Previous Button
		*/

		item.x = bounds.x;
		item.y = bounds.y;
		item.width = (float)buttonSize;
		item.height = (float)buttonSize;

		if (GuiIconButtonEx(item, "Previous", gIcons, gIconSize, gPadding,
			ICON_CUSTOM_ARROW_HEAD_LEFT, gScale))
		{
			*scroll += (int)(fixedTabWidth * 1.5f);
		}

		/*
		* Next Button
		*/

		item.x = (float)((int)item.x + buttonSize + gPadding * 2 + (int)bounds.width);

		if (GuiIconButtonEx(item, "Next", gIcons, gIconSize, gPadding,
			ICON_CUSTOM_ARROW_HEAD_RIGHT, gScale))
		{
			*scroll -= (int)(fixedTabWidth * 1.5f);
		}

		/*
		* Clamp Scroll Value
		*/

		if (*scroll > 0)
			*scroll = 0;

		tmp = gActivityCount * (fixedTabWidth + 1) - (int)bounds.width;
		if (tmp < -*scroll)
			*scroll = -tmp;

		bounds.x = (float)((int)item.x - (int)bounds.width - gPadding);
	}
	else
	{
		*scroll = 0;
		*rest = (int)bounds.width - (fixedTabWidth * gActivityCount + 1 * (gActivityCount - 1));
	}

	GuiDisableTooltip();

	tmp = 0;

	/* Lock Gui if mouse is not hovering it. */
	if (!GuiIsLocked() && !CheckCollisionPointRec(GetMousePosition(), bounds))
	{
		tmp = 1;
		GuiLock();
	}

	BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)(GetScreenHeight() - bounds.y));

	/*
	* Tabs
	*/

	item.x = (float)((int)bounds.x + *scroll);
	item.y = bounds.y;
	item.width = (float)fixedTabWidth;
	item.height = (float)buttonSize;

	for (int i = 0; i < gActivityCount; i++)
	{
		if (GuiActivityTab(item, gActivities[i].name, i))
		{
			CloseActivity(i);
			i--;
		}

		item.x = (float)((int)item.x + (int)item.width);

		if (i + 1 < gActivityCount)
		{
			DrawLine((int)item.x, (int)item.y, (int)item.x + 1, (int)item.y + (int)item.height,
				GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));
		}

		item.x = (float)((int)item.x + 1);
	}

	EndScissorMode();

	if (tmp)
		GuiUnlock();
}

int GuiWindowControl(Rectangle bounds, const char* text, int* drag, int* scroll)
{
	const int iconSize = (int)(gScale * gIconSize);
	const int buttonSize = iconSize + gPadding * 2;
	int tmp, result = 1;

	Rectangle item = bounds;

	GuiSetStyleDefault();
	GuiEnableTooltip();

	/*
	* Close Button
	*/

	item.x = (float)((int)item.x + (int)item.width - buttonSize);
	item.width = (float)buttonSize;
	item.height = (float)buttonSize;

	if (GuiIconButtonEx(item, "Close", gIcons, gIconSize, gPadding, ICON_CUSTOM_MULTIPLY, gScale))
	{
		result = 0;
	}

	/*
	* Maximize Button
	*/

	item.x = (float)((int)item.x - buttonSize - 1);

	if (IsWindowMaximized())
	{
		if (GuiIconButtonEx(item, "Normal", gIcons, gIconSize, gPadding, ICON_CUSTOM_SCREEN_NORMAL, gScale))
		{
			RestoreWindow();
		}
	}
	else
	{
		if (GuiIconButtonEx(item, "Maximize", gIcons, gIconSize, gPadding, ICON_CUSTOM_SCREEN_FULL, gScale))
		{
			MaximizeWindow();
		}
	}

	/*
	* Minimize Button
	*/

	item.x = (float)((int)item.x - buttonSize - 1);

	if (GuiIconButtonEx(item, "Minimize", gIcons, gIconSize, gPadding, ICON_CUSTOM_MINUS, gScale))
	{
		MinimizeWindow();
	}

	GuiDisableTooltip();

	/*
	* Tabs
	*/

	item.width = (float)((int)item.x - (int)bounds.x - gPadding * 2 - buttonSize * 2);
	item.height = (float)buttonSize;
	item.x = bounds.x + (float)gPadding;
	item.y = bounds.y + (float)gPadding;

	GuiActivityTabs(item, scroll, &tmp);

	/*
	* New Tab Button
	*/

	GuiSetStyleDefault();

	item.x = (float)((int)item.x + (int)item.width - tmp + gPadding);
	item.width = (float)buttonSize;
	tmp -= buttonSize + gPadding;

	GuiEnableTooltip();

	if (GuiIconButtonEx(item, "New Tab", gIcons, gIconSize, gPadding, ICON_CUSTOM_PLUS, gScale))
	{
		AddActivity(OpenExplorer(gDefaultDirectory), gActivityCount);
		gCurrentActivity = gActivityCount - 1;
	}
	
	GuiDisableTooltip();

	/*
	* Movement
	*/

	item.x = (float)((int)item.x + buttonSize + gPadding);
	item.width = (float)(tmp + buttonSize * 2);

	if (*drag == 0 && CheckCollisionPointRec(GetMousePosition(), item) &&
		IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		windowX = (int)GetWindowPosition().x;
		windowY = (int)GetWindowPosition().y;
		mouseX = windowX + GetMouseX();
		mouseY = windowY + GetMouseY();
		*drag = 1;
	}

	if (*drag)
	{
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			int moveX = (int)GetWindowPosition().x + GetMouseX() - mouseX;
			int moveY = (int)GetWindowPosition().y + GetMouseY() - mouseY;

			int posX = windowX + moveX;
			int posY = windowY + moveY;

			SetWindowPosition(posX, posY);
		}
		else
		{
			*drag = 0;
		}
	}

	return result;
}

int AddActivity(Activity activity, int index)
{
	if (gActivityCount == MAX_ACTIVITIES ||
		activity.name == NULL			 ||
		activity.data == NULL			 ||
		activity.Draw == NULL			 ||
		activity.Close == NULL)
		return 0;

	gActivities[gActivityCount] = activity;
	gActivityCount++;

	return 1;
}

void CloseActivity(int index)
{
	Activity activity;
	
	if (index >= gActivityCount)
		return;

	activity = gActivities[index];
	activity.Close(&activity);
	memmove(gActivities + index, gActivities + index + 1, (gActivityCount - index - 1) *
		sizeof(Activity));
	gActivityCount--;
}