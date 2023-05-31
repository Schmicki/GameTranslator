#include "file-formats.h"

int IsFormatXlsx(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".xlsx") == 0;
}

static FileFormat formatXlsx = {
	"Xlsx Table File",
	FILE_TYPE_TABLE,
	ICON_CUSTOM_FILE_DOCUMENT,
	&IsFormatXlsx,
	NULL
};