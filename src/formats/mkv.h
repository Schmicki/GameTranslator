#include "file-formats.h"

int IsFormatMkv(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".mkv") == 0;
}

static FileFormat formatMkv = {
	"Mkv Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatMkv,
	NULL
};