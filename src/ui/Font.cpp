#include "ui/Font.h"
#include "io/Resource.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "Dolphin/OS/OSCache.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "core/TMainTask.h"
#include "core/SdkCalls.h"

TArc* sFontArc;          // 0x802C4F68
FontData* sFontData;        // 0x802C4F6C
u16 sFontDefaultBlock;      // 0x802C4F70
u16 sFontDefaultIndex;      // 0x802C4F72
FontCell* sFontCells;       // 0x802C4F74
FontCacheEntry* sFontCache; // 0x802C4F78
int sFontDirty;             // 0x802C4F7C
NJS_TEXLIST sFontTexList;   // 0x802C4F80
u8* sFontTexBuf;            // 0x802C4F88
u32 sFontTexSize;           // 0x802C4F8C
u8* sFontWork;              // 0x802C4F90 one glyph at 4 bits a texel 24 by 24
NJS_TEXINFO sFontTexInfo;   // 0x8027FFD0
NJS_TEXNAME sFontTexName;   // 0x8027FFF4

// 0x8006878C
extern "C" int fontLookup(u32 code, u16* outBlock, u16* outIndex)
{
	FontData* data = sFontData;
	if (data == 0) {
		return 0;
	}
	int found = 0;
	for (int i = 0; i < data->mCount; i++) {
		FontBlock* block = data->mBlocks[i];
		u16 first        = block->mFirst;
		u16 last         = block->mLast;
		if (first <= (u16)code && (u16)code <= last) {
			if (block->mFlags & 1) {
				u16 row  = first & 0xFF00;
				u16 base = -(first - (row + 0x40));
				if ((u8)first >= 0x7F) {
					base++;
				}
				u32 rel = code - row;
				base    = base + ((rel >> 8) & 0xFF) * 0xBC;
				u8 col  = rel;
				if (col >= 0x7F) {
					base--;
				}
				found     = 1;
				*outIndex = col + base - 0x40;
				*outBlock = i;
			} else {
				FontRange* range = block->mRanges;
				u16 base         = 0;
				for (int j = 0; j < block->mNumRanges; j++, range++) {
					if (range->mFirst <= (u16)code && (u16)code <= range->mLast) {
						found     = 1;
						*outIndex = base + (code - range->mFirst);
						*outBlock = i;
						break;
					}
					base = base + (range->mLast - range->mFirst) + 1;
				}
			}
			return found;
		}
	}
	return 0;
}

// 0x800688D0
extern "C" int fontFind(u32 code, u16* outBlock, u16* outIndex)
{
	u16 c     = code;
	*outBlock = sFontDefaultBlock;
	*outIndex = sFontDefaultIndex;
	if (code & 0xFF00) {
		u8 col = c;
		if (col < 0x40 || col == 0x7F || col > 0xFC) {
			return 0;
		}
	}
	return fontLookup(code, outBlock, outIndex);
}

// 0x80068930
// Cells are 3 by 3 tiles of 8 by 8 texels 32 bytes a tile 32 tiles a row
extern "C" u8* fontCellAddr(u8* tex, int x, int y)
{
	return tex + ((x * 3) + ((y * 3) << 5) << 5);
}

// 0x8006894C
extern "C" u8* fontBitmap(u16 block, u16 index)
{
	return sFontData->mBlocks[block]->mBitmaps + index * 0x84;
}

// 0x80068970
// Two bits a texel to one byte a texel
extern "C" void fontExpand(u8* src, int n)
{
	u8* dst = sFontWork;
	u8 bits;
	for (int i = 0; i < n; i++) {
		bits = *src++;
		for (int k = 0; k < 4; k++) {
			if ((u8)(bits & 0xC0) == 0xC0) {
				*dst++ = 15;
			} else if ((u8)(bits & 0xC0) == 0x80) {
				*dst++ = 10;
			} else if ((u8)(bits & 0xC0) == 0x40) {
				*dst++ = 6;
			} else {
				*dst++ = 0;
			}
			bits <<= 2;
		}
	}
}

