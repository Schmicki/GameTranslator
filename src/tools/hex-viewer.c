#include "hex-viewer.h"
#include "string-tools.h"

/*************************************************************************************************/

typedef struct HexEditorState
{
	char* file;
	char* data;
	int size;

	Vector2 scroll;
	int charsPerLine; /* hex mode only */
} HexEditorState;

/*************************************************************************************************/

void FillHexInt(unsigned int i, char* txt)
{
	txt[0] = GetHexChar((i >> 28) & 0xF);
	txt[1] = GetHexChar((i >> 24) & 0xF);
	txt[2] = GetHexChar((i >> 20) & 0xF);
	txt[3] = GetHexChar((i >> 16) & 0xF);
	txt[4] = GetHexChar((i >> 12) & 0xF);
	txt[5] = GetHexChar((i >>  8) & 0xF);
	txt[6] = GetHexChar((i >>  4) & 0xF);
	txt[7] = GetHexChar((i >>  0) & 0xF);
	txt[8] = 0;
}

void FillHexChar(unsigned char i, char* txt)
{
	txt[0] = GetHexChar((i >> 4) & 0xF);
	txt[1] = GetHexChar((i >> 0) & 0xF);
	txt[2] = 0;
}

static void HexEditorSetStyleDefault(int iconSize)
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

static void DrawHexViewer(HexEditorState* editor, Rectangle bounds)
{
	if (editor->data == NULL)
		return;

	int textSize = (int)(gIconSize * gScale);
	int padding = gPadding;
	int height = textSize + padding * 2;
	int cpl = editor->charsPerLine;
	int lines = (editor->size + cpl - 1) / cpl;
	int charWidth = textSize / 2;
	int first, last;

	Rectangle content, view;
	Vector2 pos;

	HexEditorSetStyleDefault(textSize);

	content.x = 0;
	content.y = 0;
	content.width = bounds.width - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) - padding;
	content.height = lines * (float)height + (float)height;

	view = GuiScrollPanel(bounds, NULL, content, &editor->scroll);

	first = (int)(-editor->scroll.y / (float)height);
	last = (int)(first + view.height / (float)height);

	pos.x = view.x + padding + (8 * charWidth),
	pos.y = view.y;

	BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);

	/* draw top info */

	for (int i = 0; i < cpl; i++)
	{
		char txt[16];
		FillHexChar(i, txt);
		DrawTextEx(gMonoFont, txt, pos, (float)textSize, 0, DARKGRAY);
		pos.x += height;
	}

	pos.x += charWidth;
	DrawTextEx(gMonoFont, "Text", pos, (float)textSize, 0, DARKGRAY);

	pos.x = view.x + padding;
	pos.y += height;

	for (int i = first; i < last; i++)
	{
		/* draw lines */
		char txt[16];

		/* draw line number */
		FillHexInt(i * cpl, txt);
		DrawTextEx(gMonoFont, txt, pos, (float)textSize, 0, DARKGRAY);
		pos.x += 8 * charWidth;

		/* draw hex line */
		for (int j = 0; j < cpl && (i * j + j) < editor->size; j++)
		{
			FillHexChar(editor->data[i * cpl + j], txt);
			DrawTextEx(gMonoFont, txt, pos, (float)textSize, 0, GRAY);
			pos.x += height;
		}

		pos.x = (view.x + padding) + (8 * charWidth) + (height * editor->charsPerLine + charWidth);

		/* draw text line */
		for (int j = 0; j < cpl && (i * j + j) < editor->size; j++)
		{
			txt[0] = editor->data[i * cpl + j];
			txt[1] = 0;
			DrawTextEx(gMonoFont, txt, pos, (float)textSize, 0, GRAY);
			pos.x += charWidth;
		}

		/* next line */
		pos.x = view.x + padding;
		pos.y += height;
	}

	EndScissorMode();
}

static void HexEditorDraw(Activity* activity, Rectangle bounds)
{
	HexEditorState* editor = (HexEditorState*)activity->data;

	DrawHexViewer(editor, bounds);
}

void CloseHexEditor(Activity* activity)
{
	HexEditorState* state = (HexEditorState*)activity->data;

	free(state->file);
	free(state->data);
	free(state);

	activity->name = NULL;
	activity->data = NULL;
	activity->Draw = NULL;
	activity->Close = NULL;
}

Activity OpenHexEditor(const char* file)
{
	HexEditorState* editor;
	Activity activity = { NULL, NULL, NULL, NULL };
	int length;
	const char* name;

	/* Editor Setup */

	editor = malloc(sizeof(HexEditorState));
	
	if (editor == NULL)
		return activity;

	/* filepath */

	length = (int)strlen(file) + 1;
	editor->file = malloc(length);

	if (editor->file == NULL)
		goto cleanEditor;

	memcpy(editor->file, file, length);

	/* data */

	editor->data = (char*)LoadFileData(editor->file, (unsigned int*)&editor->size);

	if (editor->data == NULL)
		goto cleanFile;

	editor->scroll.x = 0.0f;
	editor->scroll.y = 0.0f;
	editor->charsPerLine = 32;

	/* Activity Setup */
	
	name = GetFileName(file);
	length = (int)strlen(name) + 1;
	activity.name = malloc(length);

	if (activity.name == NULL)
		goto cleanData;

	memcpy((void*)activity.name, name, length);

	activity.data = editor;
	activity.Draw = &HexEditorDraw;
	activity.Close = &CloseHexEditor;

	return activity;

cleanData:
	UnloadFileData(editor->data);

cleanFile:
	free(editor->file);

cleanEditor:
	free(editor);
	return activity;
}