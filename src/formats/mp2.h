#include "file-formats.h"

int IsFormatMp2(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".mp2") == 0;
}

static FileFormat formatMp2 = {
	"Mp2 Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatMp2,
	NULL
};