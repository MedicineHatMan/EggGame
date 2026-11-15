#include "ui/TextTexture.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "Ninja/ninja.h"
#include "egg/EggCreature.h"
#include "Dolphin/OS/OSFont.h"
#include "core/Mem.h"
#include "core/dolphin.h"
#include "ui/AeArc.h"
#include "core/SdkCalls.h"

#define OS_FONT_ENCODE_SJIS 1

static OSFontHeader* sFont;
static u32 sFontSheetWidth;
static u32 sFontCellWidth;
static u32 sFontCellHeight;
static u16 sFontFormat;
static int sFontOpen;

// 0x8019DBC0
// Keeps the font in memory until fontClose
void fontOpen()
{
	fontLoad();
	sFontOpen = 1;
}

// 0x8019DBE8
void fontClose()
{
	fontFree();
	sFontOpen = 0;
}

// 0x8019DC10
void fontLoad()
{
	if (sFont == 0) {
		void* temp;
		if (fontIsSjis() != 0) {
			sFont = (OSFontHeader*)memAlloc(0x90EE4);
			temp  = memAlloc(0x4D000);
		} else {
			sFont = (OSFontHeader*)memAlloc(0x10120);
			temp  = memAlloc(0x3000);
		}
		OSLoadFont(sFont, temp);
		memFree(temp);
	}
	sFontSheetWidth = sFont->sheetWidth;
	sFontCellWidth  = sFont->cellWidth;
	sFontCellHeight = sFont->cellHeight;
	sFontFormat     = sFont->sheetFormat;
}

// 0x8019DCC4
void fontFree()
{
	if (sFont != 0) {
		memFree(sFont);
		sFont = 0;
	}
}

// 0x8019DCF8
int fontIsSjis()
{
	return OSGetFontEncode() == OS_FONT_ENCODE_SJIS;
}

// 0x8019DD28
TextTexture* createTextTexture(int cols, int rows, u32 globalIndex)
{
	return new TextTexture(cols, rows, globalIndex);
}

// 0x8019DD80
TextTexture::TextTexture(int cols, int rows, u32 globalIndex)
{
	if (cols <= 0 || rows <= 0) {
		return;
	}
	mWidth     = cols * 8;
	mHeight    = rows * 8;
	mTilesW    = (mWidth + 7) / 8;
	mImageSize = GXGetTexBufferSize(mWidth, mHeight, sFontFormat, GX_FALSE, 0);
	u8* temp   = new u8[mImageSize];
	njSetTextureInfo(&mInfo, temp, sFontFormat, mWidth, mHeight);
	njSetTextureName(&mName, (void*)&mInfo, (void*)globalIndex, (u32)0x40800000);
	mList.textures  = &mName;
	mList.nbTexture = 1;
	njLoadTexture(&mList);
	mImage = (u8*)mList.textures->texaddr->hdr->info.image;
	delete[] temp;
	clearAll();
}

// 0x8019DE84
void TextTexture::clear(u32 col, u32 row, u32 cols, u32 rows)
{
	GXInvalidateTexAll();
	u32 x = col * 8;
	u32 y = row * 8;
	if (x >= mWidth || y >= mHeight) {
		return;
	}
	u32 w = cols;
	if (x + cols * 8 >= mWidth) {
		w = (mWidth - x + 7) >> 3;
	}
	u32 h = rows;
	if (y + rows * 8 >= mHeight) {
		h = (mHeight - y + 7) >> 3;
	}
	u8* line = getAddr(mImage, x, y, mWidth);
	u8* p;
	for (u32 j = 0; j < h; j++) {
		p = line;
		for (u32 i = 0; i < w; i++) {
			syMemSet(p, 0, 0x20);
			p += 0x20;
		}
		line += mTilesW * 0x20;
	}
	DCStoreRange(mImage, mImageSize);
}

// 0x8019DF8C
void TextTexture::clearAll()
{
	clear(0, 0, (mWidth + 7) / 8, (mHeight + 7) / 8);
}

// 0x8019DFD4
void TextTexture::setTexture()
{
	njSetTexture(&mList);
}

// 0x8019DFF8
// Steps over a line break if text starts with one
int TextTexture::skipNewline(const char** text)
{
	const char* breaks[3] = { "\r\n", "\n", "\r" };
	u32 lengths[3]        = { 2, 1, 1 };
	u32 n;
	int i;
	for (i = 0; i < 3; i++) {
		if (strncmp(*text, breaks[i], n = lengths[i]) == 0) {
			*text += n;
			return 1;
		}
	}
	return 0;
}

