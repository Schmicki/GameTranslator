#include "image-viewer.h"

/*************************************************************************************************/

typedef struct ImageViewerState
{
    Texture2D texture;
} ImageViewerState;

/*************************************************************************************************/

void CloseImageViewer(Activity* activity)
{
	ImageViewerState* state = (ImageViewerState*)activity->data;

	UnloadTexture(state->texture);
}

static void ImageViewerSetStyleDefault(int iconSize)
{
	GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
	GuiSetStyle(TEXTBOX, BORDER_COLOR_DISABLED, 0xFFFFFFFF);

	GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xFFFFFFFF);
	GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
	GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0xccecffFF);
	GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xccecffFF);
	GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x8bd1fcFF);
	GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0x8bd1fcFF);

	GuiSetStyle(LISTVIEW, BORDER_COLOR_NORMAL, 0xFFFFFF00);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_FOCUSED, 0xFFFFFF00);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_PRESSED, 0xFFFFFF00);
	GuiSetStyle(LISTVIEW, BORDER_COLOR_DISABLED, 0xFFFFFF00);

	GuiSetStyle(DEFAULT, TEXT_SIZE, iconSize);
	GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x00000000);
	GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0x00000000);
	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xFFFFFFFF);
}

static void DrawImageViewer(Activity* activity, Rectangle bounds)
{
	const int textSize = (int)(gIconSize * gScale);
	const int height = textSize + gPadding * 2;
	Vector2 pos;

	ImageViewerState* viewer = (ImageViewerState*)activity->data;

	BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);

	/* Draw Background */

	DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, BLACK);
	
	/* Draw Texture */

	pos.x = (float)((int)bounds.x + (int)bounds.width / 2 - ((int)(viewer->texture.width) / 2));
	pos.y = (float)((int)bounds.y + (int)bounds.height / 2 - ((int)(viewer->texture.height) / 2));

	DrawTextureEx(viewer->texture, pos, 0.0f, 1.0f, RAYWHITE);

	EndScissorMode();
}

Activity OpenImageViewer(const char* name, Image image)
{
	ImageViewerState* viewer;
	Activity activity = { NULL, NULL, NULL, NULL };
	int length;

	if (image.data == NULL)
		return activity;

	/* Image Viewer Setup */

	viewer = malloc(sizeof(ImageViewerState));

	if (viewer == NULL)
		return activity;

	/* Load Texture */

	viewer->texture = LoadTextureFromImage(image);

	if (viewer->texture.id == 0)
		goto cleanViewer;

	/* Activity Setup */

	length = (int)strlen(name) + 1;
	activity.name = malloc(length);

	if (activity.name == NULL)
		goto cleanTexture;

	memcpy((void*)activity.name, name, length);

	activity.data = viewer;
	activity.Draw = &DrawImageViewer;
	activity.Close = &CloseImageViewer;

	return activity;

cleanTexture:
	UnloadTexture(viewer->texture);

cleanViewer:
	free(viewer);
	return activity;
}