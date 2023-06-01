#pragma once

#include "tools/file-explorer/file-explorer.h"

int IsFormatGif(const char* path, const char* name, const char* extension, const char* data, int length);

Image FormatGifLoadImage(const char* src);

/*************************************************************************************************/

extern FileFormat formatGif;