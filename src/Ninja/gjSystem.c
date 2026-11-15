#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"

typedef struct GjSysParams {
	GXRenderModeObj* rmode; // _00 null picks one from the TV format
	s32 _04;                // _04 passed on and never used
	void* frameA;           // _08
	void* frameB;           // _0C
	void* fifo;             // _10
	u32 fifoSize;           // _14
	void* texHeap;          // _18
	u32 texHeapSize;        // _1C
} GjSysParams;

typedef struct NjSinCos {
	f32 sin;
	f32 cos;
} NjSinCos;

NJS_CONTEXT gjContexts[6] __attribute__((aligned(8))); // 0x802A5860 8 bytes past the end of gjRenderModeCopy's 0x3C
GXRenderModeObj gjRenderModeCopy;
GXTlutRegion gjTlutRegions[0x50];
#if 0 // GXFrameBuf.c owns these
GXRenderModeObj GXNtsc480IntDf = { 0, 640, 480, 480, 40, 0, 640, 480, 1, 0, 0,
	{ { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },
	{ 8, 8, 10, 12, 10, 8, 8 } }; // NTSC
GXRenderModeObj GXPal528IntDf = { 4, 640, 528, 528, 40, 23, 640, 528, 1, 0, 0,
	{ { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },
	{ 8, 8, 10, 12, 10, 8, 8 } }; // PAL
GXRenderModeObj GXMpal480IntDf = { 8, 640, 480, 480, 40, 0, 640, 480, 1, 0, 0,
	{ { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 }, { 6, 6 } },
	{ 8, 8, 10, 12, 10, 8, 8 } }; // MPAL
#endif
extern GXRenderModeObj GXNtsc480IntDf;
extern GXRenderModeObj GXPal528IntDf;
extern GXRenderModeObj GXMpal480IntDf;

// Not defined here the DOL keeps these in .sbss blocks that come before njTexture.c's lbl_802C6140 before njPrint's
// gjOneOver65535 after so other data-only files own them
extern void (*gjSwapCallback)(void); // called after each swap
extern void* gjFifo;         // the GX fifo object
extern void* gjFifo2;
extern u32 gjFifoSize;           // fifo size
extern void* gjFifoMem;         // fifo memory
extern u32 gjLastRetrace;           // retrace count at the last swap
extern u32 gjRetracesPerFrame;           // retraces per frame
extern void* gjFrameDrawing;         // the frame being drawn
extern void* gjFrameB;         // frame B
extern void* gjFrameA;         // frame A
extern s32 lbl_802C6140;
extern f32 gjOneOver65535;
u8 gjScreenBlanked = 1;            // the screen is still blanked

void gjSysSetRenderMode(GXRenderModeObj* rmode, s32 unused);
void gjSysSetBuffers(GjSysParams* params);
void gjSysInitTluts(void);
void gjShowFirstFrame(void);
void gjFlip(void);
void gjSysDefaults(void);
void gjFitRenderMode(GXRenderModeObj* rmode, GXRenderModeObj* out);
void njBuildSinTable(void);
void gjContextInit(NJS_CONTEXT* ctx);
void gjSelectContext(s32 index);
GXTlutRegion* gjTlutRegionCb(u32 tlut);

// 0x80214980
// Picks which of the six contexts is current
void gjSelectContext(s32 index)
{
	_nj_current_ctx_ = &gjContexts[index];
}

// 0x80214998
void gjContextInit(NJS_CONTEXT* ctx)
{
	ctx->_00 = 0x808C0000;
	ctx->tex = NULL;
	ctx->blend = 0x949004C0;
	ctx->_10 = 0x80012;
	ctx->_14 = -1;
	ctx->_18 = -1;
	ctx->_1C = 0x20000000;
	ctx->texlist = NULL;
}

// 0x802149DC
// The top three bits of blend are the source factor the next three the destination
void njColorBlendingMode(s32 target, s32 mode)
{
	s32 factors[12] = { 0xFF, 0xFF, 6, 2, 7, 3, 5, 3, 4, 2, 1, 0 };

	if (mode == 0) {
		_nj_current_ctx_->blend = (_nj_current_ctx_->blend & 0x03FFFFFF) | 0xB0000000;
	} else if (mode == 0) { // tests 0 again in the DOL a bare bne on the first compare so this is dead
		_nj_current_ctx_->blend = (_nj_current_ctx_->blend & 0x03FFFFFF) | 0x94000000;
	} else if (target == 0) {
		_nj_current_ctx_->blend = (_nj_current_ctx_->blend & 0x1FFFFFFF) | (factors[mode] << 29);
	} else {
		_nj_current_ctx_->blend = (_nj_current_ctx_->blend & 0xE3FFFFFF) | (factors[mode] << 26);
	}
}

// 0x80214AE0
void gjGXSetBlendMode(s32 enable)
{
	u32 blend = _nj_current_ctx_->blend;
	s32 src = gjBlendFactorTable[(blend & 0xE0000000) >> 29];
	s32 dst = gjBlendFactorTable[(blend & 0x1C000000) >> 26];
	if (enable != 0) {
		GXSetBlendMode(1, src, dst, 0);
	} else {
		GXSetBlendMode(0, src, dst, 0);
	}
}

// 0x80214B50
void njTextureClampMode(u32 mode)
{
	_nj_current_ctx_->blend = (_nj_current_ctx_->blend & 0xFFFE7FFF) | mode;
}

// 0x80214B68
void njPolygonCullingMode(u32 mode)
{
	_nj_current_ctx_->mode = (_nj_current_ctx_->mode & 0xE7FFFFFF) | mode;
}

// 0x80214B80
void gjSetPolygonCullingMode(void)
{
	s32 mode = _nj_current_ctx_->mode & 0x18000000;
	if (mode == 0) {
		GXSetCullMode(0);
	} else if (mode == 0x10000000) {
		GXSetCullMode(2);
	} else if (mode == 0x18000000) {
		GXSetCullMode(1);
	}
}

// 0x80214BE8
// Mode 1 has no break in the original so both modes end up with the same compare
void njZBufferMode(s32 mode)
{
	switch (mode) {
	case 1:
		_nj_current_ctx_->mode = (_nj_current_ctx_->mode & 0x1FFFFFFF) | 0xC0000000;
		break;
	case 0:
		_nj_current_ctx_->mode = (_nj_current_ctx_->mode & 0x1FFFFFFF) | 0xE0000000;
	}
}

// 0x80214C30
// Mode 0 has no break in the original so writing always ends up on
void njZWriteMode(s32 mode)
{
	switch (mode) {
	case 0:
		_nj_current_ctx_->mode |= 0x04000000;
		break;
	case 1:
		_nj_current_ctx_->mode &= ~0x04000000;
	}
}

// 0x80214C6C
void gjSetZBufferMode(s32 update)
{
	u32 mode = _nj_current_ctx_->mode;
	s32 func = (mode & 0xE0000000) == 0xE0000000 ? 7 : 1;
	if (mode & 0x04000000) {
		update = 0;
	}
	GXSetZMode(1, func, (u8)update);
}

// 0x80214CCC
void gjInitSystem(GjSysParams* params)
{
	gjSysSetRenderMode(params->rmode, params->_04);
	gjSysSetBuffers(params);
	VIConfigure(_gj_rmode_);
	gjSysInitTluts();
	gjShowFirstFrame();
}

// 0x80214D18
void gjBeforeRender(void)
{
	NJS_MATRIX scale;
	NJS_MATRIX trans;
	GXRenderModeObj* rmode = _gj_rmode_;

	if (rmode->field_rendering != 0) {
		GXSetViewportJitter(0.0f, 0.0f, rmode->fbWidth, rmode->xfbHeight, 0.0f, 1.0f, VIGetNextField());
	} else {
		GXSetViewport(0.0f, 0.0f, rmode->fbWidth, rmode->xfbHeight, 0.0f, 1.0f);
	}
	PSMTXScale(scale, -0.5f, -0.5f, 0.0f);
	PSMTXTrans(trans, 0.5f, 0.5f, 1.0f);
	PSMTXConcat(trans, scale, scale);
	GXLoadTexMtxImm(scale, 0x79, 0);
	GXInvalidateVtxCache();
	GXInvalidateTexAll();
}

// 0x80214E2C
void gjDoneRender(void)
{
	GXSetZMode(1, 3, 1);
	GXSetColorUpdate(1);
	GXCopyDisp(gjFrameDrawing, 1);
	GXDrawDone();
}

// 0x80214E70
void gjSwapBuffers(void)
{
	gjFlip();
}

// 0x80214E90
// Shows the finished frame holds to the frame rate and flips to the other buffer
void gjFlip(void)
{
	BOOL level;

	VISetNextFrameBuffer(gjFrameDrawing);
	if (gjScreenBlanked != 0) {
		VISetBlack(0);
		gjScreenBlanked = 0;
	}
	level = OSDisableInterrupts();
	while (VIGetRetraceCount() < gjLastRetrace + gjRetracesPerFrame - 1) {
		VIWaitForRetrace();
	}
	OSRestoreInterrupts(level);
	VIFlush();
	VIWaitForRetrace();
	gjLastRetrace = VIGetRetraceCount();
	if (gjSwapCallback != NULL) {
		gjSwapCallback();
	}
	gjFrameDrawing = (gjFrameDrawing == gjFrameA) ? gjFrameB : gjFrameA;
}

// 0x80214F48
void gjSysSetBuffers(GjSysParams* params)
{
	LCEnable();
	gjFrameA = params->frameA;
	gjFrameDrawing = gjFrameB = params->frameB;
	gjFifoMem = params->fifo;
	gjFifoSize = params->fifoSize;
	gjTexHeapHandle = OSCreateHeap(params->texHeap, (u8*)params->texHeap + params->texHeapSize);
	gjTexHeap = params->texHeap;
	gjTexHeapSize = params->texHeapSize;
}

// 0x80214FBC
void gjSysSetRenderMode(GXRenderModeObj* rmode, s32 unused)
{
	int flag;
	if (rmode != NULL) {
		_gj_rmode_ = rmode;
		flag = 1;
	} else {
		gjFitRenderMode(_gj_rmode_, &gjRenderModeCopy);
		_gj_rmode_ = &gjRenderModeCopy;
		flag = 1;
	}
	gjRetracesPerFrame = flag;
	gjLastRetrace = VIGetRetraceCount();
}

// 0x80215018
void gjSysInitTluts(void)
{
	s32 i;
	s32 index;
	u32 offset;
	GXTlutRegion* region;

	gjFifo = gjFifo2 = GXInit(gjFifoMem, gjFifoSize);
	GXSetViewport(0.0f, 0.0f, _gj_rmode_->fbWidth, _gj_rmode_->xfbHeight, 0.0f, 1.0f);
	GXSetScissor(0, 0, _gj_rmode_->fbWidth, _gj_rmode_->efbHeight);
	GXSetDispCopySrc(0, 0, _gj_rmode_->fbWidth, _gj_rmode_->efbHeight);
	GXSetDispCopyDst(_gj_rmode_->fbWidth, _gj_rmode_->xfbHeight);
	GXSetDispCopyYScale((f32)_gj_rmode_->xfbHeight / (f32)_gj_rmode_->efbHeight);
	GXSetCopyFilter(_gj_rmode_->aa, _gj_rmode_->sample_pattern, 1, _gj_rmode_->vfilter);
	if (_gj_rmode_->aa != 0) {
		GXSetPixelFmt(2, 0);
	} else {
		GXSetPixelFmt(0, 0);
	}

	offset = 0xC0000;
	index = 0;
	for (i = 0; i < 0x40; i++, index++) {
		GXInitTlutRegion(&gjTlutRegions[index], offset, 1);
		offset += 0x200;
	}
	for (i = 0; i < 0x10; i++, index++) {
		GXInitTlutRegion(&gjTlutRegions[index], offset, 0x10);
		offset += 0x2000;
	}
	GXSetTlutRegionCallback(gjTlutRegionCb);
	GXCopyDisp(gjFrameDrawing, 1);
}

// 0x802151FC
void gjShowFirstFrame(void)
{
	VISetNextFrameBuffer(gjFrameA);
	gjFrameDrawing = gjFrameB;
	VIFlush();
	VIWaitForRetrace();
	if ((u32)_gj_rmode_->viTVmode & 1) {
		VIWaitForRetrace();
	}
}

// 0x80215248
// The name is the PC build's see docs/notes.md
void njInitDevice(void)
{
	gjSysDefaults();
}

// 0x80215268
// Default screen and clip values the sine table and the contexts
void gjSysDefaults(void)
{
	NJS_CONTEXT* ctx;
	s32 i;

	njProjection2[7] = 1.0f;
	njProjection2[8] = 1.0f;
	njProjection2[9] = 1.0f;
	njProjection2[6] = 500.0f;
	njProjection2[0] = njProjection2[7] * njProjection2[6];
	njProjection2[1] = -(njProjection2[8] * njProjection2[6]);
	njProjection2[4] = 1.0f / njProjection2[0];
	njProjection2[5] = 1.0f / njProjection2[1];
	njProjection[0] = 60000.0f;
	njProjection[1] = 1.0f;
	_nj_control_3d_flag_ = 0;
	lbl_802C6140 = 0;
	gjOneOver65535 = 1.5259022e-05f;
	njBuildSinTable();
	for (i = 0; i < 6; i++) {
		gjContextInit(&gjContexts[i]);
	}
	gjSelectContext(0);
}

// 0x80215330
GXTlutRegion* gjTlutRegionCb(u32 tlut)
{
	return &gjTlutRegions[tlut];
}

// 0x80215344
u32 gjCalcFrameBufferSize(GXRenderModeObj* rmode)
{
	GXRenderModeObj local;
	if (rmode == NULL) {
		gjFitRenderMode(rmode, &local);
		rmode = &local;
	}
	return (u16)((rmode->fbWidth + 15) & ~15) * rmode->xfbHeight * 2;
}

// 0x80215390
// Fits the mode for the TV format to the overscan area
void gjFitRenderMode(GXRenderModeObj* rmode, GXRenderModeObj* out)
{
	switch (VIGetTvFormat()) {
	case 0:
		rmode = &GXNtsc480IntDf;
		break;
	case 1:
		rmode = &GXPal528IntDf;
		break;
	case 2:
		rmode = &GXMpal480IntDf;
		break;
	default:
		OSPanic("gjSystem.c", 0x33B, "invalid TV format\n");
		break;
	}
	GXAdjustForOverscan(rmode, out, 0, 0);
}

// 0x8021544C
// Builds the table for njSin and njCos at the bottom of the arena
void njBuildSinTable(void)
{
	s32 ang;
	NjSinCos* entry;
	s32 i;
	f32 rad;
	u8* table;

	table = OSGetArenaLo();
	OSSetArenaLo(table + 0x80000);
	njSinTable = table;
	entry = (NjSinCos*)table;
	for (i = 0, ang = 0; i < 0x10000; i++) {
		rad = 3.141592f * ang / 65536.0f;
		entry->sin = sin(rad);
		entry->cos = cos(rad);
		entry++;
		ang += 2;
	}
}