// 0x800689FC
extern "C" void fontDrawCell(u8* src, u32* dst)
{
	int k;
	int row;
	u32 word;
	u8* line;
	u32* out;
	u8* work;
	u32* tile;
	int ty;
	int tx;
	fontExpand(src, 0x84);
	work = sFontWork;
	for (ty = 0; ty < 3; ty++) {
		tile = dst;
		for (tx = 0; tx < 3; tx++) {
			out = tile;
			line = work + tx * 8;
			for (row = 0; row < 8; row++) {
				word = 0;
				for (k = 0; k < 8; k++) {
					word <<= 4;
					word |= line[k] & 0xF;
				}
				*out++ = word;
				line += 24;
			}
			tile += 8;
		}
		dst += 0x100;
		work += 24 * 8;
	}
}

// 0x80068ABC
// The entry already holding the glyph or the first free one or the hint
extern "C" int fontFindSlot(u16 block, u16 index, int hint)
{
	int slot;
	if ((sFontCache[hint].mFlags & 1) && !sFontCache[hint].is(block, index)) {
		slot = -1;
		for (int i = 0; i < 100; i++) {
			if (!(sFontCache[i].mFlags & 1)) {
				hint = i;
				for (; i < 100; i++) {
					if (sFontCache[i].is(block, index)) {
						slot = i;
						break;
					}
				}
				break;
			}
			if (sFontCache[i].is(block, index)) {
				slot = i;
				break;
			}
		}
		if (slot == -1) {
			slot = hint;
		}
	} else {
		slot = hint;
	}
	if (slot <= -1) {
		slot = 0;
	}
	return slot;
}

// 0x80068C08
extern "C" void fontFillGlyph(int slot, FontCacheEntry* e, FontGlyph* glyph)
{
	glyph->mSlot  = slot;
	glyph->mBlock = e->mBlock;
	glyph->mIndex = e->mIndex;
	glyph->_10    = 0;
	FontBlock* block = sFontData->mBlocks[e->mBlock];
	FontCell* cell   = &sFontCells[e->mCell];
	if (block->mWidths != 0) {
		f32 width     = block->mWidths[e->mIndex];
		glyph->mWidth = width;
		glyph->mU0    = cell->mU;
		glyph->mU1    = cell->mU + (int)(22.0f * (width / 22.0f));
		glyph->mV1    = cell->mV + 22;
	} else {
		glyph->mWidth = 22.0f;
		glyph->mU0    = cell->mU;
		glyph->mU1    = cell->mU + 22;
	}
	glyph->mHeight = 22.0f;
	glyph->mV0     = cell->mV;
	glyph->mV1     = cell->mV + 22;
}

// 0x80068CF8
extern "C" void fontLoadGlyph(u16 block, u16 index, FontGlyph* glyph)
{
	int slot          = fontFindSlot(block, index, index % 100);
	FontCacheEntry* e = &sFontCache[slot];
	if (!e->is(block, index)) {
		e->mFlags |= 2;
		sFontDirty = 1;
		e->mRefs   = 0;
	}
	e->mFlags |= 1;
	e->mBlock = block;
	e->mIndex = index;
	e->mRefs++;
	fontFillGlyph(slot, e, glyph);
}

