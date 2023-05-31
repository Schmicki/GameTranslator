#pragma once

#include "tools/file-explorer/file-explorer.h"

/*************************************************************************************************/

int FormatIsSaekanoScript(const char* path, const char* name, const char* extension, const char* data,
	int length);

/*************************************************************************************************/

extern FileFormat formatSaekanoScript;