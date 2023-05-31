#include "file-formats.h"

int IsFormatXls(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".xls") == 0;
}

static FileFormat formatXls = {
	"Xls Table File",
	FILE_TYPE_TABLE,
	ICON_CUSTOM_FILE_DOCUMENT,
	&IsFormatXls,
	NULL
};