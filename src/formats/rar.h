#include "file-formats.h"

int IsFormatRar(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && (strcmp(extension, ".rar") || strcmp(extension, ".RAR")) == 0;
}

static FileFormat formatRar = {
	"Rar",
	FILE_TYPE_ARCHIVE,
	ICON_CUSTOM_ARCHIVE,
	&IsFormatRar,
	NULL
};