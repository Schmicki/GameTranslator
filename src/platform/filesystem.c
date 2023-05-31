#include "filesystem.h"

/**************************************************************************************************
* Macros
*/

#define WIN32_LEAN_AND_MEAN 1
#define NOGDI 1
#define NOUSER 1
#define NOGDICAPMASKS      1
#define NOVIRTUALKEYCODES  1
#define NOWINMESSAGES      1
#define NOWINSTYLES        1
#define NOSYSMETRICS       1
#define NOMENUS            1
#define NOICONS            1
#define NOKEYSTATES        1
#define NOSYSCOMMANDS      1
#define NORASTEROPS        1
#define NOSHOWWINDOW       1
#define OEMRESOURCE        1
#define NOATOM             1
#define NOCLIPBOARD        1
#define NOCOLOR            1
#define NOCTLMGR           1
#define NODRAWTEXT         1
#define NOGDI              1
#define NOKERNEL           1
#define NOUSER             1
#define NONLS              1
#define NOMB               1
#define NOMEMMGR           1
#define NOMETAFILE         1
#define NOMINMAX           1
#define NOMSG              1
#define NOOPENFILE         1
#define NOSCROLL           1
#define NOSERVICE          1
#define NOSOUND            1
#define NOTEXTMETRIC       1
#define NOWH               1
#define NOWINOFFSETS       1
#define NOCOMM             1
#define NOKANJI            1
#define NOHELP             1
#define NOPROFILER         1
#define NODEFERWINDOWPOS   1
#define NOMCX              1
#include <Windows.h>

#ifdef CreateDirectory

#undef CreateDirectory

#endif

#ifdef CreateFile

#undef CreateFile

#endif

#ifdef DeleteFile

#undef DeleteFile

#endif

#ifdef CopyFile
#undef CopyFile
#endif

#ifdef MoveFile
#undef MoveFile
#endif

/**************************************************************************************************
* Functions
*/

FilePathList LoadLogicalDriveFiles(void)
{
	FilePathList files;
	int size;
	char* paths;
	int count;
	char* current;

	size = (int)GetLogicalDriveStringsA(0, NULL);
	paths = malloc(size);
	GetLogicalDriveStringsA(size--, paths);

	// count logical drive paths
	for (current = paths, count = 0; (int)(current - paths) < size; count++)
		current += strlen(current) + 1;

	// allocate file path list
	files = (FilePathList){ count, count, malloc(sizeof(char*) * count) };

	// copy path names into list
	for (current = paths, count = 0; (int)(current - paths) < size; count++)
	{
		int len = (int)strlen(current) + 1;
		files.paths[count] = malloc(len);
		memcpy(files.paths[count], current, len);
		current += len;
	}

	free(paths);
	return  files;
}

void UnloadLogicalDriveFiles(FilePathList files)
{
	UnloadDirectoryFiles(files);
}

int GetAbsolutePath(const char* relativePath, int bufferSize, char* buffer)
{
	int len = (int)GetFullPathNameA(relativePath, bufferSize, buffer, NULL);

	if (len < bufferSize && buffer[len - 1] == '\\')
		buffer[len - 1] = 0;

	return len;
}

int GetUniqueFileName(char* path, int bufferSize)
{
	int length = (int)strlen(path) + 1;
	int maxDigits = bufferSize - length;
	char* extra = path + (length - 1);

	if (!FileExists(path))
		return 1;

	for (int i = 0, d = 0; i < maxDigits; i++)
	{
		extra[d++] = '1';
		while (d <= i) extra[d++] = '0';
		extra[d--] = 0;

test:
		if (!FileExists(path))
			return 1;

next:
		if (extra[d]++ == '9')
		{
			extra[d] = '0';
			
			if (d-- == 0)
			{
				d = 0;
				continue;
			}

			goto next;
		}

		d = i;
		goto test;
	}

	return 0;
}

/**************************************************************************************************
* Folder helper functions
*/

static void DeleteFolderCallback(const char* path, void* data)
{
	if (IsPathFile(path))
		DeleteFile(path);
	else
		RemoveDirectoryA(path);
}

static void CopyFolderCallback(const char* path, void* data)
{
	char* dst = *((char**)data)++;
	int srclen = *((int*)data)++;
	int dstlen = *(int*)data;
	int pthlen = (int)strlen(path) + 1;
	char* nameMinusOne = dst + dstlen + pthlen - srclen;
	char c;

	memcpy(dst + dstlen, path + srclen, pthlen - srclen);

	while (nameMinusOne >= dst)
	{
		if (*nameMinusOne == '/' || *nameMinusOne == '\\')
		{
			c = *nameMinusOne;
			goto copy;
		}

		nameMinusOne--;
	}

	return;

copy:
	if (IsPathFile(path))
	{
		*nameMinusOne = 0;
		CreateFolderRecursive(dst);
		*nameMinusOne = c;
		CopyFile(path, dst);
	}
}

