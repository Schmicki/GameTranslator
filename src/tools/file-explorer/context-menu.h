#pragma once

#include "file-explorer.h"

/*************************************************************************************************/

/* Return value is -1 if nothing clicked and -2 if closing is requested. */
int GuiListMenu(Vector2 position, const char** names, int count, int* scroll);

Overlay OpenContextMenu(FileManagerState* state, int index);

Overlay OpenDriveSelectMenu();