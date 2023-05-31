#include "platform/filesystem.h"
#include "context-menu.h"
#include "file-explorer.h"

/**************************************************************************************************
* Helper functions
*/

void ReloadFilesAndTypes(FileManagerState* state)
{
	if (state->files.capacity > 0)
	{
		UnloadDirectoryFiles(state->files);
		free(state->formats);
	}

	if (state->rename)
	{
		free(state->rename);
		state->rename = NULL;
		state->renameIndex = -1;
	}

	state->lastSelected = 0;

	state->files = LoadDirectoryFiles(state->path);

	size_t formatsSize = sizeof(FileFormat*) * state->files.capacity;
	size_t selectedSize = sizeof(double) * state->files.capacity;

	state->formats = malloc(formatsSize + selectedSize);
	state->selected = (double*)((char*)state->formats + formatsSize);

	for (int i = 0; i < (int)state->files.count; i++)
	{
		state->formats[i] = GetFileFormat(state->files.paths[i]);
		state->selected[i] = 0.0;
	}
}

static void DeselectAllFiles(FileManagerState* state)
{
	for (int iter = 0; iter < (int)state->files.count; iter++)
		state->selected[iter] = 0.0;
}

static char IsFileDoubleClicked(FileManagerState* state, int i)
{
	double selected = state->selected[i];
	return selected > 0.0 && (GetTime() - selected) <= 0.4;
}

int FileManagerGetSelectedFile(FileManagerState* state)
{
	if (state->selected[state->lastSelected])
		return state->lastSelected;

	for (int i = 0; i < (int)state->files.count; i++)
		if (state->selected[i])
			return i;

	return -1;
}

void FileManagerBeginRename(FileManagerState* state, int index)
{
	const char* path;

	state->rename = malloc(0x1000);

	if (state->rename == NULL)
		return;

	path = state->files.paths[index];
	memcpy(state->rename, path, strlen(path) + 1);

	state->renameIndex = index;
}

void FileManagerEndRename(FileManagerState* state)
{
	int length;
	char* path;

	if (rename(state->files.paths[state->renameIndex], state->rename) == -1)
		goto clean;

	length = (int)strlen(state->rename) + 1;
	path = malloc(length);

	if (path == NULL)
		goto clean;

	memcpy(path, state->rename, length);
	free(state->files.paths[state->renameIndex]);
	state->files.paths[state->renameIndex] = path;

clean:
	free(state->rename);
	state->rename = NULL;
	state->renameIndex = -1;
}

static void FileViewerOnEvent(FileManagerState* state, int event, int index)
{
	double time;

	if (event == 0)
		return;

	time = GetTime();

	switch (event)
	{
	case 1:
		if (IsKeyDown(KEY_LEFT_SHIFT))
		{
			/* If a button was clicked and shift is down, select all files from first to last. */

			int first = state->lastSelected > index ? index : state->lastSelected;
			int last = state->lastSelected < index ? index : state->lastSelected;

			DeselectAllFiles(state);

			for (; first <= last; first++)
				state->selected[first] = time;
		}
		else
		{
			state->lastSelected = index;
		}
		break;

	case 2:
		if (DirectoryExists(state->files.paths[index]))
		{
			/* If a button was double clicked and it was a folder, go into folder. */

			memcpy(state->path, state->files.paths[index], strlen(state->files.paths[index]) + 1);
			state->scroll.y = 0;
			ReloadFilesAndTypes(state);
		}
		break;

	case 3:
	case 5:

		DeselectAllFiles(state);

		if (event == 3)
		{
			state->selected[index] = time;
			state->lastSelected = index;
		}

	case 4:
		/* If a button was right clicked, open context menu. */

		state->overlay = OpenContextMenu(state, index);
		break;
	}
}

