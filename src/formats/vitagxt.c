#include "psp2/gxm.h"
#include "psp2/gxt.h"

#include "platform/filesystem.h"
#include "tools/image-viewer.h"

#include "vitagxt.h"

/**************************************************************************************************
* Structure and Type Definitions
*/

typedef struct VitaGxtTextureFormat
{
	void (**SwizzleTable)(char* src, char* dst);
	int dataBytesPerPixel;
	int imageBytesPerPixel;
} VitaGxtTextureFormat;

typedef void (*SwizzleFunction)(void* src, void* dst);

/**************************************************************************************************
* Loader Functions
*/

static Color* FormatVitaGxtLoadPalette4(SceGxtHeader header, FILE* file)
{
	int p4Start = (header.dataOffset + header.dataSize) - (256 * 4 * header.numP8Palettes)
		- (16 * 4 * header.numP4Palettes);

	printf("Read palette (P4) at offset: %d!\n", p4Start);

	Color* palette = malloc(16 * 4);

	if (palette == NULL)
	{
		printf("Failed to allocate palette!\n");
		return NULL;
	}

	fseek(file, p4Start, SEEK_SET);
	
	if (fread(palette, 1, 16 * 4, file) != (16 * 4))
	{
		printf("Failed to read palette!\n");
		free(palette);
		return NULL;
	}

	return palette;
}

static Color* FormatVitaGxtLoadPalette8(SceGxtHeader header, FILE* file)
{
	int p8Start = (header.dataOffset + header.dataSize) - (256 * 4 * header.numP8Palettes);
	Color* palette = malloc(256 * 4);

	printf("Read palette (P8) at offset: %d!\n", p8Start);

	if (palette == NULL)
	{
		printf("Failed to allocate palette!\n");
		return NULL;
	}

	fseek(file, p8Start, SEEK_SET);

	if (fread(palette, 1, 256 * 4, file) != (256 * 4))
	{
		printf("Failed to read palette!\n");
		free(palette);
		return NULL;
	}

	return palette;
}

static char* FormatVitaGxtLoadImageData(SceGxtTextureInfo info, FILE* file)
{
	char* image = malloc(info.dataSize);

	if (image == NULL)
	{
		printf("Failed to allocate image!\n");
		return NULL;
	}

	fseek(file, info.dataOffset, SEEK_SET);

	if (fread(image, 1, info.dataSize, file) != (info.dataSize))
	{
		printf("Failed to read image!\n");
		free(image);
		return NULL;
	}

	printf("gxt dataOffset: %d\n", info.dataOffset);
	printf("gxt dataSize: %d\n", info.dataSize);
	printf("gxt paletteIndex: %d\n", info.paletteIndex);
	printf("gxt flags: %d\n", info.flags);
	printf("gxt type: %d\n", info.type);
	printf("gxt format: %d\n", info.format);
	printf("gxt width: %d\n", info.width);
	printf("gxt height: %d\n", info.height);
	printf("gxt mipCount: %d\n", info.mipCount);

	return image;
}

/**************************************************************************************************
* We get BGRA
* We talk about ARGB
* We want RGBA
* 
* 1RGB(ARGB) in ARGB space = {1,R,G,B}
* 1RGB(ARGB) in BGRA space = {B,G,R,1}
* 1RGB(ARGB) in RGBA space = {R,G,B,1}
* 
* RGBA(ARGB) in ARGB space = {A,R,G,B}
* RGBA(ARGB) in BGRA space = {B,G,R,A}
* RGBA(ARGB) in RGBA space = {R,G,B,A}
*/

static void RGBAFromABGR(char* rgba, char* src)
{
	char* a = rgba, *b = src;
	a[0] = b[3], a[1] = b[2], a[2] = b[1], a[3] = b[0];
}

static void RGBAFromARGB(void* rgba, void* src)
{
	char* a = rgba, * b = src;
	a[0] = b[1], a[1] = b[2], a[2] = b[3], a[3] = b[0];
}

static void RGBAFromRGBA(void* rgba, void* src)
{
	char* a = rgba, * b = src;
	a[0] = b[0], a[1] = b[1], a[2] = b[2], a[3] = b[3];
}

static void RGBAFromBGRA(void* rgba, void* src)
{
	char* a = rgba, * b = src;
	a[0] = b[2], a[1] = b[1], a[2] = b[0], a[3] = b[3];
}

