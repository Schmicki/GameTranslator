#include "context-menu.h"
#include "platform/filesystem.h"

#include "tools/hex-viewer.h"
#include "trim-overlay.h"

#include "formats/default.h"
#include "formats/folder.h"
#include "formats/unknown.h"

/**************************************************************************************************
* The context menu uses the format of the right clicked element. There can be multiple selected
* elements of different formats. So it is important that if multiple files are processed by some
* action, you do a check in the action to confirm the format.
*/

enum ContextMenuFlags
{
    CONTEXT_MENU_MULTIPLE = 1,
    CONTEXT_MENU_SHOW_HEX = 2,
};

typedef struct ContextMenuState
{
    ContextActionList actions;
    FileFormatList formats;
    Vector2 position;
	int mode;
	int scroll;
    int index;
} ContextMenuState;

typedef struct DriveSelectMenuState
{
    FilePathList drives;
    Vector2 position;
    int scroll;
} DriveSelectMenuState;

typedef struct CustomActionFormatMenuState
{
    FileFormatList formats;
    Vector2 position;
    int scroll;
    int index;
} CustomActionFormatMenuState;

/**************************************************************************************************
* Actions
*/

void ActionCustomAction(ExplorerState* state, int index)
{
}

void ActionCopy(ExplorerState* state, int index)
{
    char* path;
    int length;

    if (index == -1)
        return;

    path = state->files.paths[index];
    length = (int)strlen(path) + 1;

    memcpy(state->copyTarget, path, length);
    state->copyFlags = 0;
}

void ActionCut(ExplorerState* state, int index)
{
    char* path;
    int length;

    if (index == -1)
        return;

    path = state->files.paths[index];
    length = (int)strlen(path) + 1;

    memcpy(state->copyTarget, path, length);
    state->copyFlags = 1;
}

void ActionPaste(ExplorerState* state, int index)
{
    char* path;
    char* name;
    int length = 0;

    if (state->copyTarget == NULL)
        return;

    while (state->copyTarget[length] == state->path[length] &&
        state->copyTarget[length] != 0 &&
        state->path[length] != 0)
    {
        length++;
    }

    if (state->copyTarget[length] == 0)
    {
        /* Path is inside copy target. Cannot move into self! */
        if (state->copyFlags != 0)
            return;
    }
    else if (state->path[length] == 0)
    {
        /* Source is equal to Destination. Do nothing. */
        if (state->copyFlags != 0 &&
            !strchr(state->copyTarget + length + 1, '/') &&
            !strchr(state->copyTarget + length + 1, '\\'))
            return;
    }

    path = malloc(0x1000);

    if (path == NULL)
        return;

    length = (int)strlen(state->path) + 1;
    memcpy(path, state->path, length);
    path[length - 1] = '/';

    name = (char*)GetFileName(state->copyTarget);
    memcpy(path + length, name, strlen(name) + 1);
    
    if (!GetUniqueFileName(path, 0x1000))
        goto clean;

    if (IsPathFile(state->copyTarget))
    {
        /* Copy/Move file */
        if (state->copyFlags == 0)
            CopyFile(state->copyTarget, path);
        else
            MoveFile(state->copyTarget, path);
    }
    else
    {
        /* Copy/Move folder */
        if (state->copyFlags == 0)
            CopyFolder(state->copyTarget, path);
        else
            MoveFolder(state->copyTarget, path);
    }

    if (state->copyFlags == 1)
    {
        state->copyTarget[0] = 0;
    }

    ExplorerReload(state);

clean:
    free(path);
}

void ActionRename(ExplorerState* state, int index)
{
    if (index == -1)
        return;

    ExplorerBeginRename(state, index);
}

void ActionDelete(ExplorerState* state, int index)
{
    for (int i = 0; i < (int)state->files.count; i++)
    {
        if (state->selected[i])
        {
            const char* file = state->files.paths[i];
            if (IsPathFile(file))
            {
                printf("delete file %s\n", file);
                DeleteFile(file);
            }
            else
            {
                DeleteFolder(file);
            }
        }
    }

    ExplorerReload(state);
}

void ActionAddToArchive(ExplorerState* state, int index)
{
    for (int i = 0; i < (int)state->files.count; i++)
    {
        if (state->selected[i])
            printf("added %s to archive\n", state->files.paths[i]);
    }
}

void ActionHexEditor(ExplorerState* state, int index)
{
    int i = index;

    if (i != -1 && state->files.formats[i]->type != FILE_TYPE_FOLDER)
    {
        AddActivity(OpenHexEditor(state->files.paths[i]), gCurrentActivity + 1);
        gCurrentActivity = gActivityCount - 1;
    }
}

void ActionTrim(ExplorerState* state, int index)
{
    int i = index;

    if (i != -1 && state->files.formats[i]->type != FILE_TYPE_FOLDER)
    {
        state->overlay = OpenTrimOverlay(state->files.paths[i]);
    }
}

/**************************************************************************************************
* Helper functions
*/

