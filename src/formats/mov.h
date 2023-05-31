#include "file-formats.h"

int IsFormatMov(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".mov") == 0;
}

static FileFormat formatMov = {
	"Mov Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatMov,
	NULL
};