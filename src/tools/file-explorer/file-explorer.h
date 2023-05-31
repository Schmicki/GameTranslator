#pragma once

#include "common.h"
#include "file-formats.h"

/*************************************************************************************************/

struct Overlay
{
	/*
	* This function should return TRUR if the overlay is closed, but you dont want the file manager
	* to receive input this frame.
	*/
	int (*Draw)(FileManagerState* state, void* data, Rectangle bounds);
	void* data;
};

/*************************************************************************************************/

struct FileManagerState
{
	char* path;					/* Current directory path */
	Vector2 scroll;				/* File viwer scroll position */
	FilePathList files;			/* List of files in current directory */
	FileFormat** formats;		/* List of files formats */
	double* selected;
	int lastSelected;

	int renameIndex;
	char* rename;

	int copyFlags;
	char* copyTarget;

	Overlay editor;				/* Active editor */
	Overlay overlay;			/* Active overlay */
};

/*************************************************************************************************/

void ReloadFilesAndTypes(FileManagerState* state);

Activity OpenFileManagerActivity(const char* path);

int FileManagerGetSelectedFile(FileManagerState* state);

void FileManagerBeginRename(FileManagerState* state, int index);

void FileManagerEndRename(FileManagerState* state);