static void FileManagerSetStyleDefault(int iconSize)
{
	GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_DISABLED, 0xFFFFFFFF);

	GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xFFFFFFFF);
	GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
	GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0xccecffFF);
	GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xccecffFF);
	GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x8bd1fcFF);
	GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0x8bd1fcFF);

	GuiSetStyle(LISTVIEW, BORDER_COLOR_NORMAL, 0xFFFFFF00);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, 0xFFFFFF00);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, 0xFFFFFF00);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_DISABLED, 0xFFFFFF00);

	GuiSetStyle(DEFAULT, TEXT_SIZE, iconSize);
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x00000000);
	GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0x00000000);
	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xFFFFFFFF);
}

static void FileViewerSetStyleRename(int iconSize)
{
	GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL, 0x777777FF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, 0x777777FF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_PRESSED, 0x777777FF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_DISABLED, 0x777777FF);
}

/**************************************************************************************************
* De/Init functions
*/

FileManagerState InitFileManager(const char* path)
{
	FileManagerState state = { NULL };

	if (!DirectoryExists(path))
		return state;
	
	char absolutePath[0x400];
	int size = GetAbsolutePath(path, 0x400, absolutePath) + 1;
	StringReplaceChar(absolutePath, '\\', '/');

	if ((state.path = malloc(0x1000)) == NULL)
		return state;

	memcpy(state.path, absolutePath, size);
	ReloadFilesAndTypes(&state);

	state.renameIndex = -1;
	state.rename = NULL;

	state.copyFlags = 0;
	state.copyTarget = NULL;

	return state;
}

void CloseFileManager(FileManagerState state)
{
	if (state.path)
	{
		free(state.path);
	}

	if (state.rename)
	{
		free(state.rename);
	}

	if (state.copyTarget)
	{
		free(state.copyTarget);
	}
}

/**************************************************************************************************
* Draw functions
*/

static int DrawCurrentDirectory(FileManagerState* state, Rectangle bounds)
{
	const int iconSize = (int)(gIconSize * gScale);
	const int buttonSize = iconSize + gPadding * 2;
	int tmp, pressed = 0;

	Rectangle itemBounds;
	itemBounds.width = (float)buttonSize;
	itemBounds.height = (float)buttonSize;

	FileManagerSetStyleDefault(iconSize);
	GuiEnableTooltip();

	/*
	* Undo
	*/

	itemBounds.x = (float)((int)bounds.x + gPadding);
	itemBounds.y = (float)((int)bounds.y + 1);

	if (GuiIconButtonEx(itemBounds, "Back", gIcons, gIconSize, gPadding,
		ICON_CUSTOM_ARROW_LEFT, gScale))
	{
		pressed = 1;
	}

	/*
	* Redo
	*/

	tmp = buttonSize + gPadding;
	itemBounds.x = (float)((int)itemBounds.x + tmp);

	if (GuiIconButtonEx(itemBounds, "Forward", gIcons, gIconSize, gPadding,
		ICON_CUSTOM_ARROW_RIGHT, gScale))
	{
		pressed = 1;
	}

	/*
	* Previous Folder
	*/

	itemBounds.x = (float)((int)itemBounds.x + tmp);

	if (GuiIconButtonEx(itemBounds, "Previous Folder", gIcons, gIconSize, gPadding,
		ICON_CUSTOM_ARROW_UP, gScale))
	{
		pressed = 1;
		char* c = NULL;

		if (c == NULL)
			c = strrchr(state->path, '/');

		if (c == NULL)
			c = strrchr(state->path, '\\');

		if (c != NULL)
		{
			*c = 0;
			state->scroll.y = 0;
			ReloadFilesAndTypes(state);
		}
	}

	/*
	* Search
	*/

	itemBounds.x = (float)((int)bounds.x + (int)bounds.width - tmp);

	if (GuiIconButtonEx(itemBounds, "Filter", gIcons, gIconSize, gPadding,
		ICON_CUSTOM_SEARCH, gScale))
	{

	}

	/*
	* Refresh
	*/

	itemBounds.x = (float)((int)itemBounds.x - tmp - 1);

	if (GuiIconButtonEx(itemBounds, "Refresh", gIcons, gIconSize, gPadding,
		ICON_CUSTOM_SYNCHRONIZE, gScale))
	{
		pressed = 1;
		state->scroll.y = 0;
		ReloadFilesAndTypes(state);
	}

	GuiDisableTooltip();

	/*
	* Text Box
	*/

	itemBounds.width = (float)((int)itemBounds.x - ((int)bounds.x + tmp * 3 + gPadding + 1));
	itemBounds.x = itemBounds.x - itemBounds.width - 1.0f;

	if (GuiTextBox(itemBounds, state->path, iconSize, 0))
	{
		state->overlay = OpenDriveSelectMenu();
	}

	/*
	* Draw Text Border
	*/

	itemBounds.x -= 1.0f;
	itemBounds.width = (float)((int)itemBounds.width + buttonSize + 3);
	itemBounds.height = (float)((int)itemBounds.height + 1);

	DrawRectangleLinesEx(itemBounds, 1, GetColor(0xAAAAAAFF));

	return pressed;
}