static void RGBAFrom1BGR(void* rgba, void* src)
{
	char* a = rgba, * b = src;
	a[0] = b[3], a[1] = b[2], a[2] = b[1], a[3] = 0xFF;
}

static void RGBAFrom1RGB(void* rgba, void* src)
{
	char* a = rgba, * b = src;
	a[0] = b[1], a[1] = b[2], a[2] = b[3], a[3] = 0xFF;
}

static void RGBAFromRGB1(void* rgba, void* src)
{
	char* a = rgba, * b = src;
	a[0] = b[0], a[1] = b[1], a[2] = b[2], a[3] = 0xFF;
}

static void RGBAFromBGR1(void* rgba, void* src)
{
	char* a = rgba, * b = src;
	a[0] = b[2], a[1] = b[1], a[2] = b[0], a[3] = 0xFF;
}

/**************************************************************************************************
* 
*	The unswizzle logic is based on
*	https://github.com/FireyFly/reversing/blob/master/formats/dxt/rearrange.c which I found while
*	reading https://github.com/xdanieldzd/GXTConvert/blob/85821e7b9e5b1d0464404eec78d320c3a0e87deb/GXTConvert/Conversion/PostProcessing.cs.
*	And it is licensed under the following license.
*	
*	-----------------------------------------------------------------------------------------------
*	
*	MIT License
*	
*	Copyright (c) 2018 Jonas
*	
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*	
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*	
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*	
*/

