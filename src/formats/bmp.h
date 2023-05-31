#include "file-formats.h"

int IsFormatBmp(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".rar") == 0;
}

static FileFormat formatBmp = {
	"Bmp Image",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&IsFormatBmp,
	NULL
};