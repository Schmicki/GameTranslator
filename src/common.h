#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "raygui-impl.h"
#include "string-tools.h"
#include "style.h"

/*************************************************************************************************/

enum CustomIcons
{
	ICON_CUSTOM_FILE = 0,
	ICON_CUSTOM_FILE_IMAGE,
	ICON_CUSTOM_FILE_AUDIO,
	ICON_CUSTOM_FILE_VIDEO,
	ICON_CUSTOM_FILE_DOCUMENT,
	ICON_CUSTOM_FILE_TABLE,
	ICON_CUSTOM_FILE_BINARY,
	ICON_CUSTOM_FILE_NEW,

	ICON_CUSTOM_FOLDER = 16,
	ICON_CUSTOM_FOLDER_NEW,
	ICON_CUSTOM_FOLDER_DELETE,
	ICON_CUSTOM_FOLDER_IMAGES,
	ICON_CUSTOM_FOLDER_DOCUMENTS,
	ICON_CUSTOM_FOLDER_VIDEOS,
	ICON_CUSTOM_FOLDER_MUSIC,
	ICON_CUSTOM_ARCHIVE,

	ICON_CUSTOM_ARROW_HEAD_LEFT = 32,
	ICON_CUSTOM_ARROW_HEAD_UP,
	ICON_CUSTOM_ARROW_HEAD_DOWN,
	ICON_CUSTOM_ARROW_HEAD_RIGHT,
	ICON_CUSTOM_ARROW_LEFT,
	ICON_CUSTOM_ARROW_UP,
	ICON_CUSTOM_ARROW_DOWN,
	ICON_CUSTOM_ARROW_RIGHT,
	ICON_CUSTOM_UNDO,
	ICON_CUSTOM_REDO,
	ICON_CUSTOM_SYNCHRONIZE,
	ICON_CUSTOM_ROTATE,
	ICON_CUSTOM_PLUS,
	ICON_CUSTOM_MINUS,
	ICON_CUSTOM_MULTIPLY,

	ICON_CUSTOM_PLAYER_PREVIOUS = 64,
	ICON_CUSTOM_PLAYER_BACK,
	ICON_CUSTOM_PLAYER_PAUSE,
	ICON_CUSTOM_PLAYER_PLAY,
	ICON_CUSTOM_PLAYER_FORWARD,
	ICON_CUSTOM_PLAYER_NEXT,
	ICON_CUSTOM_SHUFFLE,
	ICON_CUSTOM_PLAYLIST,
	ICON_CUSTOM_REPEAT,
	ICON_CUSTOM_REPEAT_SINGLE,
	ICON_CUSTOM_VOLUME_OFF,
	ICON_CUSTOM_VOLUME_DOWN,
	ICON_CUSTOM_VOLUME_ON,

	ICON_CUSTOM_CUT = 96,
	ICON_CUSTOM_COPY,
	ICON_CUSTOM_PASTE,
	ICON_CUSTOM_RENAME,
	ICON_CUSTOM_TRASH,
	ICON_CUSTOM_SHARE,
	ICON_CUSTOM_PRINT,
	ICON_CUSTOM_SETTINGS,
	ICON_CUSTOM_SEARCH,
	ICON_CUSTOM_ZOOM_IN,
	ICON_CUSTOM_ZOOM_OUT,
	ICON_CUSTOM_ATTENTION,
	ICON_CUSTOM_INFO,
	ICON_CUSTOM_HEX,
	ICON_CUSTOM_TRIM,
	ICON_CUSTOM_MENU,
	ICON_CUSTOM_SCREEN_NORMAL,
	ICON_CUSTOM_SCREEN_FULL,

	ICON_CUSTOM_STAR = 144,
	ICON_CUSTOM_STAR_OFF,
	ICON_CUSTOM_EYE,
	ICON_CUSTOM_EYE_OFF,
	ICON_CUSTOM_UPLOAD,
	ICON_CUSTOM_DOWNLOAD,
	ICON_CUSTOM_CLOSE,
	ICON_CUSTOM_CHECK,
	ICON_CUSTOM_MUSIC,
	ICON_CUSTOM_MUSIC_ALBUM,
	ICON_CUSTOM_MUSIC_INTERPRET,
	ICON_CUSTOM_CAMERA,
};

/*************************************************************************************************/

#define MAX_ACTIVITIES 0x100

typedef struct Activity
{
	const char* name;
	void* data;
	void (*Draw)(struct Activity* activity, Rectangle bounds);
	void (*Close)(struct Activity* activity);
} Activity;

/**************************************************************************************************
* Globals
*/

extern Texture2D gIcons;

extern int gIconSize;

extern float gScale;

extern int gPadding;

extern Font gFont;

extern Font gMonoFont;

extern Activity gActivities[MAX_ACTIVITIES];

extern int gActivityCount;

extern int gCurrentActivity;

extern const char* gDefaultDirectory;

extern RenderTexture2D gTopLayerTexture;

/**************************************************************************************************
* Activity functions
*/

int AddActivity(Activity activity, int index);

void CloseActivity(int index);