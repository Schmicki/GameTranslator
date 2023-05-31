#include "file-formats.h"

int IsFormatWebm(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".webm") == 0;
}

static FileFormat formatWebm = {
	"Webm Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatWebm,
	NULL
};