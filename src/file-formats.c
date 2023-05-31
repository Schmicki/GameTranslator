#include "platform/filesystem.h"

#include "file-formats.h"

/* Special Formats */

#include "formats/unknown.h"
#include "formats/folder.h"
#include "formats/default.h"

/* Saekano Game Formats */

#include "formats/saekano-data.h"
#include "formats/saekano-script.h"

/* Archive Formats */

#include "formats/rar.h"
#include "formats/jar.h"
#include "formats/zip.h"
#include "formats/7zip.h"
#include "formats/gzip.h"

/* Image Formats */

#include "formats/jpg.h"
#include "formats/png.h"
#include "formats/bmp.h"
#include "formats/gif.h"
#include "formats/vitagxt.h"

/* Audio Formats */

#include "formats/mp3.h"
#include "formats/ogg.h"
#include "formats/wav.h"
#include "formats/m4a.h"
#include "formats/aac.h"
#include "formats/flac.h"

/* Video Formats */

#include "formats/mp4.h"
#include "formats/avi.h"
#include "formats/mkv.h"
#include "formats/m4v.h"
#include "formats/flv.h"
#include "formats/mov.h"
#include "formats/vob.h"
#include "formats/webm.h"

/* Binary Formats */

#include "formats/exe.h"
#include "formats/elf.h"

/* Text Formats */

#include "formats/txt.h"
#include "formats/md.h"
#include "formats/html.h"
#include "formats/csv.h"
#include "formats/ods.h"
#include "formats/pdf.h"

/* Table Formats */

#include "formats/xls.h"
#include "formats/xlsx.h"

static FileFormat* formats[MAX_FILE_FORMATS] = {

	/* Special Formats */
	
	&formatDefault,
	&formatUnknown,
	&formatFolder,

	/* Saekano Game Formats */

	&formatSaekanoData,
	&formatSaekanoScript,

	/* Archive Formats */

	&formatRar,
	&formatJar,
	&formatZip,
	&format7Zip,
	&formatGZip,

	/* Image Formats */

	&formatJpg,
	&formatPng,
	&formatBmp,
	&formatGif,
	&formatVitaGxt,

	/* Audio Formats */

	&formatMp3,
	&formatOgg,
	&formatWav,
	&formatM4a,
	&formatAac,
	&formatFlac,

	/* Video Formats */

	&formatMp4,
	&formatAvi,
	&formatMkv,
	&formatM4v,
	&formatFlv,
	&formatMov,
	&formatVob,
	&formatWebm,

	/* Binary Formats */

	&formatExe,
	&formatElf,

	/* Text Formats */

	&formatTxt,
	&formatMd,
	&formatHtml,
	&formatCsv,
	&formatOds,
	&formatPdf,

	/* Table Formats */

	&formatXls,
	&formatXlsx
};

static int formatCount = 38;

/*************************************************************************************************/

FileFormat* GetFileFormat(const char* path)
{
	if (DirectoryExists(path))
		return &formatFolder;

	const char* name = GetFileName(path);
	const char* extension = GetFileExtension(name);

	FILE* file;
	char data[0x100];
	int size = 0;

	if ((file = fopen(path, "rb")) != NULL)
	{
		size = (int)fread(data, 1, 0x100, file);
		fclose(file);
	}

	for (int i = 0; i < formatCount; i++)
	{
		FileFormat* format = formats[i];

		if (format->IsFormat && format->IsFormat(path, name, extension, data, size))
			return format;
	}

	return &formatUnknown;
}

int RegisterFileFormat(FileFormat* format)
{
	int hasSpace = formatCount < MAX_FILE_FORMATS;
	if (hasSpace)
		formats[formatCount++] = format;

	return hasSpace;
}

FileFormatList LoadFormatList()
{
	FileFormatList list = { 0, 0, NULL, NULL };

	if (formatCount == 0)
		return list;

	if ((list.names = malloc(sizeof(char*) * MAX_FILE_FORMATS)) == NULL)
		return list;

	if ((list.formats = malloc(sizeof(FileFormat*) * MAX_FILE_FORMATS)) == NULL)
		goto cleanupNames;

	list.capacity = MAX_FILE_FORMATS;

	for (int i = 0; i < formatCount; i++)
	{
		char* name;
		int size;
		FileFormat* format = formats[i];

		if (format->GetActions == NULL)
			continue;
		
		size = (int)strlen(format->name) + 1;
		
		if ((name = malloc(size)) == NULL)
			goto fail;

		memcpy(name, format->name, size);

		list.names[list.count] = name;
		list.formats[list.count] = format;
		list.count++;
	}

	return list;

fail:
	for (int i = 0; i < list.count; i++)
	{
		free(list.names[i]);
	}

	free(list.formats);

cleanupNames:
	free(list.names);

	list.capacity = 0;
	list.count = 0;
	list.names = NULL;
	list.formats = NULL;

	return list;
}

void UnloadFormatList(FileFormatList formats)
{
	if (formats.capacity)
	{
		for (int i = 0; i < formats.count; i++)
		{
			free(formats.names[i]);
		}

		free(formats.formats);
		free(formats.names);
	}
}
