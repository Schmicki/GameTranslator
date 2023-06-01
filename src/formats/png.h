#pragma once

#include "tools/file-explorer/file-explorer.h"

int IsFormatPng(const char* path, const char* name, const char* extension, const char* data, int length);

Image FormatPngLoadImage(const char* src);

/*************************************************************************************************/

extern FileFormat formatPng;