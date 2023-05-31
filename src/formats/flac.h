#include "file-formats.h"

int IsFormatFlac(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".flac") == 0;
}

static FileFormat formatFlac = {
	"Flac Audio File",
	FILE_TYPE_AUDIO,
	ICON_CUSTOM_FILE_AUDIO,
	&IsFormatFlac,
	NULL
};