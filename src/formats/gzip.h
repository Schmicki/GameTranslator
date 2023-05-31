#pragma once

#include "tools/file-explorer/file-explorer.h"

int FormatIsGZip(const char* path, const char* name, const char* extension, const char* data, int length);

int FormatGZipUnpack(const char* src, int offset, char* dst);

/*************************************************************************************************/

extern FileFormat formatGZip;