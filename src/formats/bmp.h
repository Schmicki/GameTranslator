#pragma once

#include "tools/file-explorer/file-explorer.h"

int IsFormatBmp(const char* path, const char* name, const char* extension, const char* data, int length);

Image FormatBmpLoadImage(const char* src);

/*************************************************************************************************/

extern FileFormat formatBmp;