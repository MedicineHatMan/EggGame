#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"
#include "Ninja/gjPrim.h"

u32 gjMatrixSlotTable[] = {
	0x00000000, 0x00000003, 0x00000006, 0x00000009, 0x0000000C, 0x0000000F, 0x00000012, 0x00000015,
	0x00000018, 0x0000001B,
}; // matrix slots
u32 gjTexSlotTable[] = {
	0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
	0x000000FF, 0x00000000,
}; // texture map slots
GXTexObj gjTexObj1;
GXTexObj gjTexObj0;
s8 gjFogEnabled;
GXColor gjFogColor;
f32 gjFogStart; // fog start
f32 gjFogEnd;   // fog end
u32 gjFogCurve; // fog curve

// asm symbols
void gjMtxGetTranspose();
f32 gjPrimConsts[2] = { 0.0f, 1.0f }; // only the one is read as in gjMath.c
void gjMtxWriteIdentity();
// end asm symbols

// 0x80216C6C
void gjDrawMotionLink(NJS_OBJECT* obj, void* link, f32 rate)
{
	njStartMotionLink(link, rate);
	gjDrawCalls->drawModel = (void (*)(void*))_gjDrawModel;
	gjBeginDrawModel();
	gjDrawListBegin(_gjDrawModel);
	gjDrawLinkObj(obj);
	gjEndDrawModel();
	gjDrawListFlush();
}

// 0x80216CCC
void gjDrawLinkObj(NJS_OBJECT* obj)
{
	do {
		njPushMatrixEx();
		njMotionLinkTransformEx(obj);
		if (gjObjectCallback != NULL) {
			gjObjectCallback(obj);
		}
		if (obj->model != NULL && (obj->evalflags & 8) == 0) {
			gjDrawListAdd(obj, _gj_matrix_);
		}
		if (obj->child != NULL) {
			gjDrawLinkObj(obj->child);
		}
		njPopMatrixEx();
		obj = obj->sibling;
	} while (obj != NULL);
}

// 0x80216D60
void gjMotionLinkActor(GjActor* actor, void* link, f32 rate)
{
	njStartMotionLink(link, rate);
	gjBeginCalcActor(actor);
	gjDrawLinkTree(actor->object);
	gjEndCalcActor(actor);
}

// 0x80216DA8
void gjDrawLinkTree(NJS_OBJECT* obj)
{
	do {
		njPushMatrixEx();
		njMotionLinkTransformEx(obj);
		if (gjObjectCallback != NULL) {
			gjObjectCallback(obj);
		}
		if (obj->model != NULL && (obj->evalflags & 8) == 0) {
			gjCalcActor(obj->model);
		}
		if (obj->child != NULL) {
			gjDrawLinkTree(obj->child);
		}
		njPopMatrixEx();
		obj = obj->sibling;
	} while (obj != NULL);
}

// 0x80216E34
void gjLoadPosMatrix(s32 slot)
{
	GXLoadPosMtxImm(_gj_matrix_, gjMatrixSlotTable[slot]);
}

// 0x80216E6C
void gjLoadNrmMatrix(s32 slot)
{
	GXLoadNrmMtxImm(_gj_matrix_, gjMatrixSlotTable[slot]);
}

// 0x80216EA4
void gjLoadTextureNum(s32 num, s32 wrapS, s32 wrapT, s32 map)
{
	GjTexMem* mem = _nj_current_ctx_->texlist->textures[num].texaddr;
	GjTexInfo* info = &mem->hdr->info;
	u32 flags = info->flags;
	u8 mipmap;
	if (flags & 0x80000000) {
		mipmap = 1;
	} else {
		mipmap = 0;
	}
	if (flags & 0x8000) {
		GXInitTexObjCI(&gjTexObj0, info->image, (u16)info->width, (u16)info->height, info->format, wrapS, wrapT, mipmap, mem->tlut);
	} else {
		GXInitTexObj(&gjTexObj0, info->image, (u16)info->width, (u16)info->height, info->format, wrapS, wrapT, mipmap);
	}
	GXLoadTexObj(&gjTexObj0, gjTexSlotTable[map]);
}

