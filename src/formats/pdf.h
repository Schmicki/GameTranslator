#include "file-formats.h"

int IsFormatPdf(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".pdf") == 0;
}

static FileFormat formatPdf = {
	"Portable Document Format",
	FILE_TYPE_TEXT,
	ICON_CUSTOM_FILE_DOCUMENT,
	&IsFormatPdf,
	NULL
};