static ContextActionList LoadContextActions(ExplorerState* state)
{
    ContextActionList actions = { 0, 0, NULL, NULL };
    FileFormat* format = NULL;

    if (state->files.count == 0 || (state->selected[state->lastSelected] == 0.0))
        format = &formatFolder;
    else
        format = state->files.formats[state->lastSelected];

    actions.capacity = MAX_CONTEXT_ACTIONS;
    actions.count = 0;
    actions.functions = malloc(actions.capacity * sizeof(ContextAction));
    actions.names = malloc(actions.capacity * sizeof(const char*));

    if (format != NULL && format->GetActions != NULL)
        format->GetActions(&actions);

    actions.names[actions.count] = memcpy(malloc(18), "Custom Action ...", 18);
    actions.functions[actions.count] = &ActionCustomAction;
    actions.count++;

    return actions;
}

static void UnloadContextActions(ContextActionList actions)
{
    if (actions.capacity)
    {
        for (int i = 0; i < actions.count; free(actions.names[i]), i++);
        free(actions.functions);
        free(actions.names);
    }
}

static int ContextMenuOnAction(ExplorerState* state, ContextMenuState* menu, int index)
{
    ContextAction action = menu->actions.functions[index];
    int result = 1;

    if (action == &ActionCustomAction)
    {
        UnloadContextActions(menu->actions);
        menu->formats = LoadFormatList();
        menu->mode = 1;

        if (menu->formats.count != 0)
        {
            result = 0;
        }
    }
    else
    {
        action(state, menu->index);
    }

    return result;
}

static int ContextMenuOnFormat(ExplorerState* state, ContextMenuState* menu, int index)
{
    ContextActionList actions = { 0, 0, NULL, NULL };
    FileFormat* format = menu->formats.formats[index];

    UnloadFormatList(menu->formats);
    menu->mode = 0;

    actions.functions = malloc(MAX_CONTEXT_ACTIONS * sizeof(ContextAction));

    if (actions.functions == NULL)
        return 1;

    actions.names = malloc(MAX_CONTEXT_ACTIONS * sizeof(char*));

    if (actions.names == NULL)
        goto cleanFunctions;

    if (format->GetActions == NULL || (format->GetActions(&actions), actions.count == 0))
        goto cleanNames;

    menu->actions = actions;
    return 0;

cleanNames:
    free(actions.names);

cleanFunctions:
    free(actions.functions);
    return 1;
}

/*************************************************************************************************/

int GuiListMenu(Vector2 position, const char** names, int count, int* scroll)
{
    const int iconSize = (int)(gIconSize * gScale);
    const int buttonSize = iconSize + gPadding * 2;
	const int spacing = GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING);

	Rectangle contextBounds = {
		position.x,
		position.y,
		300,
		(buttonSize + spacing) * (float)count + spacing * 2
	};

    float screenHeight = (float)GetScreenHeight();
    float screenWidth = (float)GetScreenWidth();

    GuiSetStyleListViewOutlined();
    contextBounds = ClampBounds(contextBounds, (Rectangle) {
        0.0f, 0.0f, (float)GetScreenWidth(),
            (float)GetScreenHeight()
    });

	if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
		IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) &&
		!CheckCollisionPointRec(GetMousePosition(), contextBounds))
        return -2;

	return GuiListViewEx(contextBounds, names, count, NULL, scroll, -1);
}

/**************************************************************************************************
* ContextMenu functions
*/

void CloseContextMenu(ExplorerState* state, ContextMenuState* menu)
{
    if (state->overlay.data == menu)
    {
        state->overlay.Draw = NULL;
        state->overlay.data = NULL;
    }

    if (menu->mode == 0)
	    UnloadContextActions(menu->actions);
	else
        UnloadFormatList(menu->formats);
    free(menu);
}

