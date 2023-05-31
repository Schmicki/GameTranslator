#include "file-formats.h"

int IsFormatWav(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".wav") == 0;
}

static FileFormat formatWav = {
	"Wav Audio File",
	FILE_TYPE_AUDIO,
	ICON_CUSTOM_FILE_AUDIO,
	&IsFormatWav,
	NULL
};