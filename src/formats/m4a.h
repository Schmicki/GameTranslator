#include "file-formats.h"

int IsFormatM4a(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".m4a") == 0;
}

static FileFormat formatM4a = {
	"M4a Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatM4a,
	NULL
};