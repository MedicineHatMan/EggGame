#include "ui/Font.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "Dolphin/OS/OSCache.h"
#include "core/SdkCalls.h"

#define GX_FIFO_F32 (*(volatile f32*)0xCC008000)
#define GX_FIFO_U32 (*(volatile u32*)0xCC008000)
#define GX_FIFO_S16 (*(volatile s16*)0xCC008000)

f32 sTextDepth;                         // 0x802C4F98
u32 sTextColor;                         // 0x802C4F9C
GXTlutRegion* sTlutRegions;             // 0x802C4FA0
GXTlutRegion* (*sOldTlutCallback)(u32); // 0x802C4FA4

// 0x80069448
extern "C" void textDrawBegin(int mode)
{
	njDrawTexture2DExStart(mode);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
}

// 0x800694D8
extern "C" void textDrawSet(f32 depth, u32 color)
{
	textSetDepth(depth);
	textSetColor(color);
}

// 0x8006950C
extern "C" void textSetDepth(f32 depth)
{
	sTextDepth = depth;
}

// 0x80069514
extern "C" void textSetColor(u32 color)
{
	sTextColor = color;
}

// 0x8006951C
extern "C" void textDrawQuad(s16 u0, s16 v0, s16 u1, s16 v1, f32 x, f32 y, f32 w, f32 h)
{
	GXBegin(GX_QUADS, GX_VTXFMT0, 4);
	u32 c;
	f32 z = sTextDepth;
	x *= z;
	w *= z;
	y *= z;
	h *= z;
	f32 x1 = x + w;
	f32 y1 = y + h;

	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
	c = sTextColor;
	GX_FIFO_U32 = c;
	GX_FIFO_S16 = u0;
	GX_FIFO_S16 = v0;

	GX_FIFO_F32 = x1;
	GX_FIFO_F32 = y;
	GX_FIFO_F32 = z;
	GX_FIFO_U32 = c;
	GX_FIFO_S16 = u1;
	GX_FIFO_S16 = v0;

	GX_FIFO_F32 = x1;
	GX_FIFO_F32 = y1;
	GX_FIFO_F32 = z;
	GX_FIFO_U32 = c;
	GX_FIFO_S16 = u1;
	GX_FIFO_S16 = v1;

	GX_FIFO_F32 = x;
	GX_FIFO_F32 = y1;
	GX_FIFO_F32 = z;
	GX_FIFO_U32 = c;
	GX_FIFO_S16 = u0;
	GX_FIFO_S16 = v1;
}

// 0x80069634
extern "C" void textDrawEnd() { }

// 0x80069638
extern "C" GXTlutRegion* tlutRegionCallback(u32 index)
{
	return &sTlutRegions[index];
}

// 0x80069648
// TLUTs 0-15 are 16 colours 16-30 are 256 and 31-34 are 1024
/** An opaque RGB5A3 palette entry the top bit set then 5 bits each of red green and blue */
#define TLUT_RGB(r, g, b) ((((r) & 0x1F) << 10) | (((g) & 0x1F) << 5) | ((b) & 0x1F) | 0x8000)

extern "C" void tlutInit()
{
	sTlutRegions = new GXTlutRegion[35];
	u32 addr             = 0xC0000;
	GXTlutRegion* region = sTlutRegions;
	for (int i = 0; i < 16; i++, addr += 0x200, region++) {
		GXInitTlutRegion(region, addr, GX_TLUT_16);
	}
	for (int i = 0; i < 15; i++, addr += 0x2000, region++) {
		GXInitTlutRegion(region, addr, GX_TLUT_256);
	}
	for (int i = 0; i < 4; i++, addr += 0x8000, region++) {
		GXInitTlutRegion(region, addr, GX_TLUT_1K);
	}
	sOldTlutCallback = GXSetTlutRegionCallback(tlutRegionCallback);

	u16* pal = new u16[0x400];
	GXTlutObj obj;
	u16* p;
	int c;

	pal[0] = 0;
	p      = pal + 1;
	for (c = 1; c < 0x100; c++) {
		*p++ = TLUT_RGB((u32)(((c >> 5) & 7) * 31) / 7, (u32)(((c >> 2) & 7) * 31) / 7, (u32)((c & 3) * 31) / 3);
	}
	DCFlushRange(pal, 0x800);
	for (int i = 0; i < 15; i++) {
		GXInitTlutObj(&obj, pal, GX_TL_RGB5A3, 0x100);
		GXLoadTlut(&obj, i + 16);
	}

	pal[0] = 0;
	p      = pal + 1;
	for (c = 1; c < 0x400; c++) {
		*p++ = TLUT_RGB((u32)(((c >> 7) & 7) * 31) / 7, (u32)(((c >> 3) & 0x15) * 31) / 15, (u32)((c & 7) * 31) / 7);
	}
	DCFlushRange(pal, 0x800);
	for (int i = 0; i < 4; i++) {
		GXInitTlutObj(&obj, pal, GX_TL_RGB5A3, 0x400);
		GXLoadTlut(&obj, i + 31);
	}

	p = pal;
	for (int i = 0; i < 16; i++) {
		*p++ = 0;
		for (int k = 1; k < 16; k++) {
			int v    = (k * 31) / 16;
			u32 grey = v & 0x1F;
			*p++     = TLUT_RGB(grey, grey, grey);
		}
	}
	int i;
	u16* tlut = pal;
	pal[0x0F] = 0xFFFF;
	pal[0x1F] = 0x8000;
	pal[0x2F] = 0xFC00;
	pal[0x3F] = 0x801F;
	pal[0x4F] = 0xFFE0;
	pal[0x5F] = 0xFC1F;
	pal[0x6F] = 0x83E0;
	pal[0x7F] = 0xFE8A;
	DCFlushRange(pal, 0x800);
	for (i = 0; i < 16; i++, tlut += 16) {
		GXInitTlutObj(&obj, tlut, GX_TL_RGB5A3, 0x10);
		GXLoadTlut(&obj, i);
	}
	if (pal != 0) {
		delete[] pal;
	}
}