/* From https://github.com/FireyFly/reversing/blob/master/formats/dxt/rearrange.c */
static int Compact1By1(int x)
{
	x &= 0x55555555;                 // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
	x = (x ^ (x >> 1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
	x = (x ^ (x >> 2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
	x = (x ^ (x >> 4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
	x = (x ^ (x >> 8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
	return x;
}

/* From https://github.com/FireyFly/reversing/blob/master/formats/dxt/rearrange.c */
static int DecodeMorton2X(int code)
{
	return Compact1By1(code >> 0);
}

/* From https://github.com/FireyFly/reversing/blob/master/formats/dxt/rearrange.c */
static int DecodeMorton2Y(int code)
{
	return Compact1By1(code >> 1);
}

/* Based on https://github.com/FireyFly/reversing/blob/master/formats/dxt/rearrange.c */
static void UnswizzleMorton(char* data, int width, int height, int pixelStride)
{
	char* original;
	int size = width * height * pixelStride;
	int pos = 0;

	if ((original = malloc(size)) == NULL)
		return;

	memcpy(original, data, size);

	for (int i = 0; i < width * height; i++, pos += pixelStride)
	{
		int min = width < height ? width : height;
		int k = (int)log2(min);

		int x, y;
		if (height < width)
		{
			// XXXyxyxyx -> XXXxxxyyy
			int j;

			j = i >> (2 * k) << (2 * k);
			j |= (DecodeMorton2Y(i) & (min - 1)) << k;
			j |= (DecodeMorton2X(i) & (min - 1)) << 0;
			x = j / height;
			y = j % height;
		}
		else
		{
			// YYYyxyxyx -> YYYyyyxxx
			int j;
			j = i >> (2 * k) << (2 * k);
			j |= (DecodeMorton2X(i) & (min - 1)) << k;
			j |= (DecodeMorton2Y(i) & (min - 1)) << 0;
			x = j % width;
			y = j / width;
		}

		if (y >= height || x >= width || pos >= size)
			continue;

		memcpy(data + (y * width + x) * pixelStride, original + pos, pixelStride);
	}

	free(original);
}

/*************************************************************************************************/

static int VitaGxtGetTextureFormatIndex(unsigned int format)
{
	int index = format >> 24;
	return index > 0x1F ? index - 0x60 : index;
}

static int VitaGxtGetTextureFormatSwizzle(unsigned int format)
{
	return (format >> 12) & 0xFF;
}

/*************************************************************************************************/

static SwizzleFunction textureSwizzle4Table[0x8] = {
	&RGBAFromABGR,
	&RGBAFromARGB,
	&RGBAFromRGBA,
	&RGBAFromBGRA,
	&RGBAFrom1BGR,
	&RGBAFrom1RGB, /* SCE_GXM_TEXTURE_FORMAT_P8_1RGB */
	&RGBAFromRGB1,
	&RGBAFromBGR1
};

static SwizzleFunction textureSwizzle3Table[0x2] = {
	NULL
};

static SwizzleFunction textureSwizzle2Table[0x6] = {
	NULL
};

static SwizzleFunction textureSwizzle2AltTable[0x2] = {
	NULL
};

static SwizzleFunction textureSwizzle1Table[0x8] = {
	NULL
};

static SwizzleFunction textureSwizzleYUV422Table[0x8] = {
	NULL
};

static SwizzleFunction textureSwizzleYUV420Table[0x4] = {
	NULL
};

/*************************************************************************************************/

static VitaGxtTextureFormat vitaGxtTextureFormats[0x3B] = {
	{ NULL, 0, 0 },								/* 0x00 SCE_GXM_TEXTURE_BASE_FORMAT_U8 */
	{ NULL, 0, 0 },								/* 0x01 SCE_GXM_TEXTURE_BASE_FORMAT_S8 */
	{ NULL, 0, 0 },								/* 0x02 SCE_GXM_TEXTURE_BASE_FORMAT_U4U4U4U4 */
	{ NULL, 0, 0 },								/* 0x03 SCE_GXM_TEXTURE_BASE_FORMAT_U8U3U3U2 */
	{ NULL, 0, 0 },								/* 0x04 SCE_GXM_TEXTURE_BASE_FORMAT_U1U5U5U5 */
	{ NULL, 0, 0 },								/* 0x05 SCE_GXM_TEXTURE_BASE_FORMAT_U5U6U5 */
	{ NULL, 0, 0 },								/* 0x06 SCE_GXM_TEXTURE_BASE_FORMAT_S5S5U6 */
	{ NULL, 0, 0 },								/* 0x07 SCE_GXM_TEXTURE_BASE_FORMAT_U8U8 */
	{ NULL, 0, 0 },								/* 0x08 SCE_GXM_TEXTURE_BASE_FORMAT_S8S8 */
	{ NULL, 0, 0 },								/* 0x09 SCE_GXM_TEXTURE_BASE_FORMAT_U16 */
	{ NULL, 0, 0 },								/* 0x0A SCE_GXM_TEXTURE_BASE_FORMAT_S16 */
	{ NULL, 0, 0 },								/* 0x0B SCE_GXM_TEXTURE_BASE_FORMAT_F16 */
	{ NULL, 0, 0 },								/* 0x0C SCE_GXM_TEXTURE_BASE_FORMAT_U8U8U8U8 */
	{ NULL, 0, 0 },								/* 0x0D SCE_GXM_TEXTURE_BASE_FORMAT_S8S8S8S8 */
	{ NULL, 0, 0 },								/* 0x0E SCE_GXM_TEXTURE_BASE_FORMAT_U2U10U10U10 */
	{ NULL, 0, 0 },								/* 0x0F SCE_GXM_TEXTURE_BASE_FORMAT_U16U16 */
	{ NULL, 0, 0 },								/* 0x10 SCE_GXM_TEXTURE_BASE_FORMAT_S16S16 */
	{ NULL, 0, 0 },								/* 0x11 SCE_GXM_TEXTURE_BASE_FORMAT_F16F16 */
	{ NULL, 0, 0 },								/* 0x12 SCE_GXM_TEXTURE_BASE_FORMAT_F32 */
	{ NULL, 0, 0 },								/* 0x13 SCE_GXM_TEXTURE_BASE_FORMAT_F32M */
	{ NULL, 0, 0 },								/* 0x14 SCE_GXM_TEXTURE_BASE_FORMAT_X8S8S8U8 */
	{ NULL, 0, 0 },								/* 0x15 SCE_GXM_TEXTURE_BASE_FORMAT_X8U24 */
	{ NULL, 0, 0 },								/* 0x16 Padding */
	{ NULL, 0, 0 },								/* 0x17 SCE_GXM_TEXTURE_BASE_FORMAT_U32 */
	{ NULL, 0, 0 },								/* 0x18 SCE_GXM_TEXTURE_BASE_FORMAT_S32 */
	{ NULL, 0, 0 },								/* 0x19 SCE_GXM_TEXTURE_BASE_FORMAT_SE5M9M9M9 */
	{ NULL, 0, 0 },								/* 0x1A SCE_GXM_TEXTURE_BASE_FORMAT_F11F11F10 */
	{ NULL, 0, 0 },								/* 0x1B SCE_GXM_TEXTURE_BASE_FORMAT_F16F16F16F16 */
	{ NULL, 0, 0 },								/* 0x1C SCE_GXM_TEXTURE_BASE_FORMAT_U16U16U16U16 */
	{ NULL, 0, 0 },								/* 0x1D SCE_GXM_TEXTURE_BASE_FORMAT_S16S16S16S16 */
	{ NULL, 0, 0 },								/* 0x1E SCE_GXM_TEXTURE_BASE_FORMAT_F32F32 */
	{ NULL, 0, 0 },								/* 0x1F SCE_GXM_TEXTURE_BASE_FORMAT_U32U32 */
	{ NULL, 0, 0 },								/* 0x20 SCE_GXM_TEXTURE_BASE_FORMAT_PVRT2BPP */
	{ NULL, 0, 0 },								/* 0x21 SCE_GXM_TEXTURE_BASE_FORMAT_PVRT4BPP */
	{ NULL, 0, 0 },								/* 0x22 SCE_GXM_TEXTURE_BASE_FORMAT_PVRTII2BPP */
	{ NULL, 0, 0 },								/* 0x23 SCE_GXM_TEXTURE_BASE_FORMAT_PVRTII4BPP */
	{ NULL, 0, 0 },								/* 0x24 Padding */
	{ NULL, 0, 0 },								/* 0x25 SCE_GXM_TEXTURE_BASE_FORMAT_UBC1 */
	{ NULL, 0, 0 },								/* 0x26 SCE_GXM_TEXTURE_BASE_FORMAT_UBC2 */
	{ NULL, 0, 0 },								/* 0x27 SCE_GXM_TEXTURE_BASE_FORMAT_UBC3 */
	{ NULL, 0, 0 },								/* 0x28 SCE_GXM_TEXTURE_BASE_FORMAT_UBC4 */
	{ NULL, 0, 0 },								/* 0x29 SCE_GXM_TEXTURE_BASE_FORMAT_SBC4 */
	{ NULL, 0, 0 },								/* 0x2A SCE_GXM_TEXTURE_BASE_FORMAT_UBC5 */
	{ NULL, 0, 0 },								/* 0x2B SCE_GXM_TEXTURE_BASE_FORMAT_SBC5 */
	{ NULL, 0, 0 },								/* 0x2C Padding */
	{ NULL, 0, 0 },								/* 0x2D Padding */
	{ NULL, 0, 0 },								/* 0x2E Padding */
	{ NULL, 0, 0 },								/* 0x2F Padding */
	{ NULL, 0, 0 },								/* 0x30 SCE_GXM_TEXTURE_BASE_FORMAT_YUV420P2 */
	{ NULL, 0, 0 },								/* 0x31 SCE_GXM_TEXTURE_BASE_FORMAT_YUV420P3 */
	{ NULL, 0, 0 },								/* 0x32 SCE_GXM_TEXTURE_BASE_FORMAT_YUV422 */
	{ NULL, 0, 0 },								/* 0x33 Padding */
	{ NULL, 0, 0 },								/* 0x34 SCE_GXM_TEXTURE_BASE_FORMAT_P4 */
	{ textureSwizzle4Table, 1, 4 },				/* 0x35 SCE_GXM_TEXTURE_BASE_FORMAT_P8 */
	{ NULL, 0, 0 },								/* 0x36 Padding */
	{ NULL, 0, 0 },								/* 0x37 Padding */
	{ NULL, 0, 0 },								/* 0x38 SCE_GXM_TEXTURE_BASE_FORMAT_U8U8U8 */
	{ NULL, 0, 0 },								/* 0x39 SCE_GXM_TEXTURE_BASE_FORMAT_S8S8S8 */
	{ NULL, 0, 0 },								/* 0x3A SCE_GXM_TEXTURE_BASE_FORMAT_U2F10F10F10 */
};

/*************************************************************************************************/

int FormatIsVitaGxt(const char* path, const char* name, const char* extension, const char* data, int length)
{
	return (extension && strcmp(extension, ".gxt") == 0) && (length >= 4 && strncmp(data, "GXT\0", 4) == 0);
}

Image FormatVitaGxtLoadImage(const char* src, int textureIndex)
{
	/*
	* 1. Load data
	* 2. Process data (Unswizzle, Untile, ...)
	* 3. Build image
	*/

	SceGxtHeader header;
	SceGxtTextureInfo info;
	VitaGxtTextureFormat format;
	SwizzleFunction Swizzle;
	unsigned char* data;
	FILE* file;
	Color* palette = NULL;
	Image image = { NULL, 0, 0, 0, -1 };

	if ((file = fopen(src, "rb")) == NULL)
		goto cleanupFile;

	if (fread(&header, 1, sizeof(SceGxtHeader), file) != sizeof(SceGxtHeader))
		goto cleanupFile;

	if ((int)header.numTextures <= textureIndex)
		goto cleanupFile;

	if (fread(&info, 1, sizeof(SceGxtTextureInfo), file) != sizeof(SceGxtTextureInfo))
		goto cleanupFile;

	/* Load data */
	if ((data = FormatVitaGxtLoadImageData(info, file)) == NULL)
		goto cleanupFile;

	format = vitaGxtTextureFormats[VitaGxtGetTextureFormatIndex(info.format)];
	Swizzle = format.SwizzleTable[VitaGxtGetTextureFormatSwizzle(info.format)];

	printf("gxt format index: %d\n", VitaGxtGetTextureFormatIndex(info.format));
	printf("gxt format swizzle: %d\n", VitaGxtGetTextureFormatSwizzle(info.format));

	/* Process data */

	/* Build image */

	if ((info.format & 0xFF000000) == SCE_GXM_TEXTURE_BASE_FORMAT_P8 &&
		(palette = FormatVitaGxtLoadPalette8(header, file)) == NULL)
		goto cleanupData;

	if ((info.format & 0xFF000000) == SCE_GXM_TEXTURE_BASE_FORMAT_P4 &&
		(palette = FormatVitaGxtLoadPalette4(header, file)) == NULL)
		goto cleanupData;

	info.width = IntegerAlignUp(info.width, 4);

	if ((image.data = malloc(info.width * info.height * format.imageBytesPerPixel)) == NULL)
		goto cleanupPalette;

	if (palette != NULL)
	{
		for (int i = 0; i < (info.width * info.height); i++)
		{
			int index = i;
			Color swizzled, color = palette[data[index]];
			RGBAFromABGR((char*)&swizzled, (char*)&color);
			Swizzle((char*)((Color*)image.data + i), (char*)&swizzled);
		}
	}
	else
	{
		for (int i = 0; i < (info.width * info.height); i++)
		{
			void* color = (char*)data + (i * format.dataBytesPerPixel);
			void* swizzled = (char*)image.data + (i * format.imageBytesPerPixel);
			Swizzle(color, swizzled);
		}
	}

	if (info.type == SCE_GXM_TEXTURE_SWIZZLED)
		UnswizzleMorton(image.data, info.width, info.height, 4);

	image.width = info.width;
	image.height = info.height;
	image.mipmaps = 1;
	image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

cleanupPalette:
	free(palette);

cleanupData:
	free(data);

cleanupFile:
	fclose(file);
	return image;
}

static void FortmatVitaGxtViewImage(ExplorerState* state, int index)
{
	FileFormat* format = NULL;
	const char* path = NULL;
	Image img;

	int i = index;

	if (i != -1 && state->files.formats[i]->type != FILE_TYPE_FOLDER)
	{
		format = state->files.formats[i];
		path = state->files.paths[i];
	}

	if (format == NULL || format->type != FILE_TYPE_IMAGE)
		return;

	img = FormatVitaGxtLoadImage(path, 0);
	AddActivity(OpenImageViewer(GetFileName(path), img), gActivityCount);
	UnloadImage(img);
	gCurrentActivity = gActivityCount - 1;
}

static void FormatVitaGxtGetActions(ContextActionList* actions)
{
	actions->names[actions->count] = memcpy(malloc(13), "Image Viewer", 13);
	actions->functions[actions->count] = &FortmatVitaGxtViewImage;
	actions->count++;
}

/*************************************************************************************************/

FileFormat formatVitaGxt = {
	"Vita GXT Image",
	FILE_TYPE_IMAGE,
	ICON_CUSTOM_FILE_IMAGE,
	&FormatIsVitaGxt,
	&FormatVitaGxtGetActions
};