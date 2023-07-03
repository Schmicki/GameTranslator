#include "platform/filesystem.h"
#include "context-menu.h"
#include "explorer.h"

/**************************************************************************************************
* Crawler functions
*/

static void CrawlerMain(void* data)
{
	CrawlerState* state = (CrawlerState*)data;

	while (1)
	{
		while (1)
		{
			int tmp, target, result;

			FutexWait(&state->futex, 0);

			tmp = AtomicLoadInteger(&state->futex);
			target = tmp > 0 ? tmp - 1 : tmp;
			result = AtomicCompareAndSwapInteger(&state->futex, target, tmp);

			if (result == tmp && tmp != 0)
				break;
		}
		
		LockSpinLock(&state->nextLock);

		state->generation = state->nextGeneration;
		memcpy(state->path, state->nextPath, strlen(state->nextPath) + 1);
		memcpy(state->filter, state->nextFilter, strlen(state->nextFilter) + 1);

		UnlockSpinLock(&state->nextLock);

		if (state->filter[0] == 0)
		{
			FilePathList files = LoadDirectoryFiles(state->path);
			FileNode* first = NULL;
			FileNode* last = NULL;
			
			for (int i = 0; i < (int)files.count; i++)
			{
				FileNode* node = malloc(sizeof(FileNode));

				if (node == NULL)
					continue;

				else if (first == NULL)
					first = node;

				else
					last->next = node;

				last = node;

				node->next = NULL;
				node->generation = state->generation;
				node->path = files.paths[i];
				node->format = GetFileFormat(files.paths[i]);
			}

			LockSpinLock(&state->queueLock);
			
			if (state->last == NULL)
				state->first = first;

			else
				state->last->next = first;

			state->last = last;
			
			UnlockSpinLock(&state->queueLock);
		}
		else
		{

		}
	}
}

/**************************************************************************************************
* Helper functions
*/

void PullFiles(ExplorerState* state)
{
	CrawlerState* crawler = state->crawler;
	FileNode* first;
	FileNode* last;
	FileNode* node;

	LockSpinLock(&crawler->queueLock);

	first = crawler->first;
	last = crawler->last;

	crawler->first = NULL;
	crawler->last = NULL;

	UnlockSpinLock(&crawler->queueLock);

	node = first;

	while (node != NULL)
	{
		FileNode* next = node->next;

		if (node->generation != state->generation)
		{
			free(node->path);
			free(node);
			node = next;
			continue;
		}

		if (state->files.count == state->files.capacity)
		{
			FileList list;
			double* selected;

			list.capacity = state->files.capacity * 2;
			list.count = state->files.count;
			
			if (list.capacity == 0)
				list.capacity = 64;

			list.paths = malloc(sizeof(char*) * list.capacity);

			if (list.paths == NULL)
				goto fail;

			list.formats = malloc(sizeof(FileFormat*) * list.capacity);

			if (list.formats == NULL)
			{
				free(list.paths);
				goto fail;
			}

			selected = malloc(sizeof(double) * list.capacity);

			if (selected == NULL)
			{
				free(list.paths);
				free(list.formats);
				goto fail;
			}

			memcpy(list.paths, state->files.paths, state->files.count * sizeof(char*));
			memcpy(list.formats, state->files.formats, state->files.count * sizeof(FileFormat*));
			memcpy(selected, state->selected, state->files.count * sizeof(double));

			free(state->files.paths);
			free(state->files.formats);
			free(state->selected);
			state->files = list;
			state->selected = selected;
		}

		state->files.paths[state->files.count] = node->path;
		state->files.formats[state->files.count] = node->format;
		state->selected[state->files.count] = 0.0;
		state->files.count++;

		free(node);
		node = next;
		continue;

	fail:
		free(node->path);
		free(node);
		node = next;
	}
}

void ExplorerReload(ExplorerState* state)
{
	CrawlerState* crawler = state->crawler;

	state->reName[0] = 0;
	state->lastSelected = 0;

	for (int i = 0; i < state->files.count; i++)
	{
		free(state->files.paths[i]);
	}

	free(state->files.paths);
	free(state->files.formats);
	free(state->selected);

	state->files.capacity = 0;
	state->files.count = 0;
	state->files.paths = NULL;
	state->files.formats = NULL;
	state->selected = NULL;
	state->generation++;

	LockSpinLock(&crawler->nextLock);

	memcpy(crawler->nextPath, state->path, strlen(state->path) + 1);
	crawler->nextFilter[0] = 0;
	crawler->nextGeneration = state->generation;

	UnlockSpinLock(&crawler->nextLock);

	while (1)
	{
		int tmp, result;

		tmp = AtomicLoadInteger(&crawler->futex);
		result = AtomicCompareAndSwapInteger(&crawler->futex, tmp + 1, tmp);

		if (result == tmp)
		{
			FutexWakeSingle(&crawler->futex);
			break;
		}
	}
}

