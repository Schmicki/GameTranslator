#include "file-formats.h"

int IsFormatAac(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".aac") == 0;
}

static FileFormat formatAac = {
	"Aac Audio File",
	FILE_TYPE_AUDIO,
	ICON_CUSTOM_FILE_AUDIO,
	&IsFormatAac,
	NULL
};