static int FileViewerButton(FileManagerState* state, Rectangle bounds, int index)
{
	const int iconSize = (int)(gIconSize * gScale);
	const int buttonSize = iconSize + gPadding * 2;
	int drawLabel = 1;

	Rectangle label = {
		   bounds.x + (float)buttonSize,
		   bounds.y + gPadding,
		   bounds.width - (float)buttonSize,
		   (float)buttonSize - gPadding * 2
	};

	Vector2 icon = {
		bounds.x + (float)gPadding,
		bounds.y + (float)gPadding
	};

	int event, result;

	if (!GuiIsLocked() &&
		!IsKeyDown(KEY_LEFT_CONTROL) &&
		!IsKeyDown(KEY_LEFT_SHIFT) &&
		(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) &&
		!CheckCollisionPointRec(GetMousePosition(), bounds))
	{
		state->selected[index] = 0.0;

		if (state->renameIndex != -1 && index == state->renameIndex)
			FileManagerEndRename(state);
	}

	event = GuiButtonEx(bounds, "");
	result = 0;

	if (event == 1)
	{
		result = (int)IsFileDoubleClicked(state, index) + 1;
	}
	else if (event == 2)
	{
		result = (int)(state->selected[index] > 0.0) + 3;
	}

	if (event != 0)
	{
		state->selected[index] = GetTime();

		if (state->renameIndex != -1 && index != state->renameIndex)
			FileManagerEndRename(state);
	}

	if (state->selected[index] > 0.0)
	{
		DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height,
			GetColor(GuiGetStyle(BUTTON, BASE_COLOR_PRESSED)));

		if (index == state->renameIndex)
		{
			if (IsKeyPressed(KEY_ENTER))
			{
				FileManagerEndRename(state);
			}
			else
			{
				char* name = (char*)GetFileName(state->rename);

				label.width = Clamp(MeasureTextEx(gFont, name, (float)iconSize, (float)GuiGetStyle(DEFAULT,
					TEXT_SPACING)).x + iconSize, (float)(buttonSize * 3), label.width);

				FileViewerSetStyleRename(iconSize);
				DrawRectangleRec(label, GetColor(0xFFFFFFFF));
				GuiTextBox(label, name, 0x1000 - (int)strlen(name), 1);
				FileManagerSetStyleDefault(iconSize);

				drawLabel = 0;
			}
		}
	}

	if (drawLabel)
	{
		GuiLabel(label, GetFileName(state->files.paths[index]));
	}

	GuiDrawCustomIcon(gIcons, gIconSize, state->formats[index]->icon, (int)icon.x,
		(int)icon.y, gScale, RAYWHITE);

	return result;
}

