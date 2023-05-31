#include "file-formats.h"

int IsFormatHtml(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".html") == 0;
}

static FileFormat formatHtml = {
	"Html Text File",
	FILE_TYPE_TEXT,
	ICON_CUSTOM_FILE_DOCUMENT,
	&IsFormatHtml,
	NULL
};