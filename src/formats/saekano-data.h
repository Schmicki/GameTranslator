#pragma once

#include "tools/file-explorer/file-explorer.h"

/*************************************************************************************************/

int FormatIsSaekanoData(const char* path, const char* name, const char* extension, const char* data,
	int length);

int FormatSaekanoDataUnpack(const char* src, int offset, char* dst);

/*************************************************************************************************/

extern FileFormat formatSaekanoData;