static void DrawFileViewer(FileManagerState* state, Rectangle bounds)
{
	const int iconSize = (int)(gIconSize * gScale);
	const int buttonSize = iconSize + gPadding * 2;
	int tmp = gPadding + buttonSize + 1;
	int first, last;
	int event, index;

	Rectangle view;
	Rectangle content;

	FileManagerSetStyleDefault(iconSize);

	/* Scroll Panel */

	view.x = bounds.x;
	view.y = (float)((int)bounds.y + tmp);
	view.width = bounds.width;
	view.height = (float)((int)bounds.height - tmp);

	tmp = buttonSize + 1;

	content.x = 0;
	content.y = 0;
	content.height = (float)(tmp * state->files.count);

	if (content.height > view.height)
		content.width = (float)((int)view.width - GuiGetStyle(SLIDER, SLIDER_WIDTH));
	else
		content.width = (float)((int)view.width - GuiGetStyle(SLIDER, SLIDER_PADDING) * 2);

	view = GuiScrollPanel(view, NULL, content, &state->scroll);
	BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);

	/* File Viewer	*/

	first = -(int)state->scroll.y / tmp;
	last = first + (int)view.height / tmp + 2;

	if (last > (int)state->files.count)
		last = (int)state->files.count;

	content.x = view.x + gPadding;
	content.y = view.y + state->scroll.y + first * tmp;
	content.width = view.width - gPadding * 2;
	content.height = (float)buttonSize;

	event = 0;
	index = 0;

	for (int i = first; i < last; i++)
	{
		tmp = FileViewerButton(state, content, i);

		if (tmp != 0)
		{
			event = tmp;
			index = i;
		}

		content.y += (float)buttonSize + 1;
	}

	EndScissorMode();

	/*
	* Empty space
	*/

	content.height = (float)((int)bounds.y + (int)bounds.height - (int)content.y);
	tmp = GuiInvisibleButtonEx(content);

	if (tmp == 2)
	{
		event = 5;
		index = -1;
	}

	FileViewerOnEvent(state, event, index);
}

void GuiFileManager(FileManagerState* state, Rectangle bounds)
{
	int unlock = 0;
	Rectangle itemBounds;

	BeginTextureMode(gTopLayerTexture);

	if (state->overlay.data != NULL)
	{
		if (state->overlay.Draw(state, state->overlay.data, bounds) || state->overlay.data)
			GuiLock();
	}

	EndTextureMode();

	/*
	* File Viewer
	*/

	itemBounds.x = bounds.x;
	itemBounds.y = (float)((int)bounds.y + gScale * gIconSize + gPadding * 4 + 1);
	itemBounds.width = bounds.width;
	itemBounds.height = bounds.y + bounds.height - itemBounds.y;

	if (!GuiIsLocked() && !CheckCollisionPointRec(GetMousePosition(), itemBounds))
	{
		unlock = 1;
		GuiLock();
	}

	DrawFileViewer(state, bounds);

	if (unlock)
		GuiUnlock();

	/*
	* Current Directory
	*/

	DrawCurrentDirectory(state, bounds);

	GuiUnlock();
}

/**************************************************************************************************
* Activity functions
*/

void FileManagerActivityDraw(Activity* activity, Rectangle bounds)
{
	GuiFileManager((FileManagerState*)activity->data, bounds);
}

void CloseFileManagerActivity(Activity* activity)
{
	FileManagerState* state = (FileManagerState*)activity->data;

	CloseFileManager(*state);

	activity->name = NULL;
	activity->data = NULL;
	activity->Draw = NULL;
	activity->Close = NULL;
}

Activity OpenFileManagerActivity(const char* path)
{
	Activity activity = { NULL, NULL, NULL, NULL };

	if (path == NULL)
		return activity;

	activity.data = malloc(sizeof(FileManagerState));

	if (activity.data == NULL)
		return activity;

	activity.name = "File Manager";
	*((FileManagerState*)activity.data) = InitFileManager(path);
	activity.Draw = &FileManagerActivityDraw;
	activity.Close = &CloseFileManagerActivity;

	return activity;
}