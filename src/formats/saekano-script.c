#include "platform/filesystem.h"
#include "saekano-script.h"

/*************************************************************************************************/

/*
* # HEADER
* 
* SIZE      | TYPE              | NAME                      | INFO
* 4 bytes   | unsgined int      | command count				|
* 4 bytes   | unsigned int      | command offset			|
* x bytes   | string (utf16)    | edit date                 | null terminated
* 
* # COMMAND_COUNT * COMMAND
* 
* SIZE      | TYPE              | NAME              | INFO
* 4 bytes   | unsgined int      | command size      |
* x bytes   | ?                 | ?                 |
* 
* # END
* 
* SIZE      | TYPE              | NAME              | INFO
* 4 bytes   | unsigned int      | filename length   |
* x bytes   | string (utf16)    | filename          | not null terminated
*/

/*************************************************************************************************/

typedef struct SaekanoScriptHeader
{
	unsigned int commandCount;
	unsigned int commandOffset;
	unsigned short editDate[20]; /* 2015/02/26 18:45:34 */
} SaekanoScriptHeader;

typedef struct SaekanoScriptCommand
{
	unsigned int commandSize;
} SaekanoScriptCommand;

/*************************************************************************************************/

void FormatSaekanoScriptPrintInfo(FileManagerState* state, int index)
{
	FileFormat* format = NULL;
	const char* path = NULL;
	unsigned int size, pos = 0;
	char* data;

	int i = index;

	if (i != -1 && state->formats[i]->type != FILE_TYPE_FOLDER)
	{
		format = state->formats[i];
		path = state->files.paths[i];
	}

	if (format == NULL)
		return;

	data = LoadFileData(path, &size);

	if (data == NULL)
		return;

	int commandCount = *(int*)(data + pos);
	pos += 4;

	int commandOffset = *(int*)(data + pos);
	pos = commandOffset;

	int realCmdCount = 0;

	printf("command count: %d\n", commandCount);
	printf("command offset: %d\n", commandOffset);

	for (int commandSize = *(int*)(data + pos); commandSize != 0; commandSize = *(int*)(data + pos))
	{
		pos += commandSize;
		realCmdCount++;
	}

	printf("real command count: %d\n", realCmdCount);

	UnloadFileData(data);
}

/*************************************************************************************************/

void FormatSaekanoScriptGetActions(ContextActionList* actions)
{
	actions->names[actions->count] = memcpy(malloc(18), "Print Script Info", 18);
	actions->functions[actions->count] = &FormatSaekanoScriptPrintInfo;
	actions->count++;
}

/*************************************************************************************************/

FileFormat formatSaekanoScript = {
	"Saekano Script File",
	FILE_TYPE_TEXT,
	ICON_CUSTOM_FILE_DOCUMENT,
	NULL,
	&FormatSaekanoScriptGetActions
};