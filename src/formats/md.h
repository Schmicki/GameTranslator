#include "file-formats.h"

int IsFormatMd(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".md") == 0;
}

static FileFormat formatMd = {
	"Md Text File",
	FILE_TYPE_TEXT,
	ICON_CUSTOM_FILE_DOCUMENT,
	&IsFormatMd,
	NULL
};