// 0x8019E0B4
// Where pixel x y is in a 4 bit image that is width pixels wide laid out in rows
u8* TextTexture::getAddr(u8* base, int x, int y, int width)
{
	return base + (x + 1) / 2 + y * ((width + 7) / 8) * 4;
}

// 0x8019E0E0
void TextTexture::print(int col, int row, const char* text)
{
	if (sFontOpen == 0) {
		fontLoad();
	}
	if (sFont == 0) {
		return;
	}

	int x    = col * 8;
	int y    = row * 8;
	int maxY = mHeight - sFontCellHeight;
	if (col < 0 || row < 0 || (skipNewline(&text) != 0 && row < maxY)) {
		return;
	}
	if (untile(mImage, mImage, mWidth / 8, mHeight / 8) == 0) {
		return;
	}

	while (*text != '\0') {
		u8 glyph[0x240];
		s32 width;
		memset(glyph, 0, sizeof(glyph));
		text = OSGetFontTexel((char*)text, glyph, 0, 6, &width);
		if (untile(glyph, glyph, 3, 3) == 0) {
			break;
		}
		if (x + ((width + 7) / 8) * 8 > mWidth) {
			x = 0;
			y += sFontCellHeight;
			if (y > maxY) {
				break;
			}
		}
		u8* dst = getAddr(mImage, x, y, mWidth);
		blitGlyph(dst, getAddr(glyph, 0, 0, 24), width, x % 2 == 1);
		x += width;
		while (skipNewline(&text) != 0) {
			x = 0;
			y += sFontCellHeight;
		}
		if (y > maxY) {
			break;
		}
	}

	tile(mImage, mImage, mWidth / 8, mHeight / 8);
	DCStoreRange(mImage, mImageSize);
	if (sFontOpen == 0) {
		fontClose();
	}
}

// 0x8019E31C
// ORs 24 rows of a glyph in half a byte along when x is odd
#pragma opt_propagation off
void TextTexture::blitGlyph(u8* dst, const u8* src, int width, int odd)
{
	int perByte;
	int row;
	int bytes;
	int i;
	perByte = 2;
	bytes   = (width + 1) / 2;
	for (row = 0; row < 24; row++) {
		if (perByte > 1) {
			i = 0;
			if (odd) {
				const u8* s = src;
				for (; i < bytes; i++) {
					u8 c = *s++;
					dst[i - 1] |= (c >> 4) & 0x0F;
					dst[i] |= (c << 4) & 0xF0;
				}
			} else {
				const u8* s = src;
				for (; i < bytes; i++) {
					u8 c = *s++;
					dst[i] |= c & 0xF0;
					dst[i] |= c & 0x0F;
				}
			}
		} else {
			syMemCopy(dst, src, width);
		}
		dst += mTilesW * 4;
		src += (((int)sFontCellWidth + 7) / 8) * 4;
	}
}

#pragma opt_propagation reset

// 0x8019E448
// 8x8 4 bit tiles to plain rows one row of tiles at a time
int TextTexture::untile(u8* dst, u8* src, u32 tilesW, u32 tilesH)
{
	u32 size = tilesW * 0x20;
	u8* buf  = (u8*)memAlloc(size);
	if (buf == 0) {
		return 0;
	}
	for (u32 ty = 0; ty < tilesH; ty++) {
		syMemSet(buf, 0, size);
		u32 base = ty * tilesW;
		for (u32 tx = 0; tx < tilesW; tx++) {
			for (int y = 0; y < 8; y++) {
				for (int b = 0; b < 4; b++) {
					syMemCopy(buf + (tx + y * tilesW) * 4 + b, src + (tx + base) * 0x20 + y * 4 + b, 1);
				}
			}
		}
		syMemCopy(dst + size * ty, buf, size);
	}
	memFree(buf);
	return 1;
}

// 0x8019E54C
// Plain rows back to 8x8 tiles
int TextTexture::tile(u8* dst, u8* src, u32 tilesW, u32 tilesH)
{
	u32 size = tilesW * 0x20;
	u8* buf  = (u8*)memAlloc(size);
	if (buf == 0) {
		return 0;
	}
	for (u32 ty = 0; ty < tilesH; ty++) {
		syMemSet(buf, 0, size);
		u32 base = ty * tilesW * 0x20;
		for (u32 tx = 0; tx < tilesW; tx++) {
			for (int y = 0; y < 8; y++) {
				for (int b = 0; b < 4; b++) {
					syMemCopy(buf + tx * 0x20 + y * 4 + b, src + base + (tx + y * tilesW) * 4 + b, 1);
				}
			}
		}
		syMemCopy(dst + size * ty, buf, size);
	}
	memFree(buf);
	return 1;
}
