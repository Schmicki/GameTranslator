#include "gzip.h"
#include "platform/filesystem.h"
#include "saekano-data.h"

/*************************************************************************************************/

/*
* From: https://wiki.xentax.com/index.php/Boku_wa_Tomodachi_ga_Sukunai_Portable_DAT
* 
* Format Specifications
* 
* // little endian
* 
* //header
* 4 bytes (char) - signature // "GPDA"
* 4 bytes (uint32) - archive size
* 4 bytes - nulls
* 4 bytes (uint32) - number of files
* 
* num_of_files *
* {
*    4 bytes (uint32) - file offset
*    4 bytes (uint32) - gzip flag
*    4 bytes (uint32) - file size
*    4 bytes (uint32) - filename offset
* }
* 
* num_of_files *
* {
*    4 bytes (uint32) - filename length
*    x bytes (char) - filename  // e.g. "image_block.dat"
* }
*  
* x bytes - nulls
* 
* num_of_files *
* {
*    x bytes - file data
* }
*/

typedef struct SaekanoDatHeader
{
	unsigned int signature;
	unsigned int size;
	unsigned int pad;
	unsigned int fileCount;
} SaekanoDatHeader;

typedef struct SaekanoDatFileInfo
{
	unsigned int offset;
	unsigned int flags;
	unsigned int size;
	unsigned int name;
} SaekanoDatFileInfo;

/*************************************************************************************************/

int FormatIsSaekanoData(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return length >= 4 && strncmp(data, "GPDA", 4) == 0;
}

int FormatSaekanoDataUnpack(const char* src, int offset, char* dst)
{
	printf("Archive: %s Unpack To: %s\n", src, dst);

	char* entry = (char*)dst + strlen(dst);
	memcpy(entry++, "/", 2);

	FILE* f = fopen(src, "rb");
	
	if (f == NULL)
	{
		printf("failed to open file: %s\n", src);
		return 0;
	}

    fseek(f, offset, SEEK_SET);

	SaekanoDatHeader header;
	int bytesRead = (int)fread(&header, 1, sizeof(header), f);

	if (bytesRead < sizeof(SaekanoDatHeader))
	{
		printf("failed to read file header\n");
		return 0;
	}

	for (int i = 0; i < (int)header.fileCount; i++)
	{
		SaekanoDatFileInfo info;
		int size;

		/* read file info */
		fseek(f, sizeof(SaekanoDatHeader) + i * sizeof(SaekanoDatFileInfo), SEEK_SET);
		bytesRead = (int)fread(&info, 1, sizeof(SaekanoDatFileInfo), f);

		if (bytesRead < sizeof(SaekanoDatFileInfo))
		{
			printf("failed to read file info: %d\n", i);
			continue;
		}

		/* read file name */
		fseek(f, info.name, SEEK_SET);
		bytesRead = (int)fread(&size, 1, 4, f);

		if (bytesRead < 4)
		{
			printf("failed to read file name length\n");
			continue;
		}

		bytesRead = (int)fread(entry, 1, size, f);

		if (bytesRead < size)
		{
			printf("failed to read file name\n");
			continue;
		}

		*(entry + size) = 0;

		if (info.flags)
		{
			printf("compressed with gzip\n");
			FormatGZipUnpack(src, info.offset, dst);
			continue;
		}

		/* write file */
		if (FileExists(dst))
		{
			printf("file already exists: %s\n", dst);
			continue;
		}

		FILE* fdst = fopen(dst, "wb");

		if (fdst == NULL)
		{
			printf("failed to create file: %s\n", dst);
			continue;
		}

		char* buffer = malloc(0x100000);
		fseek(f, info.offset, SEEK_SET);

		for (int j = 0; j < (int)info.size;)
		{
			int rest = info.size - j;
			size = rest > 0x100000 ? 0x100000 : rest;
			bytesRead = (int)fread(buffer, 1, size, f);

			if (bytesRead < size)
			{
				printf("failed to read data: %d\n", j);
				break;
			}

			bytesRead = (int)fwrite(buffer, 1, size, fdst);

			if (bytesRead < size)
			{
				printf("failed to write data: %d, size: %d\n", j, size);
				break;
			}

			j += size;
		}

		fclose(fdst);
		free(buffer);
	}

	*(entry - 1) = 0;
	fclose(f);
	return 1;
}

void FormatSaekanoDataUnpackHere(ExplorerState* state, int index)
{
	FileFormat* format = NULL;
	const char* path = NULL;

	for (int i = 0; i < (int)state->files.count; i++)
	{
		if (state->selected[i] && state->files.formats[i] == &formatSaekanoData)
		{
			path = state->files.paths[i];
			FormatSaekanoDataUnpack(path, 0, state->path);
		}
	}

	ExplorerReload(state);
}

void FormatSaekanoDataUnpackToFolder(ExplorerState* state, int index)
{
	FileFormat* format = NULL;
	char* path = NULL;
	char* dst;
	char* ext;

	if ((dst = malloc(0x1000)) == NULL)
		return;

	for (int i = 0; i < (int)state->files.count; i++)
	{
		if (state->selected[i] && state->files.formats[i] == &formatSaekanoData)
		{
			int length;

			path = state->files.paths[i];
			length = (int)strlen(path) + 1;
			memcpy(dst, path, length);

			if ((ext = (char*)GetFileExtension(GetFileName(dst))) != NULL)
				*ext = 0;

			if (!GetUniqueFileName(dst, 0x1000))
				continue;

			CreateFolder(dst);
			FormatSaekanoDataUnpack(path, 0, dst);
		}
	}
	
	ExplorerReload(state);
	free(dst);
}

/*************************************************************************************************/

void FormatSaekanoDataGetActions(ContextActionList* actions)
{
	actions->names[actions->count] = memcpy(malloc(12), "Unpack Here", 12);
	actions->functions[actions->count] = &FormatSaekanoDataUnpackHere;
	actions->count++;

	actions->names[actions->count] = memcpy(malloc(17), "Unpack To Folder", 17);
	actions->functions[actions->count] = &FormatSaekanoDataUnpackToFolder;
	actions->count++;
}

/*************************************************************************************************/

FileFormat formatSaekanoData = {
	"Saekano Data File",
	FILE_TYPE_ARCHIVE,
	ICON_CUSTOM_ARCHIVE,
	&FormatIsSaekanoData,
	&FormatSaekanoDataGetActions
};