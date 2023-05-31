#include "file-formats.h"

int IsFormatPng(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".png") == 0;
}

static FileFormat formatPng = {
	"Png Image",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&IsFormatPng,
	NULL
};