#ifndef _FONT_H
#define _FONT_H

#include "stage/Stage.h"

struct FontGlyph {
	int mSlot;   // _00 the cache entry
	u16 mBlock;  // _04
	u16 mIndex;  // _06
	f32 mWidth;  // _08
	f32 mHeight; // _0C
	s16 _10;     // _10
	s16 mU0;     // _12 in texels of the 256 by 256 texture
	s16 mU1;     // _14
	s16 mV0;     // _16
	s16 mV1;     // _18
};

/** Codes first..last of a block without the Shift-JIS flag. @size 0x4 */
struct FontRange {
	u16 mFirst; // _00
	u16 mLast;  // _02
};

/** @size 0x14 */
struct FontBlock {
	s16 mNumRanges;     // _00
	u16 mFlags;         // _02 bit 0 the codes run as Shift-JIS rows of 0xBC
	u16 mFirst;         // _04
	u16 mLast;          // _06
	FontRange* mRanges; // _08
	u8* mBitmaps;       // _0C 0x84 bytes a glyph
	u8* mWidths;        // _10 or null for fixed width
};

/** What fontdat.bin holds */
struct FontData {
	int mCount;          // _00
	FontBlock** mBlocks; // _04
};

/** One of the 100 texture cells. @size 0xC */
struct FontCacheEntry {
	bool is(u16 block, u16 index)
	{
		u8 same = 0;
		if (mBlock == block && mIndex == index) {
			same = 1;
		}
		return same;
	}

	u16 mBlock; // _00 0xFFFF when empty
	u16 mIndex; // _02
	u16 mCell;  // _04
	s16 mRefs;  // _06
	u16 mFlags; // _08 bit 0 in use bit 1 needs drawing
	u8 _0A[2];
};

/** Top left of a cell in the font texture in texels */
struct FontCell {
	s16 mU; // _00
	s16 mV; // _02
};

/** Draws the new glyphs into the texture once a frame. @size 0x20 */
class TFontTask : public TObject {
public:
	TFontTask(TObject* parent)
	    : TObject(parent)
	{
	}
	virtual ~TFontTask();
	virtual void draw();
};

extern "C" {
int fontLookup(u32 code, u16* outBlock, u16* outIndex);  // 0x8006878C
int fontFind(u32 code, u16* outBlock, u16* outIndex);    // 0x800688D0 falls back to a default glyph
u8* fontCellAddr(u8* tex, int x, int y);                 // 0x80068930
u8* fontBitmap(u16 block, u16 index);                    // 0x8006894C
void fontExpand(u8* src, int n);                         // 0x80068970
void fontDrawCell(u8* src, u32* dst);                    // 0x800689FC
int fontFindSlot(u16 block, u16 index, int hint);        // 0x80068ABC
void fontFillGlyph(int slot, FontCacheEntry* e, FontGlyph* glyph); // 0x80068C08
void fontLoadGlyph(u16 block, u16 index, FontGlyph* glyph); // 0x80068CF8
void fontInit();                                         // 0x80068DC0
void fontCreateTask();                                   // 0x80068F70
void fontClearCache();                                   // 0x80068FBC
void fontClearTexture();                                 // 0x8006900C
void fontReset();                                        // 0x8006907C
int fontIsLead(int c);                                   // 0x800690A0 nonzero if c starts a two byte character
void fontGetGlyph(u32 code, FontGlyph* glyph);           // 0x800690D8
void fontReleaseGlyph(FontGlyph* glyph);                 // 0x8006911C
void fontSetTexture();                                   // 0x800691A0
void fontFlush();                                        // 0x800691C4

void textDrawBegin(int mode);                            // 0x80069448
void textDrawSet(f32 depth, u32 color);                  // 0x800694D8
void textSetDepth(f32 depth);                            // 0x8006950C
void textSetColor(u32 color);                            // 0x80069514
void textDrawQuad(s16 u0, s16 v0, s16 u1, s16 v1, f32 x, f32 y, f32 w, f32 h); // 0x8006951C
void textDrawEnd();                                      // 0x80069634
void tlutInit();                                         // 0x80069648
}

#endif
