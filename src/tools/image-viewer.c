#include "image-viewer.h"

/*************************************************************************************************/

typedef struct ImageViewerState
{
    Texture2D texture;
	float scale;
} ImageViewerState;

/*************************************************************************************************/

void CloseImageViewer(Activity* activity)
{
	ImageViewerState* state = (ImageViewerState*)activity->data;

	UnloadTexture(state->texture);
}

static void DrawImageViewer(Activity* activity, Rectangle bounds)
{
	const int textSize = (int)(gIconSize * gScale);
	const int height = textSize + gPadding * 2;
	Vector2 pos;

	ImageViewerState* viewer = (ImageViewerState*)activity->data;

	if (CheckCollisionPointRec(GetMousePosition(), bounds))
	{
		float wheelMovement = GetMouseWheelMove();
		
		if (wheelMovement > 0)
			viewer->scale += 0.05f;
		else if (wheelMovement < 0)
			viewer->scale -= 0.05f;

		viewer->scale = Clamp(viewer->scale, 0.05f, 16.0f);
	}

	BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);

	/* Draw Background */
	DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, BLACK);

	/* Draw Texture */
	pos.x = (float)(int)(bounds.x + bounds.width / 2.0f - ((float)viewer->texture.width * viewer->scale) / 2.0f);
	pos.y = (float)(int)(bounds.y + bounds.height / 2.0f - ((float)viewer->texture.height * viewer->scale) / 2.0f);

	DrawTextureEx(viewer->texture, pos, 0.0f, viewer->scale, RAYWHITE);

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
	viewer->scale = 1.0f;

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