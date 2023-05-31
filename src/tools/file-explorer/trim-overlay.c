#include "platform/filesystem.h"
#include "trim-overlay.h"

/*************************************************************************************************/

typedef struct TrimOverlayState
{
	char* path;
	int size;
    int front;
    int back;
	int edit;
} TrimOverlayState;

/*************************************************************************************************/

void CloseTrimOverlay(FileManagerState* state, TrimOverlayState* trim)
{
	free(trim->path);
	free(trim);

	state->overlay.Draw = NULL;
	state->overlay.data = NULL;
}

static void TrimFile(TrimOverlayState* trim)
{
	FILE* src;
	FILE* dst;
	char* name;
	char* buffer;
	int rest;
	int combinedTrim;

	if ((name = malloc(0x1000)) == NULL)
		return;

	memcpy(name, trim->path, strlen(trim->path) + 1);

	if (!GetUniqueFileName(name, 0x1000))
	{
		printf("Trimming failed: GetUniqueFileName!\n");
		goto cleanupName;
	}

	if ((src = fopen(trim->path, "rb")) == NULL)
	{
		printf("Trimming failed: Open Src!\n");
		goto cleanupName;
	}

	if ((dst = fopen(name, "wb")) == NULL)
	{
		printf("Trimming failed: Open Dst!\n");
		goto cleanupSrc;
	}

	if ((buffer = malloc(0x100000)) == NULL)
	{
		printf("Trimming failed: Allocate buffer!\n");
		goto cleanupDst;
	}

	fseek(src, trim->front, SEEK_SET);
	rest = GetFileLength(trim->path);

	if ((combinedTrim = trim->front + trim->back) >= rest)
	{
		printf("Trimming failed: File too small!\n");
		goto cleanupBuffer;
	}

	rest -= combinedTrim;

	while (rest > 0)
	{
		int size = rest > 0x100000 ? 0x100000 : rest;
		if (fread(buffer, 1, size, src) < size)
		{
			printf("Trimming failed: Read file!\n");
			break;
		}

		if (fwrite(buffer, 1, size, dst) < size)
		{
			printf("Trimming failed: Write file!\n");
			break;
		}

		rest -= size;
	}

cleanupBuffer:
	free(buffer);

cleanupDst:
	fclose(dst);

cleanupSrc:
	fclose(src);

cleanupName:
	free(name);
}

int GuiTrimOverlay(FileManagerState* state, TrimOverlayState* trim, Rectangle bounds)
{
	const int textSize = (int)(gIconSize * gScale);
	const int sliderWidth = 200;
	const int sliderHeight = textSize + gPadding * 2;
	const int windowWidth = sliderWidth + gPadding * 2;
	const int windowHeight = RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT + sliderHeight * 3 + gPadding * 4;

	Rectangle window = {
		bounds.x = bounds.width / 2 - (float)windowWidth / 2,
		bounds.y = bounds.height / 2 - (float)windowHeight / 2,
		bounds.width = (float)windowWidth,
		bounds.height = (float)windowHeight
	};

	Rectangle inputBox = {
		window.x + gPadding,
		window.y + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT + gPadding,
		(float)sliderWidth,
		(float)sliderHeight
	};

	if (GuiWindowBox(window, "Trimming"))
	{
		CloseTrimOverlay(state, trim);
		return 1;
	}

	char inputText[16];
	int edit = trim->edit;

	inputText[IntegerToString(trim->front, inputText)] = 0;
	edit = GuiTextBox(inputBox, inputText, 16, edit == 1) ? (edit == 1 ? 0 : 1) : edit;
	trim->front = TextToInteger(inputText);
	inputBox.y += sliderHeight + gPadding;

	inputText[IntegerToString(trim->back, inputText)] = 0;
	edit = GuiTextBox(inputBox, inputText, 16, edit == 2) ? (edit == 2 ? 0 : 2) : edit;
	trim->back = TextToInteger(inputText);
	inputBox.y += sliderHeight + gPadding;

	trim->edit = edit;

	if (GuiButtonEx(inputBox, "Trim"))
	{
		TrimFile(trim);
		ReloadFilesAndTypes(state);
		CloseTrimOverlay(state, trim);
		return 1;
	}

	if (IsKeyPressed(KEY_TAB) && IsKeyDown(KEY_LEFT_CONTROL))
	{
		CloseTrimOverlay(state, trim);
		return 1;
	}

	return 0;
}

Overlay OpenTrimOverlay(const char* path)
{
	TrimOverlayState* state;
	Overlay overlay = { NULL, NULL };
	int len;

	if ((state = malloc(sizeof(TrimOverlayState))) == NULL)
		return overlay;

	if ((state->path = malloc(len = (int)strlen(path) + 1)) == NULL)
		goto cleanupState;

	memcpy(state->path, path, len);
	state->size = GetFileLength(path);
	state->front = 0;
	state->back = 0;

	overlay.Draw = &GuiTrimOverlay;
	overlay.data = state;
	return overlay;

cleanupState:
	free(state);
	return overlay;
}