// 0x80216F88
void gjLoadTexture(NJS_TEXLIST* list, s32 num, s32 wrapS, s32 wrapT, s32 map)
{
	GjTexMem* mem = list->textures[num].texaddr;
	GjTexInfo* info = &mem->hdr->info;
	u32 flags = info->flags;
	u8 mipmap;
	if (flags & 0x80000000) {
		mipmap = 1;
	} else {
		mipmap = 0;
	}
	if (flags & 0x8000) {
		GXInitTexObjCI(&gjTexObj0, info->image, (u16)info->width, (u16)info->height, info->format, wrapS, wrapT, mipmap, mem->tlut);
	} else {
		GXInitTexObj(&gjTexObj0, info->image, (u16)info->width, (u16)info->height, info->format, wrapS, wrapT, mipmap);
	}
	GXLoadTexObj(&gjTexObj0, gjTexSlotTable[map]);
}

// 0x80217064
// Runs gjMtxGetTranspose on the top matrix in place by falling into it
// asm hand written
asm void gjMtxTopInPlace(void)
{
	nofralloc
	lis r4, _gj_matrix_@ha
	addi r4, r4, _gj_matrix_@l
	lwz r5, 0x0(r4)
	mr r6, r5
}

// 0x80217074
// asm hand written
asm void gjMtxGetTranspose(void)
{
	nofralloc
	psq_l f0, 0x0(r5), 0, 0
	psq_l f2, 0x10(r5), 0, 0
	psq_l f4, 0x20(r5), 0, 0
	psq_l f1, 0x8(r5), 0, 0
	ps_merge00 f6, f0, f2
	psq_l f3, 0x18(r5), 0, 0
	ps_merge01 f12, f4, f0
	psq_l f5, 0x28(r5), 0, 0
	ps_merge11 f7, f2, f4
	psq_st f6, 0x0(r6), 0, 0
	ps_merge00 f8, f1, f3
	psq_st f12, 0x8(r6), 0, 0
	ps_merge01 f9, f5, f1
	psq_st f7, 0x10(r6), 0, 0
	ps_merge11 f10, f3, f5
	psq_st f8, 0x18(r6), 0, 0
	psq_st f9, 0x20(r6), 0, 0
	psq_st f10, 0x28(r6), 0, 0
	blr
}

// 0x802170C0
// asm hand written
asm void gjMtxPush(void)
{
	nofralloc
	lis r4, _gj_matrix_@ha
	addi r4, r4, _gj_matrix_@l
	lwz r5, 0x0(r4)
	addi r6, r5, 0x30
	stw r6, 0x0(r4)
	b gjMtxGetTranspose
}

// 0x802170D8
// asm hand written
asm void gjUnitMatrix(void)
{
	nofralloc
	lis r4, _gj_matrix_@ha
	addi r4, r4, _gj_matrix_@l
	lwz r6, 0x0(r4)
}

// 0x802170E4
// asm hand written
asm void gjMtxWriteIdentity(void)
{
	nofralloc
	lfs f1, gjPrimConsts + 4
	ps_sub f0, f1, f1
	ps_merge01 f1, f1, f0
	psq_st f0, 0x8(r6), 0, 0
	stfs f0, 0x10(r6)
	stfs f0, 0x1c(r6)
	psq_st f0, 0x20(r6), 0, 0
	psq_st f1, 0x0(r6), 0, 0
	psq_st f1, 0x14(r6), 0, 0
	psq_st f1, 0x28(r6), 0, 0
	blr
}

// 0x80217110
// asm hand written
asm void gjPushUnitMatrix(void)
{
	nofralloc
	lis r4, _gj_matrix_@ha
	addi r4, r4, _gj_matrix_@l
	lwz r5, 0x0(r4)
	addi r6, r5, 0x30
	stw r6, 0x0(r4)
	b gjMtxWriteIdentity
}

// 0x80217128
void gjEnableFog(void)
{
	gjFogEnabled = 1;
}

// 0x80217134
void gjDisableFog(void)
{
	gjFogEnabled = 0;
}

// 0x80217140
void gjSetFogColor(u32 col)
{
	gjFogColor.r = (col & 0xFF000000) >> 24;
	gjFogColor.g = (col & 0xFF0000) >> 16;
	gjFogColor.b = (col & 0xFF00) >> 8;
}

