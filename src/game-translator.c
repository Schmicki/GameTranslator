#include "status-bar.h"
#include "tools/file-explorer/file-explorer.h"
#include "string-tools.h"
#include "platform/threadsafe.h"

/*************************************************************************************************/

Texture2D gIcons = { 0 };

int gIconSize = 0;

float gScale = 0.5f;

int gPadding = 4;

Font gFont = { 0 };

Font gMonoFont = { 0 };

Activity gActivities[MAX_ACTIVITIES];

int gActivityCount = 0;

int gCurrentActivity = -1;

const char* gDefaultDirectory = NULL;

RenderTexture2D gTopLayerTexture = { 0 };

/*************************************************************************************************/

int main(int argc, char** args)
{
	int running, drag = 0, scroll = 0;

	/*
	* Create Window
	*/

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetConfigFlags(FLAG_WINDOW_UNDECORATED);

	InitWindow(1280, 720, "Game Translator Toolkit");

	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	/*
	* Load Global Resources
	*/

	gIcons = LoadTexture("resources/Icons.png");
	SetTextureFilter(gIcons, TEXTURE_FILTER_BILINEAR);
	gIconSize = 48;
	gScale = 0.5f;
	gPadding = 4;

	gFont = LoadFontEx("resources/Noto_Sans/NotoSans-Light.ttf", 48, NULL, 95);
	SetTextureFilter(gFont.texture, TEXTURE_FILTER_BILINEAR);
	GuiSetFont(gFont);

	if (gMonoFont.texture.id != 0)
		UnloadFont(gMonoFont);

	gMonoFont = LoadFontEx("resources/Noto_Sans_Mono/static/NotoSansMono-Light.ttf", 48, NULL, 95);
	SetTextureFilter(gMonoFont.texture, TEXTURE_FILTER_BILINEAR);

	gDefaultDirectory = "C:/Users/nicke/Desktop/Game Translation/";

	gTopLayerTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	AddActivity(OpenFileManagerActivity(gDefaultDirectory), 0);

	running = 1;
	while (running && !WindowShouldClose())
	{
		const int iconSize = (int)(gIconSize * gScale);
		const int buttonSize = iconSize + gPadding * 2;
		Rectangle bounds;

		float mouseWheel = GetMouseWheelMove();

		if (mouseWheel > 0.0f && IsKeyDown(KEY_LEFT_CONTROL))
			gScale += 0.05f;

		if (mouseWheel < 0.0f && IsKeyDown(KEY_LEFT_CONTROL))
			gScale -= 0.05f;

		gScale = Clamp(gScale, 0.35f, 1.0f);

		/*
		* Render target cleanup
		*/
		
		if (gTopLayerTexture.texture.width != GetScreenWidth() &&
			gTopLayerTexture.texture.height != GetScreenHeight())
		{
			UnloadRenderTexture(gTopLayerTexture);
			gTopLayerTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
		}

		BeginTextureMode(gTopLayerTexture);
		ClearBackground(BLANK);
		EndTextureMode();

		BeginDrawing();
		ClearBackground(WHITE);

		/*
		* Activity
		*/

		bounds.x = 0;
		bounds.y = (float)(buttonSize + gPadding * 2);

		bounds.width = (float)GetScreenWidth();
		bounds.height = (float)(GetScreenHeight() - buttonSize - gPadding);

		if (gCurrentActivity >= gActivityCount)
			gCurrentActivity = gActivityCount - 1;

		if (gCurrentActivity == -1 && gActivityCount > 0)
			gCurrentActivity = 0;

		if (gCurrentActivity != -1)
		{
			Activity activity = gActivities[gCurrentActivity];
			activity.Draw(gActivities + gCurrentActivity, bounds);
		}
		else
		{
			gCurrentActivity = gActivityCount == 0 ? -1 : gActivityCount - 1;
		}

		DrawTextureRec(gTopLayerTexture.texture, (Rectangle) { 0, 0,
			(float)gTopLayerTexture.texture.width, (float)-gTopLayerTexture.texture.height },
			(Vector2) { 0, 0 }, WHITE);

		/*
		* Status Bar
		*/

		bounds.y = 0;
		bounds.height = (float)GetScreenHeight();

		running = GuiWindowControl(bounds, "Game Translator", &drag, &scroll);

		EndDrawing();
	}

	UnloadFont(gFont);
	UnloadFont(gMonoFont);
	CloseWindow();
}
