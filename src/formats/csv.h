#include "file-formats.h"

int IsFormatCsv(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".csv") == 0;
}

static FileFormat formatCsv = {
	"Csv Text File",
	FILE_TYPE_TEXT,
	ICON_CUSTOM_FILE_DOCUMENT,
	&IsFormatCsv,
	NULL
};