int ExplorerGetSelectedFile(ExplorerState* state)
{
	if (state->selected[state->lastSelected])
		return state->lastSelected;

	for (int i = 0; i < (int)state->files.count; i++)
		if (state->selected[i])
			return i;

	return -1;
}

void ExplorerBeginRename(ExplorerState* state, int index)
{
	const char* path;

	path = state->files.paths[index];
	memcpy(state->reName, path, strlen(path) + 1);

	state->renameIndex = index;
}

void ExplorerEndRename(ExplorerState* state)
{
	int length;
	char* path;

	if (rename(state->files.paths[state->renameIndex], state->reName) == -1)
		goto clean;

	length = (int)strlen(state->reName) + 1;
	path = malloc(length);

	if (path == NULL)
		goto clean;

	memcpy(path, state->reName, length);
	free(state->files.paths[state->renameIndex]);
	state->files.paths[state->renameIndex] = path;

clean:
	*state->reName = 0;
	state->renameIndex = -1;
}

static void DeselectAllFiles(ExplorerState* state)
{
	for (int iter = 0; iter < (int)state->files.count; iter++)
		state->selected[iter] = 0.0;
}

static char IsFileDoubleClicked(ExplorerState* state, int i)
{
	double selected = state->selected[i];
	return selected > 0.0 && (GetTime() - selected) <= 0.4;
}

static void FileViewerOnEvent(ExplorerState* state, int event, int index)
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
			ExplorerReload(state);
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

/**************************************************************************************************
* Gui functions
*/

static int DrawCurrentDirectory(ExplorerState* state, Rectangle bounds)
{
	const int iconSize = (int)(gIconSize * gScale);
	const int buttonSize = iconSize + gPadding * 2;
	int tmp, pressed = 0;

	Rectangle itemBounds;
	itemBounds.width = (float)buttonSize;
	itemBounds.height = (float)buttonSize;

	GuiSetStyleDefault();
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
		pressed = 2;
	}

	/*
	* Previous Folder
	*/

	itemBounds.x = (float)((int)itemBounds.x + tmp);

	if (GuiIconButtonEx(itemBounds, "Previous Folder", gIcons, gIconSize, gPadding,
		ICON_CUSTOM_ARROW_UP, gScale))
	{
		pressed = 3;
		char* c = NULL;

		if (c == NULL)
			c = strrchr(state->path, '/');

		if (c == NULL)
			c = strrchr(state->path, '\\');

		if (c != NULL)
		{
			*c = 0;
			state->scroll.y = 0;
			ExplorerReload(state);
		}
	}

	/*
	* Search
	*/

	itemBounds.x = (float)((int)bounds.x + (int)bounds.width - tmp);

	if (GuiIconButtonEx(itemBounds, "Filter", gIcons, gIconSize, gPadding,
		ICON_CUSTOM_SEARCH, gScale))
	{
		pressed = 4;

		if (state->showFilter)
		{
			*state->filter = 0;
			state->showFilter = 0;
			state->editFilter = 0;
		}
		else
		{
			*state->filter = 0;
			state->showFilter = 1;
			state->editFilter = 1;
		}
	}

	/*
	* Refresh
	*/

	itemBounds.x = (float)((int)itemBounds.x - tmp - 1);

	if (GuiIconButtonEx(itemBounds, "Refresh", gIcons, gIconSize, gPadding,
		ICON_CUSTOM_SYNCHRONIZE, gScale))
	{
		pressed = 5;
		state->scroll.y = 0;
		ExplorerReload(state);
	}

	GuiDisableTooltip();

	/*
	* Text Box
	*/

	itemBounds.width = (float)((int)itemBounds.x - ((int)bounds.x + tmp * 3 + gPadding + 1));
	itemBounds.x = itemBounds.x - itemBounds.width - 1.0f;

	if (!state->showFilter)
	{
		GuiSetStyleTextFocusable();
		GuiTextBox(itemBounds, state->path, iconSize, 0);
		GuiSetStyleTextDefault();

		if (!GuiIsLocked() && GuiGetState() != STATE_DISABLED && CheckCollisionPointRec(
			GetMousePosition(), itemBounds) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		{
			state->overlay = OpenDriveSelectMenu();
		}
	}
	else
	{
		int edit = state->editFilter;

		GuiSetStyleTextboxOutlined();
		edit = GuiTextBox(itemBounds, state->filter, iconSize, state->editFilter) ? (edit == 1 ? 0 : 1) :
			edit;
		GuiSetStyleTextboxDefault();

		if (pressed == 4)
			edit = 1;

		state->editFilter = edit;
	}

	/* Draw Text Border */
	itemBounds.width = (float)((int)itemBounds.width + buttonSize + 2);

	/*DrawRectangleLinesEx(itemBounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), GetColor(GuiGetStyle(
		DEFAULT, LINE_COLOR)));*/

	return pressed != 0;
}

