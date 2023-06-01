#include "platform/filesystem.h"
#include "tools/image-viewer.h"

#include "png.h"

int IsFormatPng(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".png") == 0;
}

Image FormatPngLoadImage(const char* src)
{
    return LoadImage(src);
}

static void FortmatPngViewImage(FileManagerState* state, int index)
{
	FileFormat* format = NULL;
	const char* path = NULL;
	Image img;

	int i = index;

	if (i != -1 && state->formats[i]->type != FILE_TYPE_FOLDER)
	{
		format = state->formats[i];
		path = state->files.paths[i];
	}

	if (format == NULL || format->type != FILE_TYPE_IMAGE)
		return;

	img = FormatPngLoadImage(path);
	AddActivity(OpenImageViewer(GetFileName(path), img), gActivityCount);
	UnloadImage(img);
	gCurrentActivity = gActivityCount - 1;
}

static void FormatPngGetActions(ContextActionList* actions)
{
	actions->names[actions->count] = memcpy(malloc(13), "Image Viewer", 13);
	actions->functions[actions->count] = &FortmatPngViewImage;
	actions->count++;
}

/*************************************************************************************************/

FileFormat formatPng = {
	"Png Image",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&IsFormatPng,
	&FormatPngGetActions
};