#pragma once

#include "common.h"
#include "file-formats.h"
#include "platform/threadsafe.h"

#define MAX_PATH_LENGTH 1024

/**************************************************************************************************
* Async file system crawling
*/

typedef struct FileList
{
	int capacity;
	int count;
	char** paths;
	FileFormat** formats;
} FileList;

typedef struct FileNode
{
	struct FileNode* next;
	int generation;
	char* path;
	FileFormat* format;
} FileNode;

typedef struct ExplorerOverlay
{
	/*
	* This function should return TRUE if the overlay is closed, but you dont want the file manager
	* to receive input this frame.
	*/
	int (*Draw)(struct ExplorerState* state, void* data, Rectangle bounds);
	void* data;
} ExplorerOverlay;

/*************************************************************************************************/

typedef struct CrawlerState
{
	THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE)
	int generation;
	char path[MAX_PATH_LENGTH];
	char filter[MAX_PATH_LENGTH];

	THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE)
	int futex; /* atomic */

	THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE)
	int nextLock; /* atomic */
	int nextGeneration;
	char nextPath[MAX_PATH_LENGTH];
	char nextFilter[MAX_PATH_LENGTH];

	THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE)
	int queueLock; /* atomic */
	FileNode* first;
	FileNode* last;
} CrawlerState;

typedef struct ExplorerState
{
	int generation;
	char path[MAX_PATH_LENGTH];
	char filter[MAX_PATH_LENGTH];
	char reName[MAX_PATH_LENGTH];
	char copyTarget[MAX_PATH_LENGTH];

	FileList files;
	double* selected;
	int lastSelected;

	int showFilter;
	int editFilter;
	int renameIndex;
	int copyFlags;
	
	ExplorerOverlay overlay;
	Vector2 scroll;

	CrawlerState* crawler;
	Thread crawlerThread;
} ExplorerState;

/*************************************************************************************************/

Activity OpenExplorer(const char* path);

void ExplorerReload(ExplorerState* state);

int ExplorerGetSelectedFile(ExplorerState* state);

void ExplorerBeginRename(ExplorerState* state, int index);

void ExplorerEndRename(ExplorerState* state);