static int FileViewerButton(ExplorerState* state, Rectangle bounds, int index)
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
			ExplorerEndRename(state);
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
			ExplorerEndRename(state);
	}

	if (state->selected[index] > 0.0)
	{
		DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height,
			GetColor(GuiGetStyle(BUTTON, BASE_COLOR_PRESSED)));

		if (index == state->renameIndex)
		{
			if (IsKeyPressed(KEY_ENTER))
			{
				ExplorerEndRename(state);
			}
			else
			{
				char* name = (char*)GetFileName(state->reName);

				label.width = Clamp(MeasureTextEx(gFont, name, (float)iconSize, (float)GuiGetStyle(
					DEFAULT, TEXT_SPACING)).x + iconSize, (float)(buttonSize * 3), label.width);

				GuiSetStyleTextboxOutlined(iconSize);
				GuiTextBox(label, name, 0x1000 - (int)strlen(name), 1);
				GuiSetStyleTextboxDefault(iconSize);

				drawLabel = 0;
			}
		}
	}

	if (drawLabel)
	{
		GuiLabel(label, GetFileName(state->files.paths[index]));
	}

	GuiDrawCustomIcon(gIcons, gIconSize, state->files.formats[index]->icon, (int)icon.x,
		(int)icon.y, gScale, RAYWHITE);

	return result;
}

static void DrawFileViewer(ExplorerState* state, Rectangle bounds)
{
	const int iconSize = (int)(gIconSize * gScale);
	const int buttonSize = iconSize + gPadding * 2;
	int tmp = gPadding + buttonSize + 1;
	int first, last;
	int event, index;

	Rectangle view;
	Rectangle content;

	GuiSetStyleDefault();

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

void ExplorerDraw(Activity* activity, Rectangle bounds)
{
	ExplorerState* state = (ExplorerState*)activity->data;
	int unlock = 0;
	Rectangle itemBounds;

	PullFiles(state);

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

void CloseExplorer(Activity* activity)
{
	free(activity->data);

	activity->name = NULL;
	activity->data = NULL;
	activity->Draw = NULL;
	activity->Close = NULL;
}

Activity OpenExplorer(const char* path)
{
	Activity activity = { NULL, NULL, NULL, NULL };
	ExplorerState* explorer;
	CrawlerState* crawler;
	char absolutePath[MAX_PATH_LENGTH];
	int size;

	if (path == NULL || !DirectoryExists(path))
		return activity;

	explorer = malloc(sizeof(ExplorerState));

	if (explorer == NULL)
		return activity;

	crawler = malloc(sizeof(CrawlerState));

	if (crawler == NULL)
	{
		free(explorer);
		return activity;
	}

	size = GetAbsolutePath(path, MAX_PATH_LENGTH, absolutePath) + 1;
	StringReplaceChar(absolutePath, '\\', '/');
	memcpy(explorer->path, absolutePath, size);
	memcpy(crawler->path, absolutePath, size);

	crawler->generation = 0;
	/* path is set above */
	crawler->filter[0] = 0;
	crawler->futex = 0;
	crawler->nextLock = 0;
	crawler->nextGeneration = 0;
	crawler->nextPath[0] = 0;
	crawler->nextFilter[0] = 0;
	crawler->queueLock = 0;
	crawler->first = NULL;
	crawler->last = NULL;

	explorer->generation = 0;
	/* path is set above */
	explorer->filter[0] = 0;
	explorer->reName[0] = 0;
	explorer->copyTarget[0] = 0;
	explorer->files.capacity = 0;
	explorer->files.count = 0;
	explorer->files.paths = NULL;
	explorer->files.formats = NULL;
	explorer->selected = NULL;
	explorer->lastSelected = 0;
	explorer->showFilter = 0;
	explorer->editFilter = 0;
	explorer->renameIndex = -1;
	explorer->copyFlags = 0;
	explorer->overlay.Draw = NULL;
	explorer->overlay.data = NULL;
	explorer->scroll.x = 0.0f;
	explorer->scroll.y = 0.0f;
	explorer->crawler = crawler;
	explorer->crawlerThread = InitThread(&CrawlerMain, crawler);

	ExplorerReload(explorer);

	activity.name = "Explorer";
	activity.data = explorer;
	activity.Draw = &ExplorerDraw;
	activity.Close = &CloseExplorer;

	return activity;
}