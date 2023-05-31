#include "file-formats.h"

int IsFormatJar(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".jar") == 0;
}

static FileFormat formatJar = {
	"Java Executable",
	FILE_TYPE_ARCHIVE,
	ICON_CUSTOM_ARCHIVE,
	&IsFormatJar,
	NULL
};