static void MoveFolderCallback(const char* path, void* data)
{
	char* dst = *((char**)data)++;
	int srclen = *((int*)data)++;
	int dstlen = *(int*)data;
	int pthlen = (int)strlen(path) + 1;
	char* nameMinusOne = dst + dstlen + pthlen - srclen;
	char c;

	memcpy(dst + dstlen, path + srclen, pthlen - srclen);

	while (nameMinusOne >= dst)
	{
		if (*nameMinusOne == '/' || *nameMinusOne == '\\')
		{
			c = *nameMinusOne;
			goto move;
		}

		nameMinusOne--;
	}

	return;

move:
	if (IsPathFile(path))
	{
		*nameMinusOne = 0;
		CreateFolderRecursive(dst);
		*nameMinusOne = c;
		CopyFile(path, dst);
	}
	else
	{
		DeleteFolder(path);
	}
}

/**************************************************************************************************
* Folder functions
*/

int CreateFolder(const char* path)
{
	return (int)CreateDirectoryA(path, NULL);
}

int CreateFolderRecursive(const char* path)
{
	char* tmp;
	int length;
	char* end;

	if (DirectoryExists(path))
		return 0;

	tmp = malloc(0x1000);
	length = (int)strlen(path);
	
	if (tmp == NULL)
		return 0;

	memcpy(tmp, path, length + 1);
	end = tmp + length;

	while (end >= tmp)
	{
		if (*end == '/' || *end == '\\')
		{
			*end = 0;

			if (DirectoryExists(tmp))
				goto create;
		}

		end--;
	}

	free(tmp);
	return 0;

create:
	do
	{
		*end = '/';
		end += strlen(end);
		CreateFolder(tmp);
	}
	while ((end - tmp) < length);

	free(tmp);
	return 1;
}

int DeleteFolder(const char* path)
{
	ForFilesInFolderRecursive(path, &DeleteFolderCallback, NULL);
	DeleteFolderCallback(path, NULL);
	return !DirectoryExists(path);
}

int CopyFolder(const char* src, const char* dst)
{
	struct Data
	{
		char* dst;
		int srclen;
		int dstlen;
	} data;

	data.dst = malloc(0x1000);

	if (data.dst == NULL)
		return 0;

	data.srclen = (int)strlen(src) + 1;
	data.dstlen = (int)strlen(dst) + 1;

	memcpy(data.dst, dst, data.dstlen);
	data.dst[data.dstlen - 1] = '/';

	ForFilesInFolderRecursive(src, &CopyFolderCallback, &data);

	free(data.dst);

	return 1;
}

int MoveFolder(const char* src, const char* dst)
{
	struct Data
	{
		char* dst;
		int srclen;
		int dstlen;
	} data;

	data.dst = malloc(0x1000);

	if (data.dst == NULL)
		return 0;

	data.srclen = (int)strlen(src) + 1;
	data.dstlen = (int)strlen(dst) + 1;

	memcpy(data.dst, dst, data.dstlen);
	data.dst[data.dstlen - 1] = '/';

	ForFilesInFolderRecursive(src, &MoveFolderCallback, &data);
	DeleteFolder(src);

	free(data.dst);

	return 1;
}

void ForFilesInFolder(const char* path, void(*callback)(const char* path, void* data), void* data)
{
	FilePathList files = LoadDirectoryFiles(path);

	for (int i = 0; i < (int)files.count; i++)
	{
		callback(files.paths[i], data);
	}

	UnloadDirectoryFiles(files);
}

void ForFilesInFolderRecursive(const char* path, void(*callback)(const char* path, void* data), void* data)
{
	FilePathList files = LoadDirectoryFiles(path);

	for (int i = 0; i < (int)files.count; i++)
	{
		if (!IsPathFile(files.paths[i]))
		{
			ForFilesInFolderRecursive(files.paths[i], callback, data);
		}

		callback(files.paths[i], data);
	}

	UnloadDirectoryFiles(files);
}

/**************************************************************************************************
* File functions
*/

int CreateFile(const char* path)
{
	FILE* file = fopen(path, "wab");

	if (file)
		fclose(file);

	return file != NULL;
}

int DeleteFile(const char* path)
{
	return (int)DeleteFileA(path);
}

int CopyFile(const char* src, const char* dst)
{
	return (int)CopyFileA(src, dst, TRUE);
}

int MoveFile(const char* src, const char* dst)
{
	return (int)MoveFileA(src, dst);
}