// 0x80217174
void gjSetFogRange(f32 start, f32 end)
{
	gjFogStart = start;
	gjFogEnd = end;
}

// 0x80217180
void gjSetFogCurve(u32 curve)
{
	gjFogCurve = curve;
}

// 0x80217188
void gjSetFog(void)
{
	s32 type;
	if (gjFogEnabled == 0) {
		type = 0;
	} else {
		switch (gjFogCurve) {
		case 0:
			type = 0;
			break;
		case 1:
			type = 2;
			break;
		case 2:
			type = 4;
			break;
		case 3:
			type = 5;
			break;
		case 4:
			type = 6;
			break;
		case 5:
			type = 7;
			break;
		}
	}
	GXSetFog(type, gjFogStart, gjFogEnd, gProjection.n, gProjection.f, gjFogColor);
}

// 0x80217248
// A matrix that puts screen coordinates into the projection
void gjStartVertex2D(void)
{
	NJS_MATRIX m;
	f32 inv;
	f32 scale;

	PSMTXIdentity(m);
	inv = gProjection._54;
	scale = gProjection.aspect / ((f32)_gj_rmode_->fbWidth / (f32)_gj_rmode_->efbHeight);
	m[0] = -inv * scale;
	m[2] = scale * (0.5f * (f32)_gj_rmode_->fbWidth * inv);
	m[5] = inv;
	m[6] = 0.5f * (f32)(s32)-_gj_rmode_->efbHeight * inv;
	GXLoadPosMtxImm(m, 0);
	GXSetCurrentMtx(0);
}

// 0x80217328
void gjStartVertex3D(void)
{
	GXLoadPosMtxImm(_gj_matrix_, 0);
}

// 0x80217350
void gjStartVertexNonTexOpaque(void)
{
	gjPrimZOpaque();
	gjVtxFmtColor();
	gjSetPolygonCullingMode();
}

// 0x80217378
void gjStartVertexNonTexTrans(void)
{
	gjPrimZTrans();
	gjVtxFmtColor();
	gjSetPolygonCullingMode();
}

// 0x802173A0
void gjStartVertexTexOpaque(void)
{
	gjPrimZOpaque();
	gjVtxFmtColorTex();
	gjSetPolygonCullingMode();
}

// 0x802173C8
void gjStartVertexTexOpaqueEx(void)
{
	gjPrimZOpaque();
	gjVtxFmtColorSpecTex();
	gjSetPolygonCullingMode();
}

// 0x802173F0
void gjStartVertexTexTrans(void)
{
	gjPrimZTrans();
	gjVtxFmtColorTex();
	gjSetPolygonCullingMode();
}

// 0x80217418
void gjStartVertexTexTransEx(void)
{
	gjPrimZTrans();
	gjVtxFmtColorSpecTex();
	gjSetPolygonCullingMode();
}

// 0x80217440
void gjEndVertex(void)
{
}

// 0x80217444
// Loads the context's own texture with the context's clamp bits
void gjSetTexture(void)
{
	NJS_CONTEXT* ctx = _nj_current_ctx_;
	GjTexInfo* info = ctx->tex;
	u32 flags = info->flags;
	u8 mipmap;
	s32 wrapS;
	s32 wrapT;
	u32 blend;

	if (flags & 0x80000000) {
		mipmap = 1;
	} else {
		mipmap = 0;
	}
	blend = ctx->blend;
	wrapT = 1;
	wrapS = 1;
	if (blend & 0x10000) {
		wrapS = 0;
	} else if (blend & 0x40000) {
		wrapS = 2;
	}
	if (blend & 0x8000) {
		wrapT = 0;
	} else if (blend & 0x20000) {
		wrapT = 2;
	}
	if (flags & 0x8000) {
		GXInitTexObjCI(&gjTexObj1, info->image, (u16)info->width, (u16)info->height, info->format, wrapS, wrapT, mipmap, ctx->_18);
	} else {
		GXInitTexObj(&gjTexObj1, info->image, (u16)info->width, (u16)info->height, info->format, wrapS, wrapT, mipmap);
	}
	GXLoadTexObj(&gjTexObj1, 0);
}

