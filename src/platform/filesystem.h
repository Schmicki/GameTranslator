#pragma once

#include "common.h"

/**************************************************************************************************
* Functions
*/

FilePathList LoadLogicalDriveFiles(void);

void UnloadLogicalDriveFiles(FilePathList files);

int GetAbsolutePath(const char* relativePath, int bufferSize, char* buffer);

/* The buffer should at least be 5 bytes larger that the path. */
int GetUniqueFileName(char* path, int bufferSize);

/*
* Folder functions
*/

int CreateFolder(const char* path);

int CreateFolderRecursive(const char* path);

int DeleteFolder(const char* path);

int CopyFolder(const char* src, const char* dst);

int MoveFolder(const char* src, const char* dst);

void ForFilesInFolder(const char* path, void (*callback)(const char* path, void* data), void* data);

void ForFilesInFolderRecursive(const char* path, void (*callback)(const char* path, void* data),
	void* data);

/*
* File functions
*/

int CreateFile(const char* path);

int DeleteFile(const char* path);

int CopyFile(const char* src, const char* dst);

int MoveFile(const char* src, const char* dst);
