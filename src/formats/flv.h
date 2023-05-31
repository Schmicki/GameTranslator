#include "file-formats.h"

int IsFormatFlv(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".flv") == 0;
}

static FileFormat formatFlv = {
	"Flv Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatFlv,
	NULL
};