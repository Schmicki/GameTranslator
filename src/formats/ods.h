#include "file-formats.h"

int IsFormatOds(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".ods") == 0;
}

static FileFormat formatOds = {
	"Open Document Text File",
	FILE_TYPE_TEXT,
	ICON_CUSTOM_FILE_DOCUMENT,
	&IsFormatOds,
	NULL
};