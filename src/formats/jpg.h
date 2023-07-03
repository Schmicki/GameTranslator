#pragma once

#include "tools/explorer/explorer.h"

int IsFormatJpg(const char* path, const char* name, const char* extension, const char* data, int length);

Image FormatJpgLoadImage(const char* src);

/*************************************************************************************************/

extern FileFormat formatJpg;
