#include "file-formats.h"

int IsFormatAvi(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".avi") == 0;
}

static FileFormat formatAvi = {
	"Avi Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatAvi,
	NULL
};