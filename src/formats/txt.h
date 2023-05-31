#include "file-formats.h"

int IsFormatTxt(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && (strcmp(extension, ".txt") == 0 ||
	strcmp(extension, ".h") == 0 ||
	strcmp(extension, ".hpp") == 0 ||
	strcmp(extension, ".c") == 0 ||
	strcmp(extension, ".cc") == 0 ||
	strcmp(extension, ".cpp") == 0 ||
	strcmp(extension, ".py") == 0 ||
	strcmp(extension, ".java") == 0 ||
	strcmp(extension, ".rs") == 0 ||
	strcmp(extension, ".lua") == 0);
}

static FileFormat formatTxt = {
	"Txt Text File",
	FILE_TYPE_TEXT,
	ICON_CUSTOM_FILE_DOCUMENT,
	&IsFormatTxt,
	NULL
};