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
	const int buttonSize = textSize + gPadding * 2;
	const int windowWidth = (int)((float)600 * gScale);
	const int windowHeight = (int)((float)gPadding * 6.0f + (float)buttonSize * 4.0f);

	Rectangle item = {
		bounds.x = bounds.width / 2 - (float)windowWidth / 2,
		bounds.y = bounds.height / 2 - (float)windowHeight / 2,
		bounds.width = (float)windowWidth,
		bounds.height = (float)windowHeight
	};

	int itmp;
	char inputText[16];
	int edit = trim->edit;

	GuiSetStyleDefault();
	GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

	/* Draw background */
	itmp = (int)item.y + buttonSize + gPadding * 2;

	GuiPanel(item, NULL);
	DrawLine((int)item.x, itmp, (int)item.x + (int)item.width, itmp, GetColor(GuiGetStyle(DEFAULT,
		LINE_COLOR)));

	/* Draw title */
	item.x += (float)gPadding;
	item.y += (float)gPadding;
	item.width -= (float)gPadding * 2;
	item.height = (float)buttonSize;

	GuiTextBox(item, "Trim File", textSize, 0);

	GuiEnableTooltip();

	/* Draw cancel button */
	itmp = (int)item.x - gPadding;
	item.x = (float)itmp + (float)((windowWidth / 3) * 1 - buttonSize / 2);
	item.y += (int)((float)gPadding * 4.0f + (float)buttonSize * 3.0f);
	item.width = (float)buttonSize;
	item.height = (float)buttonSize;

	if (GuiIconButtonEx(item, "Cancel", gIcons, gIconSize, gPadding, ICON_CUSTOM_CLOSE, gScale))
	{
		CloseTrimOverlay(state, trim);
		GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
		return 1;
	}

	item.x += (float)windowWidth / 3.0f;

	if (GuiIconButtonEx(item, "Trim", gIcons, gIconSize, gPadding, ICON_CUSTOM_TRIM, gScale))
	{
		TrimFile(trim);
		ReloadFilesAndTypes(state);
		CloseTrimOverlay(state, trim);
		GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
		return 1;
	}

	/* Draw back trim input */
	GuiSetStyleTextboxOutlined();
	GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

	item.x = itmp + (float)gPadding;
	item.y -= item.height + (float)gPadding;
	item.width = (float)(windowWidth - gPadding * 2);
	inputText[IntegerToString(trim->back, inputText)] = 0;

	GuiSetTooltip("Number of bytes to cut from back of file");
	edit = GuiTextBox(item, inputText, textSize, edit == 2) ? (edit == 2 ? 0 : 2) : edit;

	trim->back = TextToInteger(inputText);

	/* Draw front trim input */	
	item.y -= (float)(buttonSize + gPadding);
	inputText[IntegerToString(trim->front, inputText)] = 0;

	GuiSetTooltip("Number of bytes to cut from front of file");
	edit = GuiTextBox(item, inputText, textSize, edit == 1) ? (edit == 1 ? 0 : 1) : edit;

	trim->front = TextToInteger(inputText);
	trim->edit = edit;

	GuiDisableTooltip();

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