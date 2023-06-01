#include "platform/filesystem.h"
#include "tools/image-viewer.h"

#include "Gif.h"

int IsFormatGif(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".gif") == 0;
}

Image FormatGifLoadImage(const char* src)
{
    return LoadImage(src);
}

static void FortmatGifViewImage(FileManagerState* state, int index)
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

	img = FormatGifLoadImage(path);
	AddActivity(OpenImageViewer(GetFileName(path), img), gActivityCount);
	UnloadImage(img);
	gCurrentActivity = gActivityCount - 1;
}

static void FormatGifGetActions(ContextActionList* actions)
{
	actions->names[actions->count] = memcpy(malloc(13), "Image Viewer", 13);
	actions->functions[actions->count] = &FortmatGifViewImage;
	actions->count++;
}

/*************************************************************************************************/

FileFormat formatGif = {
	"Gif Image",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&IsFormatGif,
	&FormatGifGetActions
};