// 0x80217548
// Position and colour
void gjVtxFmtColor(void)
{
	GXClearVtxDesc();
	GXSetVtxDesc(9, 1);
	GXSetVtxDesc(11, 1);
	GXSetVtxAttrFmt(4, 9, 1, 4, 0);
	GXSetVtxAttrFmt(4, 11, 1, 5, 0);
	gjSetNumChannels(1);
	gjPrimTevPass();
	gjPrimNoTexGen();
}

// 0x802175C0
// Position colour and one texture coordinate
void gjVtxFmtColorTex(void)
{
	GXClearVtxDesc();
	GXSetVtxDesc(9, 1);
	GXSetVtxDesc(11, 1);
	GXSetVtxDesc(13, 1);
	GXSetVtxAttrFmt(4, 9, 1, 4, 0);
	GXSetVtxAttrFmt(4, 11, 1, 5, 0);
	GXSetVtxAttrFmt(4, 13, 1, 4, 0);
	gjSetNumChannels(1);
	gjPrimTevStages(1);
	gjPrimOneTexGen();
}

// 0x80217660
// As above with a second colour which is the quad's specular
void gjVtxFmtColorSpecTex(void)
{
	GXClearVtxDesc();
	GXSetVtxDesc(9, 1);
	GXSetVtxDesc(11, 1);
	GXSetVtxDesc(12, 1);
	GXSetVtxDesc(13, 1);
	GXSetVtxAttrFmt(4, 9, 1, 4, 0);
	GXSetVtxAttrFmt(4, 11, 1, 5, 0);
	GXSetVtxAttrFmt(4, 12, 1, 5, 0);
	GXSetVtxAttrFmt(4, 13, 1, 4, 0);
	gjSetNumChannels(2);
	gjPrimTevStages(2);
	gjPrimOneTexGen();
}

// 0x80217724
void gjSetNumChannels(s32 n)
{
	GXSetNumChans((u8)n);
	GXSetChanCtrl(4, 0, 0, 1, 0, 0, 2);
	if (n == 2) {
		GXSetChanCtrl(5, 0, 0, 1, 0, 0, 2);
	}
}

// 0x8021779C
void gjPrimZOpaque(void)
{
	GXSetZCompLoc(1);
	gjSetZBufferMode(1);
	gjGXSetBlendMode(0);
}

// 0x802177D0
void gjPrimZTrans(void)
{
	GXSetZCompLoc(0);
	if ((u32)(_nj_control_3d_flag_ & 0x4000) != 0) {
		GXSetAlphaCompare(4, (u8)gDrawAlphaRef, 0, 7, 0);
		gjSetZBufferMode(1);
		GXSetZMode(1, 1, 1);
	} else {
		GXSetAlphaCompare(4, 0, 0, 7, 0);
		gjSetZBufferMode(0);
	}
	gjGXSetBlendMode(1);
}

// 0x80217864
void gjPrimTevPass(void)
{
	GXSetNumTevStages(1);
	GXSetTevOrder(0, 0xFF, 0xFF, 4);
	GXSetTevOp(0, 4);
}

// 0x802178A8
// With two stages the second adds the specular
void gjPrimTevStages(s32 n)
{
	GXSetNumTevStages((u8)n);
	switch (n) {
	case 1:
		GXSetTevOrder(0, 0, 0, 4);
		GXSetTevOp(0, 0);
		break;
	case 2:
		GXSetTevOrder(0, 0, 0, 4);
		GXSetTevOrder(1, 0xFF, 0xFF, 5);
		GXSetTevOp(0, 0);
		GXSetTevColorIn(1, 15, 10, 12, 0);
		GXSetTevColorOp(1, 0, 0, 0, 1, 0);
		GXSetTevAlphaIn(1, 7, 7, 7, 0);
		GXSetTevAlphaOp(1, 0, 0, 0, 1, 0);
		break;
	}
}

// 0x802179B0
void gjPrimNoTexGen(void)
{
	GXSetNumTexGens(0);
}

// 0x802179D4
void gjPrimOneTexGen(void)
{
	GXSetNumTexGens(1);
	gjPrimTexCoordGen(0, 1, 4, 0x3C);
}

// 0x80217A0C
void gjPrimTexCoordGen(s32 dst, s32 func, s32 src, s32 mtx)
{
	GXSetTexCoordGen2(dst, func, src, mtx, 0, 0x7D);
}
