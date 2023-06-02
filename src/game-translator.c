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

static const char* gFontPath = "resources/Noto_Sans/NotoSans-Light.ttf";
static unsigned char* gFontData = NULL;
static unsigned int gFontDataSize = 0;

static const char* gMonoFontPath = "resources/Noto_Sans_Mono/static/NotoSansMono-Light.ttf";
static unsigned char* gMonoFontData = NULL;
static unsigned int gMonoFontDataSize = 0;

static const int gFontFilter = TEXTURE_FILTER_POINT;

/*************************************************************************************************/

int main(int argc, char** args)
{
	int running, drag = 0, scroll = 0;

	/* Create window */
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetConfigFlags(FLAG_WINDOW_UNDECORATED);

	InitWindow(1280, 720, "Game Translator Toolkit");

	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	/* Load icons */
	gIcons = LoadTexture("resources/Icons.png");
	SetTextureFilter(gIcons, TEXTURE_FILTER_BILINEAR);
	gIconSize = 48;
	gScale = 0.5f;
	gPadding = 4;

	/* Load font */
	gFontData = LoadFileData(gFontPath, &gFontDataSize);
	gFont = LoadFontFromMemory(GetFileExtension(gFontPath), gFontData, gFontDataSize, (int)(gScale
		* (float)gIconSize), NULL, 224);
	SetTextureFilter(gFont.texture, gFontFilter);
	GuiSetFont(gFont);

	/* Load monospace font */
	gMonoFontData = LoadFileData(gMonoFontPath, &gMonoFontDataSize);
	gMonoFont = LoadFontFromMemory(GetFileExtension(gMonoFontPath), gMonoFontData, gMonoFontDataSize,
		(int)(gScale * (float)gIconSize), NULL, 95);
	SetTextureFilter(gMonoFont.texture, gFontFilter);

	gDefaultDirectory = "C:/Users/nicke/Desktop/Game Translation/";
	gTopLayerTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
	AddActivity(OpenFileManagerActivity(gDefaultDirectory), 0);
	running = 1;

	while (running && !WindowShouldClose())
	{
		int iconSize;
		int buttonSize;
		Rectangle bounds;

		/* Scale UI */
		float mouseWheel = GetMouseWheelMove();
		float newScale = gScale;

		if (mouseWheel > 0.0f && IsKeyDown(KEY_LEFT_CONTROL))
			newScale += 0.05f;

		if (mouseWheel < 0.0f && IsKeyDown(KEY_LEFT_CONTROL))
			newScale -= 0.05f;

		newScale = Clamp(newScale, 0.35f, 1.0f);
		iconSize = (int)(gIconSize * newScale);
		buttonSize = iconSize + gPadding * 2;

		/* Reload fonts */
		if (newScale != gScale)
		{
			gScale = newScale;

			UnloadFont(gFont);
			UnloadFont(gMonoFont);

			gFont = LoadFontFromMemory(GetFileExtension(gFontPath), gFontData, gFontDataSize,
				iconSize, NULL, 224);
			SetTextureFilter(gFont.texture, gFontFilter);
			GuiSetFont(gFont);

			gMonoFont = LoadFontFromMemory(GetFileExtension(gMonoFontPath), gMonoFontData,
				gMonoFontDataSize, iconSize, NULL, 224);
			SetTextureFilter(gMonoFont.texture, gFontFilter);
		}

		/* Render target cleanup */
		if (gTopLayerTexture.texture.width != GetScreenWidth() &&
			gTopLayerTexture.texture.height != GetScreenHeight())
		{
			UnloadRenderTexture(gTopLayerTexture);
			gTopLayerTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
		}

		/* Set default style */
		GuiSetStyle(DEFAULT, TEXT_SIZE, iconSize);
		GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

		BeginTextureMode(gTopLayerTexture);
		ClearBackground(BLANK);
		EndTextureMode();

		BeginDrawing();
		ClearBackground(GetColor(gBackgroundColor));

		/* Activity */
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

		/* Status Bar */
		bounds.y = 0;
		bounds.height = (float)GetScreenHeight();

		running = GuiWindowControl(bounds, "Game Translator", &drag, &scroll);

		EndDrawing();
	}

	UnloadFont(gFont);
	UnloadFont(gMonoFont);
	CloseWindow();
}
