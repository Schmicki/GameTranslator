#include "file-formats.h"

int IsFormatM4v(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".m4v") == 0;
}

static FileFormat formatM4v = {
	"M4v Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatM4v,
	NULL
};