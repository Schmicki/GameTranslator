#include "file-formats.h"

int IsFormatExe(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension != NULL && strcmp(extension, ".exe") == 0;
}

static FileFormat formatExe = {
	"Windows Executable",
	FILE_TYPE_BINARY,
	ICON_CUSTOM_FILE_BINARY,
	&IsFormatExe,
	NULL
};