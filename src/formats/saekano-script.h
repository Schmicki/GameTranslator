#pragma once

#include "tools/explorer/explorer.h"

/*************************************************************************************************/

int FormatIsSaekanoScript(const char* path, const char* name, const char* extension, const char* data,
	int length);

/*************************************************************************************************/

extern FileFormat formatSaekanoScript;