int GuiContextMenu(ExplorerState* state, ContextMenuState* menu, Rectangle bounds)
{
    const int iconSize = (int)(gIconSize * gScale);
    const int buttonSize = iconSize + gPadding * 2;
    const int spacing = GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING);
    const int itemCount = menu->mode == 0 ? menu->actions.count : menu->formats.count;
    const char** names;
    int result;

    /*
    * Calc menu bounds
    */

    Rectangle item = {
        menu->position.x,
        menu->position.y,
        600.0f * gScale,
        (float)(buttonSize + gPadding * 2 /* Quick buttons */
        + (buttonSize + spacing) * itemCount + spacing * 2) /* List */
    };

    Rectangle max = {
        0,
        0,
        (float)GetScreenWidth(),
        (float)GetScreenHeight(),
    };

    item = ClampBounds(item, max);

    if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
        IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) &&
        !CheckCollisionPointRec(GetMousePosition(), item))
    {
        CloseContextMenu(state, menu);
        return 0;
    }

    /*
    * Draw background
    */

    GuiPanel(item, NULL);

    /*
    * List Menu
    */

    max.x = item.x + gPadding;
    max.y = item.y + gPadding;
    max.width = (float)buttonSize;
    max.height = (float)buttonSize;

	GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
	GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, buttonSize);
    GuiSetStyleListViewOutlined();
    
    item.y = (float)((int)item.y + buttonSize + gPadding * 2);
    item.height = (float)((int)item.height - buttonSize - gPadding * 2);
    names = menu->mode == 0 ? menu->actions.names : menu->formats.names;

    result = GuiListViewEx(item, names, itemCount, NULL, &menu->scroll, -1);

    /*
    * Quick buttons
    */

    GuiEnableTooltip();

    if (GuiIconButtonEx(max, "Cut", gIcons, gIconSize, gPadding, ICON_CUSTOM_CUT, gScale))
    {
        ActionCut(state, menu->index);
        result = -2;
    }

    max.x = (float)((int)max.x + buttonSize + gPadding);

    if (GuiIconButtonEx(max, "Copy", gIcons, gIconSize, gPadding, ICON_CUSTOM_COPY, gScale))
    {
        ActionCopy(state, menu->index);
        result = -2;
    }

    max.x = (float)((int)max.x + buttonSize + gPadding);

    if (GuiIconButtonEx(max, "Paste", gIcons, gIconSize, gPadding, ICON_CUSTOM_PASTE, gScale))
    {
        ActionPaste(state, menu->index);
        result = -2;
    }

    max.x = (float)((int)max.x + buttonSize + gPadding);

    if (GuiIconButtonEx(max, "Rename", gIcons, gIconSize, gPadding, ICON_CUSTOM_RENAME, gScale))
    {
        ActionRename(state, menu->index);
        result = -2;
    }

    max.x = (float)((int)max.x + buttonSize + gPadding);

    if (GuiIconButtonEx(max, "Delete", gIcons, gIconSize, gPadding, ICON_CUSTOM_TRASH, gScale))
    {
        ActionDelete(state, menu->index);
        result = -2;
        goto end;
    }

    max.x = (float)((int)max.x + buttonSize + gPadding);

    if (menu->index >= 0 && (state->files.formats[menu->index] != &formatFolder))
    {
        if (GuiIconButtonEx(max, "Trim", gIcons, gIconSize, gPadding, ICON_CUSTOM_TRIM, gScale))
        {
            ActionTrim(state, menu->index);
            result = -2;
        }

        max.x = (float)((int)max.x + buttonSize + gPadding);

        if (GuiIconButtonEx(max, "Hex Viewer", gIcons, gIconSize, gPadding, ICON_CUSTOM_HEX, gScale))
        {
            ActionHexEditor(state, menu->index);
            result = -2;
        }

        max.x = (float)((int)max.x + buttonSize + gPadding);
    }

    GuiDisableTooltip();

    if (result == -1)
        return 0;

    if (result == -2)
        goto end;

    if (menu->mode == 0)
        result = ContextMenuOnAction(state, menu, result);
    else
        result = ContextMenuOnFormat(state, menu, result);

end:
    if (result != 0)
        CloseContextMenu(state, menu);
    
    return 1;
}

ExplorerOverlay OpenContextMenu(ExplorerState* state, int index)
{
    ExplorerOverlay overlay = { NULL, NULL };
    ContextMenuState* menu = malloc(sizeof(ContextMenuState));

    if (menu == NULL)
        return overlay;

    menu->actions = LoadContextActions(state);
    menu->position = GetMousePosition();
    menu->mode = 0;
    menu->scroll = 0;
    menu->index = index;

    overlay.Draw = &GuiContextMenu;
    overlay.data = menu;
    return overlay;
}

/**************************************************************************************************
* DriveMenu functions
*/

void CloseDriveSelectMenu(ExplorerState* state, DriveSelectMenuState* menu)
{
    if (state->overlay.data == menu)
    {
        state->overlay.Draw = NULL;
        state->overlay.data = NULL;
    }

    UnloadLogicalDriveFiles(menu->drives);
    free(menu);
}

int GuiDriveSelectMenu(ExplorerState* state, DriveSelectMenuState* menu, Rectangle bounds)
{
    const int iconSize = (int)(gIconSize * gScale);
    const int buttonSize = iconSize + gPadding * 2;
    int result;

    GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, buttonSize);

    result = GuiListMenu(menu->position, menu->drives.paths, menu->drives.count, &menu->scroll);

    if (result == -1)
    {
        return 0;
    }
    
    if (result != -2)
    {
        int len = (int)strlen(menu->drives.paths[result]);
        memcpy(state->path, menu->drives.paths[result], len);
        state->path[len - 1] = 0;
        ExplorerReload(state);
    }

    CloseDriveSelectMenu(state, menu);
    return result != -2;
}

ExplorerOverlay OpenDriveSelectMenu()
{
    ExplorerOverlay overlay = { NULL, NULL };
    DriveSelectMenuState* menu = malloc(sizeof(DriveSelectMenuState));

    if (menu == NULL)
        return overlay;

    menu->drives = LoadLogicalDriveFiles();
    menu->position = GetMousePosition();
    menu->scroll = 0;

    overlay.Draw = &GuiDriveSelectMenu;
    overlay.data = menu;
    return overlay;
}