// 0x80068DC0
extern "C" void fontInit()
{
	sFontArc  = 0;
	sFontData = 0;
	loadArc("fontdat.bin", &sFontArc);
	if (sFontArc != 0) {
		sFontData = (FontData*)sFontArc->mData;
	}

	FontCell* cell = sFontCells = new FontCell[100];
	for (int y = 0; y < 10; y++) {
		int v = (s16)(y * 24);
		for (int x = 0; x < 10; x++) {
			cell->mU = x * 24;
			cell->mV = v;
			cell++;
		}
	}

	sFontCache = new FontCacheEntry[100];
	fontClearCache();

	sFontWork = new u8[0x240];
	memset(sFontWork, 0, 0x240);
	DCFlushRange(sFontWork, 0x240);

	sFontTexSize = GXGetTexBufferSize(256, 256, 8, GX_FALSE, 0);
	u8* buf      = new u8[sFontTexSize];
	njSetTextureInfo(&sFontTexInfo, buf, 0x208, 256, 256);
	njSetTextureName(&sFontTexName, (void*)&sFontTexInfo, (void*)0x101A, (u32)0x40800000);
	sFontTexList.textures  = &sFontTexName;
	sFontTexList.nbTexture = 1;
	njLoadTexture(&sFontTexList);
	sFontTexBuf = (u8*)sFontTexList.textures->texaddr->hdr->info.image;
	delete[] buf;
	fontClearTexture();

	int n;
	u16 code = 0x81A6;
	for (n = 200; n >= 0; n--, code++) {
		if (fontLookup(code, &sFontDefaultBlock, &sFontDefaultIndex)) {
			break;
		}
	}
	if (n < 0) {
		sFontDefaultBlock = 0;
		sFontDefaultIndex = 0;
	}
}

// 0x80068F70
extern "C" void fontCreateTask()
{
	new TFontTask(gTL_08);
}

// 0x80068FBC
extern "C" void fontClearCache()
{
	u16 cell          = 0;
	FontCacheEntry* e = sFontCache;
	for (int y = 0; y < 10; y++) {
		for (int x = 0; x < 10; x++) {
			e->mBlock = 0xFFFF;
			e->mIndex = 0xFFFF;
			e->mCell  = cell++;
			e->mFlags = 0;
			e->mRefs  = 0;
			e++;
		}
	}
}

// 0x8006900C
extern "C" void fontClearTexture()
{
	GXInvalidateTexAll();
	u32* word = (u32*)sFontTexBuf;
	int n;
	for (n = (int)sFontTexSize >> 2; n > 0; n--) {
		*word++ = 0;
	}
	u8* byte = (u8*)word;
	for (n = sFontTexSize & 3; n > 0; n--) {
		*byte++ = 0;
	}
	DCStoreRange(sFontTexBuf, sFontTexSize);
}

// 0x8006907C
extern "C" void fontReset()
{
	fontClearCache();
	fontClearTexture();
}

// 0x800690A0
extern "C" int fontIsLead(int c)
{
	u8 lead = 0;
	if (((u8)c >= 0x80 && (u8)c <= 0x9F) || ((u8)c >= 0xE0 && (u8)c <= 0xEF)) {
		lead = 1;
	}
	return lead;
}

// 0x800690D8
extern "C" void fontGetGlyph(u32 code, FontGlyph* glyph)
{
	u16 block;
	u16 index;
	fontFind(code, &block, &index);
	fontLoadGlyph(block, index, glyph);
}

// 0x8006911C
extern "C" void fontReleaseGlyph(FontGlyph* glyph)
{
	FontCacheEntry* e = &sFontCache[glyph->mSlot];
	if (e->is(glyph->mBlock, glyph->mIndex)) {
		e->mRefs--;
		if (e->mRefs <= 0) {
			if (e->mFlags & 2) {
				e->mBlock = 0xFFFF;
				e->mIndex = 0xFFFF;
			}
			e->mFlags = 0;
			e->mRefs  = 0;
		}
	}
}

// 0x800691A0
extern "C" void fontSetTexture()
{
	njSetTexture(&sFontTexList);
}

// 0x800691C4
extern "C" void fontFlush()
{
	u32* dst;
	FontCacheEntry* e;
	int y;
	int x;

	if (sFontDirty != 0) {
		sFontDirty        = 0;
		e = sFontCache;
		for (y = 0; y < 10; y++) {
			for (x = 0; x < 10; x++, e++) {
				if (e->mFlags & 2) {
					dst = (u32*)fontCellAddr(sFontTexBuf, x, y);
					fontDrawCell(fontBitmap(e->mBlock, e->mIndex), dst);
					e->mFlags &= ~2;
				}
			}
		}
	}
	DCStoreRange(sFontTexBuf, sFontTexSize);
}

// 0x80069274
TFontTask::~TFontTask() { }

// 0x800692D0
void TFontTask::draw()
{
	fontFlush();
}
