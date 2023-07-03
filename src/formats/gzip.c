#include "platform/filesystem.h"
#include "zlib-master/zlib.h"
#include "gzip.h"

int FormatIsGZip(const char* path, const char* name, const char* extension, const char* data, int length)
{
	char magic[4] = { 0x1F, 0x8B };
	return length >= 2 && strncmp(data, magic, 2) == 0;
}

int FormatGZipUnpack_(const char* src, int offset, const char* dst)
{
	FILE* fsrc;
	FILE* fdst;
	char* in;
	char* out;
	z_stream stream;
	int result, ret = 0;
	const int IN_CHUNK_SIZE = 0x100000;
	const int OUT_CHUNK_SIZE = IN_CHUNK_SIZE * 2;

	if ((fsrc = fopen(src, "rb")) == NULL)
		return 0;

	fseek(fsrc, offset, SEEK_SET);

	if ((fdst = fopen(dst, "wb")) == NULL)
		goto cleanupSrc;

	if ((in = malloc(IN_CHUNK_SIZE)) == NULL)
		goto cleanupDst;

	if ((out = malloc(OUT_CHUNK_SIZE)) == NULL)
		goto cleanupIn;

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_in = Z_NULL;

	if ((result = inflateInit2(&stream, MAX_WBITS | 0x20)) != Z_OK)
		goto cleanupOut;

	do
	{
		if ((stream.avail_in = (unsigned int)fread(in, 1, IN_CHUNK_SIZE, fsrc)) == 0)
			goto cleanAll;

		stream.next_in = (Bytef*)in;

		do
		{
			int length;

			stream.avail_out = OUT_CHUNK_SIZE;
			stream.next_out = out;

			result = inflate(&stream, Z_NO_FLUSH);
			switch (result) {
			case Z_STREAM_ERROR:
			case Z_NEED_DICT:
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				printf("%s\n", stream.msg);
				goto cleanAll;
			}

			printf("writing...\n");
			length = OUT_CHUNK_SIZE - stream.avail_out;

			if (fwrite(out, 1, length, fdst) != length && (printf("writing error\n"), 1))
				goto cleanAll;

		} while (stream.avail_out == 0);
	} while (result != Z_STREAM_END);

	ret = 1;
	printf("success\n");

cleanAll:
	(void)inflateEnd(&stream);

cleanupOut:
	free(out);

cleanupIn:
	free(in);

cleanupDst:
	fclose(fdst);

cleanupSrc:
	fclose(fsrc);
	return ret;
}

int FormatGZipUnpack(const char* src, int offset, char* dst)
{
	char* p;
	char* n;
	char* sn;
	char* se;
	int len;

	if ((p = malloc(0x1000)) == NULL)
		return 0;

	n = p + strlen(dst) + 1;
	memcpy(p, dst, n - p);
	memcpy(n - 1, "/", 2);

	sn = (char*)GetFileName(src);
	se = (char*)GetFileExtension(sn);
	len = se ? (int)(se - sn) : (int)strlen(sn);
	memcpy(n, sn, len);
	n[len] = 0;

	FormatGZipUnpack_(src, offset, p);

	free(p);
	return 1;
}

void FormatGZipUnpackHere(ExplorerState* state, int index)
{
	const char* path;

	for (int i = 0; i < (int)state->files.count; i++)
	{
		if (state->selected[i] && state->files.formats[i] == &formatGZip)
		{
			path = state->files.paths[i];
			FormatGZipUnpack(path, 0, state->path);
		}
	}

	ExplorerReload(state);
}

void FormatGZipUnpackToFolder(ExplorerState* state, int index)
{
	FileFormat* format = NULL;
	char* path;
	char* src;
	char* noext;
	int length;

	if ((src = malloc(0x1000)) == NULL)
		return;

	if ((noext = malloc(0x1000)) == NULL)
		goto cleanupSrc;

	for (int i = 0; i < (int)state->files.count; i++)
	{
		if (state->selected[i] && state->files.formats[i] == &formatGZip)
		{
			path = state->files.paths[i];
			length = (int)strlen(path) + 1;
			memcpy(src, path, length);
			memcpy(noext, path, length);

			if ((path = (char*)GetFileExtension(GetFileName(noext))) != NULL)
				*path = 0;

			if (!GetUniqueFileName(noext, 0x1000))
				continue;

			CreateFolder(noext);
			FormatGZipUnpack(src, 0, noext);
			ExplorerReload(state);
		}
	}

	free(noext);

cleanupSrc:
	free(src);
}

/*************************************************************************************************/

void FormatGZipGetActions(ContextActionList* actions)
{
	actions->names[actions->count] = memcpy(malloc(12), "Unpack Here", 12);
	actions->functions[actions->count] = &FormatGZipUnpackHere;
	actions->count++;

	actions->names[actions->count] = memcpy(malloc(17), "Unpack To Folder", 17);
	actions->functions[actions->count] = &FormatGZipUnpackToFolder;
	actions->count++;
}

/*************************************************************************************************/

FileFormat formatGZip = {
	"GZip Archive",
	FILE_TYPE_ARCHIVE,
	ICON_CUSTOM_ARCHIVE,
	&FormatIsGZip,
	&FormatGZipGetActions
};