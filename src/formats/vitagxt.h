#pragma once

#include "tools/file-explorer/file-explorer.h"

int FormatIsVitaGxt(const char* path, const char* name, const char* extension, const char* data,
	int length);

Image FormatVitaGxtLoadImage(const char* src, int index);

/*************************************************************************************************/

extern FileFormat formatVitaGxt;
