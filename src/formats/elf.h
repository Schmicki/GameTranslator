#include "file-formats.h"

int IsFormatElf(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return length >= 4 && strncmp(data, ".elf", 4) == 0;
}

static FileFormat formatElf = {
	"Elf File",
	FILE_TYPE_BINARY,
	ICON_CUSTOM_FILE_BINARY,
	&IsFormatElf,
	NULL
};