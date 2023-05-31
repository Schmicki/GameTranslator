#include "file-formats.h"

int IsFormatJpg(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".jpg") == 0;
}

static FileFormat formatJpg = {
	"Jpg Image",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&IsFormatJpg,
	NULL
};