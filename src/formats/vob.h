#include "file-formats.h"

int IsFormatVob(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".vob") == 0;
}

static FileFormat formatVob = {
	"Vob Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatVob,
	NULL
};