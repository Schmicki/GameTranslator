#include "file-formats.h"

int IsFormatMp4(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".mp4") == 0;
}

static FileFormat formatMp4 = {
	"Mp4 Video File",
	FILE_TYPE_VIDEO,
	ICON_CUSTOM_FILE_VIDEO,
	&IsFormatMp4,
	NULL
};