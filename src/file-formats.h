#pragma once

#include "common.h"

/*************************************************************************************************/

#define MAX_FILE_FORMATS 0x400
#define MAX_CONTEXT_ACTIONS 0x400

#define FILE_TYPE_UNKNOWN 0
#define FILE_TYPE_FOLDER 1
#define FILE_TYPE_ARCHIVE 2
#define FILE_TYPE_TEXT 3
#define FILE_TYPE_TABLE 4
#define FILE_TYPE_AUDIO 5
#define FILE_TYPE_IMAGE 6
#define FILE_TYPE_VIDEO 7
#define FILE_TYPE_BINARY 8

/*************************************************************************************************/

typedef struct FileManagerState FileManagerState;

typedef struct Overlay Overlay;

typedef void (*ContextAction)(FileManagerState* state, int clickedIndex);

/*************************************************************************************************/

typedef struct ContextActionList
{
	int capacity;
	int count;
	ContextAction* functions;
	char** names;
} ContextActionList;

/*************************************************************************************************/

typedef struct FileFormat
{
	const char* name;
	int type;
	int icon;

	/* data is length bytes from the beginning of the file */
	int (*IsFormat)(const char* path, const char* name, const char* extension, const char* data, int length);
	void (*GetActions)(ContextActionList* list);
} FileFormat;

/*************************************************************************************************/

typedef struct FileFormatList
{
	int capacity;
	int count;
	char** names;
	FileFormat** formats;
} FileFormatList;

/*************************************************************************************************/

FileFormat* GetFileFormat(const char* path);

int RegisterFileFormat(FileFormat* format);

FileFormatList LoadFormatList();

void UnloadFormatList(FileFormatList formats);