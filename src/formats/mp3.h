#include "file-formats.h"

int IsFormatMp3(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".mp3") == 0;
}

static FileFormat formatMp3 = {
	"Mp3 Audio File",
	FILE_TYPE_AUDIO,
	ICON_CUSTOM_FILE_AUDIO,
	&IsFormatMp3,
	NULL
};