#include "file-formats.h"

int IsFormat7Zip(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".zip") == 0;
}

static FileFormat format7Zip = {
	"7Zip Archive",
	FILE_TYPE_ARCHIVE,
	ICON_CUSTOM_ARCHIVE,
	&IsFormat7Zip,
	NULL
};