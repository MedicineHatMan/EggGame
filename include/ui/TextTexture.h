#ifndef _TEXTTEXTURE_H
#define _TEXTTEXTURE_H

#include "core/types.h"
#include "Ninja/ninja.h"

/**
 * @size 0x4C
 */
struct TextTexture {
	TextTexture(int cols, int rows, u32 globalIndex);

	void clear(u32 col, u32 row, u32 cols, u32 rows);
	void clearAll();
	void setTexture();
	int skipNewline(const char** text);
	u8* getAddr(u8* base, int x, int y, int width);
	void print(int col, int row, const char* text);
	void blitGlyph(u8* dst, const u8* src, int width, int odd);
	int untile(u8* dst, u8* src, u32 tilesW, u32 tilesH);
	int tile(u8* dst, u8* src, u32 tilesW, u32 tilesH);

	int mWidth;          // _00 in pixels
	int mHeight;         // _04
	int mTilesW;         // _08 8 pixel tiles in a row
	NJS_TEXINFO mInfo;   // _0C
	NJS_TEXNAME mName;   // _30
	NJS_TEXLIST mList;   // _3C
	u8* mImage;          // _44 the loaded texture's pixels
	u32 mImageSize;      // _48
};

extern "C" {
void fontOpen();
void fontClose();
void fontLoad();
void fontFree();
int fontIsSjis();
}

TextTexture* createTextTexture(int cols, int rows, u32 globalIndex);

#endif
