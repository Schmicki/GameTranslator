#include "platform/filesystem.h"
#include "tools/image-viewer.h"

#include "jpg.h"

int IsFormatJpg(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".jpg") == 0;
}

Image FormatJpgLoadImage(const char* src)
{
    return LoadImage(src);
}

static void FortmatJpgViewImage(FileManagerState* state, int index)
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

	img = FormatJpgLoadImage(path);
	AddActivity(OpenImageViewer(GetFileName(path), img), gActivityCount);
	UnloadImage(img);
	gCurrentActivity = gActivityCount - 1;
}

static void FormatJpgGetActions(ContextActionList* actions)
{
	actions->names[actions->count] = memcpy(malloc(13), "Image Viewer", 13);
	actions->functions[actions->count] = &FortmatJpgViewImage;
	actions->count++;
}

/*************************************************************************************************/

FileFormat formatJpg = {
	"Jpg Image",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&IsFormatJpg,
	&FormatJpgGetActions
};