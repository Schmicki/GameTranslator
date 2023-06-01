#include "platform/filesystem.h"
#include "tools/image-viewer.h"

#include "Bmp.h"

int IsFormatBmp(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return extension && strcmp(extension, ".bmp") == 0;
}

Image FormatBmpLoadImage(const char* src)
{
    return LoadImage(src);
}

static void FortmatBmpViewImage(FileManagerState* state, int index)
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

	img = FormatBmpLoadImage(path);
	AddActivity(OpenImageViewer(GetFileName(path), img), gActivityCount);
	UnloadImage(img);
	gCurrentActivity = gActivityCount - 1;
}

static void FormatBmpGetActions(ContextActionList* actions)
{
	actions->names[actions->count] = memcpy(malloc(13), "Image Viewer", 13);
	actions->functions[actions->count] = &FortmatBmpViewImage;
	actions->count++;
}

/*************************************************************************************************/

FileFormat formatBmp = {
	"Bmp Image",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&IsFormatBmp,
	&FormatBmpGetActions
};