#include "file-formats.h"

int IsFormatGif(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".gif") == 0;
}

static FileFormat formatGif = {
	"Gif File",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&IsFormatGif,
	NULL
};
