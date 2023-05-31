#include "file-formats.h"

int IsFormatOgg(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".ogg") == 0;
}

static FileFormat formatOgg = {
	"Ogg",
	FILE_TYPE_AUDIO,
	ICON_CUSTOM_FILE_AUDIO,
	&IsFormatOgg,
	NULL
};