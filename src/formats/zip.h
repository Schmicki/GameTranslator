#include "file-formats.h"

int FormatIsZip(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".zip") == 0;
}

void ZipUnpack(const char* src, const char* dst)
{

}

static FileFormat formatZip = {
	"Zip Archive",
	FILE_TYPE_ARCHIVE,
	ICON_CUSTOM_ARCHIVE,
	&FormatIsZip,
	NULL
};