#include "engine/Landscape.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "Dolphin/OS/OSCache.h"
#include "core/SdkCalls.h"
#include "engine/LandModel.h"
struct Vec3f;

/** The cull state for a mesh */
struct LandCull {
	int mTwoSided; // _00 both faces are drawn
	int _04;       // _04
};

static u8 sTevStages;        // 0x802C56D0 only reset
static u8 sNumTexGens;       // 0x802C56D1
static int sVtxSet;          // 0x802C56D4
static u32 sMatFlags;        // 0x802C56D8
static u32 sVtxDesc;         // 0x802C56DC
static u8 sZCompLoc;         // 0x802C56E0
static u8 sZEnable;          // 0x802C56E1
static int sZFunc;           // 0x802C56E4
static u8 sZUpdate;          // 0x802C56E8
static LandCull sCullDefault; // 0x802C56EC
static LandCull sCull;        // 0x802C56F4
static LandDrawHook sDrawHook; // 0x802C56FC

static LandDrawState sDrawState; // 0x80282908

// 0x8013F068
// out = from + to - from * t per channel in 16.16 fixed point
static void lerpLandColors(u32* to, u32* from, u32* out, int count, f32 t)
{
	int k   = 65536.0f * t;
	u32* dst = out;
	for (int i = 0; i < count; i++) {
		u32 b  = *from++;
		u32 a  = *to++;
		u32 br = b >> 24;
		u32 bg = b >> 16 & 0xFF;
		u32 ar = a >> 24;
		u32 ag = a >> 16 & 0xFF;
		u32 bb = b >> 8 & 0xFF;
		u32 ab = a >> 8 & 0xFF;
		u32 ba = b & 0xFF;
		u32 aa = a & 0xFF;
		*dst++ = (((br << 16) + k * (ar - br)) >> 16 << 24) | (((bg << 16) + k * (ag - bg)) >> 16 << 16)
		       | (((bb << 16) + k * (ab - bb)) >> 16 << 8) | (((ba << 16) + k * (aa - ba)) >> 16);
	}
	DCFlushRange(out, count * 4);
}

// 0x8013F154
// The size of the copy that initLandColorBuffer makes
u32 getLandColorBufferSize(LandModel* model, LandMotion* motion, int unused)
{
	int count = motion->mVtxSetCount;
	u32 size  = count * sizeof(LandVtxEntry) + sizeof(LandModel);
	for (int i = 0; i < count; i++) {
		LandVtxSet* set = motion->mVtxSets[i].mSet;
		size += sizeof(LandVtxSet);
		if (set->mClr != 0) {
			size += set->mClrCount * 4;
		}
		size = (size + 3) & ~3;
	}
	return size;
}

// 0x8013F1AC
// Copies the model with its own vertex sets and colour arrays
void initLandColorBuffer(LandModel* buffer, LandModel* model, LandMotion* motion, int unused)
{
	u8* p = (u8*)(buffer + 1);

	buffer->mMaterialCount = model->mMaterialCount;
	buffer->mMaterials     = model->mMaterials;
	buffer->mVtxSetCount   = model->mVtxSetCount;
	buffer->mVtxSets       = (LandVtxEntry*)p;
	p += motion->mVtxSetCount * sizeof(LandVtxEntry);
	buffer->mDisplayCount = model->mDisplayCount;
	buffer->mDisplays     = model->mDisplays;
	buffer->mNodeCount    = model->mNodeCount;
	buffer->mNodes        = model->mNodes;
	buffer->mMatrixCount  = model->mMatrixCount;
	buffer->mGroupCount   = model->mGroupCount;
	buffer->mGroups       = model->mGroups;

	for (int i = 0; i < motion->mVtxSetCount; i++) {
		LandVtxEntry* src = &model->mVtxSets[i];
		LandVtxSet* set   = (LandVtxSet*)p;
		LandVtxSet* from  = src->mSet;

		buffer->mVtxSets[i]._00  = src->_00;
		buffer->mVtxSets[i].mSet = set;
		p += sizeof(LandVtxSet);
		syMemCopy(set, from, sizeof(LandVtxSet));
		set->mClr = (u32*)p;
		p += set->mClrCount * 4;
		p = (u8*)(((u32)p + 3) & ~3);
	}
}

// 0x8013F2B0
// t = 1 gives the model's colours 0 the motion's
void blendLandColors(LandModel* buffer, LandModel* model, LandMotion* motion, int unused, f32 t)
{
	for (int i = 0; i < motion->mVtxSetCount; i++) {
		LandVtxSet* set = model->mVtxSets[i].mSet;
		LandVtxSet* out = buffer->mVtxSets[i].mSet;
		lerpLandColors(set->mClr, motion->mVtxSets[i].mSet->mClr, out->mClr, set->mClrCount, t);
		DCFlushRange(out->mClr, out->mClrCount * 4);
	}
}

// 0x8013F350
static void setNumTexGens(u8 count)
{
	if (count != sNumTexGens) {
		GXSetNumTexGens(count);
		sNumTexGens = count;
	}
}

// 0x8013F390
// GXSetZCompLoc and GXSetZMode take GXBool u8 in the SDK but Ninja/ninja.h declares s32 which adds masks here
static void setZCompLoc(int before)
{
	if (sZCompLoc != (u8)before) {
		GXSetZCompLoc(before);
		sZCompLoc = before;
	}
}

// 0x8013F3D0
static void setZMode(int enable, int func, int update)
{
	if ((u8)enable != sZEnable || func != sZFunc || (u8)update != sZUpdate) {
		sZEnable = enable;
		sZFunc   = func;
		sZUpdate = update;
		GXSetZMode(enable, func, update);
	}
}

// 0x8013F428
static void setLandVtxArrays(LandModel* model, LandVtxEntry* entry)
{
	LandVtxSet* set = entry->mSet;

	GXClearVtxDesc();

	switch (set->mPosType) {
	case 1:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GXSetArray(GX_VA_POS, set->mPos, 0xC);
		break;
	}

	switch (set->mNrmType) {
	case 1:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 6);
		GXSetArray(GX_VA_NRM, set->mNrm, 3);
		break;
	case 2:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
		GXSetArray(GX_VA_NRM, set->mNrm, 6);
		break;
	case 3:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
		GXSetArray(GX_VA_NRM, set->mNrm, 0xC);
		break;
	}

	void* clr = set->mClr;
	switch (set->mClrType) {
	case 1:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
		GXSetArray(GX_VA_CLR0, clr, 3);
		break;
	case 2:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
		GXSetArray(GX_VA_CLR0, clr, 4);
		break;
	case 3:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0);
		GXSetArray(GX_VA_CLR0, clr, 2);
		break;
	}

	clr = set->mClr2;
	switch (set->mClr2Type) {
	case 1:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGB, GX_RGB8, 0);
		GXSetArray(GX_VA_CLR1, clr, 3);
		break;
	case 2:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0);
		GXSetArray(GX_VA_CLR1, clr, 4);
		break;
	case 3:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA4, 0);
		GXSetArray(GX_VA_CLR1, clr, 2);
		break;
	}

	void* tex = set->mTex;
	switch (set->mTexType) {
	case 1:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
		GXSetArray(GX_VA_TEX0, tex, 4);
		break;
	case 2:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		GXSetArray(GX_VA_TEX0, tex, 8);
		break;
	}

	tex = set->mTex2;
	switch (set->mTex2Type) {
	case 1:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_S16, 8);
		GXSetArray(GX_VA_TEX1, tex, 4);
		break;
	case 2:
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
		GXSetArray(GX_VA_TEX1, tex, 8);
		break;
	}
}

// The texgen source the wrap modes and the TEV mode of one texture from a material's texture flags
inline int getTexGenSrc(u32 flags)
{
	if (flags & 0x200) {
		return 5;
	} else if (flags & 0x400) {
		return 0;
	} else if (flags & 0x800) {
		return 1;
	}
	return 4;
}

inline int getWrapS(u32 flags)
{
	if (flags & 0x20000) {
		return 1;
	} else if (flags & 0x40000) {
		return 2;
	}
	return 0;
}

inline int getWrapT(u32 flags)
{
	if (flags & 0x100000) {
		return 1;
	} else if (flags & 0x200000) {
		return 2;
	}
	return 0;
}

// 0x8013F774
// Returns 1
int setLandMaterial(const LandDrawState* state)
{
	int ambSrc;
	int matSrc;
	int src;
	u32 texFlags;

	if (state->mForce == 0 && state->mMaterial == state->mLastMaterial) {
		return 1;
	}

	u32 flags;
	LandMaterialEntry* entry = state->mEntry;
	LandMaterial* mat        = entry->mMaterial;
	flags                    = mat->mFlags;
	if (sMatFlags != flags) {
		sMatFlags = flags;
		if (flags & 1) {
			matSrc = 1;
			ambSrc = 1;
		} else {
			matSrc = 0;
			ambSrc = 0;
			GXSetChanMatColor(GX_COLOR0A0, *(GXColor*)mat->mColor);
		}
		if (flags & 2) {
			GXSetChanCtrl(GX_COLOR0, GX_DISABLE, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
		} else {
			GXSetChanCtrl(GX_COLOR0, GX_ENABLE, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_SPOT);
		}
		GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, ambSrc, matSrc, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	}

	switch (entry->mTexMode) {
	case 0:
		setNumTexGens(0);
		GXSetNumTevStages(1);
		GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
		break;
	case 1:
		setNumTexGens(1);
		GXSetNumTevStages(1);
		texFlags = mat->mTexFlags;
		src      = getTexGenSrc(texFlags);
		gjLoadTextureNum(mat->mTexNum, getWrapS(texFlags), getWrapT(texFlags), 0);
		GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, src, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
		GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		GXSetTevOp(GX_TEVSTAGE0, texFlags & 0xFF);
		break;
	case 2:
		break;
	case 3:
		setNumTexGens(2);
		GXSetNumTevStages(2);
		texFlags = mat->mTexFlags;
		src      = getTexGenSrc(texFlags);
		gjLoadTextureNum(mat->mTexNum, getWrapS(texFlags), getWrapT(texFlags), 0);
		GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, src, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
		GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		GXSetTevOp(GX_TEVSTAGE0, texFlags & 0xFF);
		texFlags = mat->mTexFlags2;
		src      = getTexGenSrc(texFlags);
		gjLoadTextureNum(mat->mTexNum2, getWrapS(texFlags), getWrapT(texFlags), 1);
		GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3X4, src, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
		GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
		GXSetTevOp(GX_TEVSTAGE1, texFlags & 0xFF);
		break;
	}

	setZCompLoc(!(mat->mFlags & 0x40));
	setZMode(!(mat->mFlags & 0x10), mat->mZFunc, !(mat->mFlags & 0x20));
	if (mat->mFlags & 0x40) {
		GXSetAlphaCompare(GX_GREATER, 0xBF, GX_AOP_AND, GX_ALWAYS, 0);
	} else {
		GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	}
	GXSetBlendMode(mat->mBlend, mat->mSrc, mat->mDst, GX_LO_SET);
	return 1;
}

// 0x8013FBE0
static void drawLandDisplay(LandModel* model, int index)
{
	LandDisplayEntry* entry = &model->mDisplays[index];
	if (entry->mType != 0) {
		return;
	}

	LandDisplay* display = entry->mDisplay;
	if (sVtxDesc != display->mVtxFlags) {
		sVtxDesc = display->mVtxFlags;
		GXClearVtxDesc();
		if (display->mVtxFlags & 1) {
			GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
		}
		if (display->mVtxFlags & 2) {
			GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
		}
		if (display->mVtxFlags & 4) {
			GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
		}
		if (display->mVtxFlags & 8) {
			GXSetVtxDesc(GX_VA_CLR1, GX_INDEX16);
		}
		if (display->mVtxFlags & 0x10) {
			GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
		}
		if (display->mVtxFlags & 0x20) {
			GXSetVtxDesc(GX_VA_TEX1, GX_INDEX16);
		}
	}

	if (display->mFrontSize != 0) {
		if (sCull.mTwoSided) {
			GXSetCullMode(GX_CULL_NONE);
		} else {
			GXSetCullMode(GX_CULL_BACK);
		}
		GXCallDisplayList(display->mFront, display->mFrontSize);
	}
	if (display->mBackSize != 0) {
		if (sCull.mTwoSided) {
			GXSetCullMode(GX_CULL_NONE);
		} else {
			GXSetCullMode(GX_CULL_FRONT);
		}
		GXCallDisplayList(display->mBack, display->mBackSize);
	}
}

// 0x8013FD38
// Forgets the cached state and sets GX to the defaults
static void resetLandState()
{
	sTevStages  = 0xFF;
	sNumTexGens = 0xFF;
	sVtxDesc    = -1;
	sMatFlags   = 1;
	GXSetChanCtrl(GX_COLOR0, GX_ENABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_SPOT);
	GXSetChanCtrl(GX_ALPHA0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
	sZCompLoc = 0;
	setZCompLoc(1);
	sZEnable = 0;
	sZFunc   = 1;
	sZUpdate = 0;
	setZMode(1, 1, 1);
}

// 0x8013FDE4
// Returns 1 to skip the mesh and sets the cull state for it
static int isLandMeshHidden(u32 hidden, u32 flags)
{
	sCull = sCullDefault;
	if (hidden & 1) {
		return 1;
	}
	if (flags & 4) {
		sCull.mTwoSided = 1;
	}
	return 0;
}

// 0x8013FE24
static void drawLandGroup(LandModel* model, LandMeshGroup* group, f32* mtx, u32* hidden)
{
	if (model->mMatrixCount == 1) {
		GXLoadPosMtxImm(mtx, GX_PNMTX0);
		GXLoadNrmMtxImm(mtx, GX_PNMTX0);

		LandMesh* mesh = group->mMeshes;
		for (int i = 0; i < group->mMeshCount; i++, mesh++) {
			u32 skip;
			LandVtxEntry* vtx;
			LandMaterialEntry* entry;
			skip = hidden != 0 ? hidden[mesh->mNode] : 0;
			entry = &model->mMaterials[mesh->mMaterial];
			vtx = &model->mVtxSets[mesh->mVtxSet];
			if (isLandMeshHidden(skip, entry->mMaterial->mFlags) == 0) {
				if (sVtxSet != mesh->mVtxSet) {
					sVtxSet = mesh->mVtxSet;
					setLandVtxArrays(model, vtx);
				}
				sDrawState.mLastMaterial = sDrawState.mMaterial;
				sDrawState.mMaterial     = mesh->mMaterial;
				sDrawState.mEntry        = entry;
				sDrawState.mMesh         = i;
				if (sDrawHook != 0) {
					sDrawHook(&sDrawState);
					if (sDrawState.mForce != 0) {
						sDrawState.mForce = 0;
					}
				} else {
					setLandMaterial(&sDrawState);
				}
				drawLandDisplay(model, mesh->mDisplay);
			}
		}
	} else if (model->mMatrixCount < 10) {
		for (int j = 0; j < model->mMatrixCount; j++) {
			GXLoadPosMtxImm(mtx + j * 12, j * 3);
			GXLoadNrmMtxImm(mtx + j * 12, j * 3);
		}

		LandMesh* mesh = group->mMeshes;
		for (int i = 0; i < group->mMeshCount; i++, mesh++) {
			u32 skip = hidden != 0 ? hidden[mesh->mNode] : 0;
			LandMaterialEntry* entry = &model->mMaterials[mesh->mMaterial];
			LandVtxEntry* vtx        = &model->mVtxSets[mesh->mVtxSet];
			if (isLandMeshHidden(skip, entry->mMaterial->mFlags) == 0) {
				if (sVtxSet != mesh->mVtxSet) {
					sVtxSet = mesh->mVtxSet;
					setLandVtxArrays(model, vtx);
				}
				sDrawState.mLastMaterial = sDrawState.mMaterial;
				sDrawState.mMaterial     = mesh->mMaterial;
				sDrawState.mEntry        = entry;
				sDrawState.mMesh         = i;
				if (sDrawHook != 0) {
					sDrawHook(&sDrawState);
					if (sDrawState.mForce != 0) {
						sDrawState.mForce = 0;
					}
				} else {
					setLandMaterial(&sDrawState);
				}
				GXSetCurrentMtx(mesh->mMatrix * 3);
				drawLandDisplay(model, mesh->mDisplay);
			}
		}
		GXSetCurrentMtx(GX_PNMTX0);
	}
}

// 0x801400C0
// Draws the groups whose pass bits meet pass
void drawLandModel(LandModel* model, f32* mtx, u32* hidden, u32 pass, u32 flags)
{
	sCullDefault.mTwoSided = flags & 1;
	if (flags & 2) {
		sCullDefault._04 = 1;
	} else {
		sCullDefault._04 = 0;
	}
	resetLandState();

	sDrawState.mMaterial     = -1;
	sDrawState.mLastMaterial = -1;
	sDrawState.mModel        = model;
	sDrawState.mMtx          = mtx;
	sDrawState.mPass         = pass;
	sDrawState.mFlags        = flags;
	sDrawState.mForce        = 0;
	sVtxSet                  = -1;

	LandMeshGroup* group = model->mGroups;
	for (int i = 0; i < model->mGroupCount; i++, group++) {
		sDrawState.mGroup = i;
		if (pass & group->mPass) {
			drawLandGroup(model, group, mtx, hidden);
		}
	}
}

// 0x8014019C
// Like gjGetMotionNodeData Returns the rotation order 0 XYZ 1 ZXY 2 a quaternion
static int getLandNodeMotion(LandNode* node, f32* pos, u32* ang, f32* scl, NJS_QUATERNION* quat)
{
	int type;

	if (gjCurMotionState->getPos(pos) == 0) {
		pos[0] = node->mPosX;
		pos[1] = node->mPosY;
		pos[2] = node->mPosZ;
	}

	if (gjCurMotionState->getAng != 0) {
		if (gjCurMotionState->getAng((s32*)ang) != 0) {
			type = (node->mFlags >> 6) & 1;
		} else {
			ang[0] = node->mAngX;
			ang[1] = node->mAngY;
			ang[2] = node->mAngZ;
			type   = (node->mFlags >> 6) & 1;
		}
	} else if (gjCurMotionState->getAngS != 0) {
		s16 angS[3];
		if (gjCurMotionState->getAngS(angS) != 0) {
			ang[0] = angS[0];
			ang[1] = angS[1];
			ang[2] = angS[2];
			type   = (node->mFlags >> 6) & 1;
		} else {
			ang[0] = node->mAngX;
			ang[1] = node->mAngY;
			ang[2] = node->mAngZ;
			type   = (node->mFlags >> 6) & 1;
		}
	} else if (gjCurMotionState->getQuat(quat) != 0) {
		type = 2;
	} else {
		ang[0] = node->mAngX;
		ang[1] = node->mAngY;
		ang[2] = node->mAngZ;
		type   = (node->mFlags >> 6) & 1;
	}

	if (gjCurMotionState->getScl(scl) == 0) {
		scl[0] = node->mSclX;
		scl[1] = node->mSclY;
		scl[2] = node->mSclZ;
	}

	njSetNextMotionNodeEx();
	return type;
}

// 0x80140340
static void transformLandMotionNode(LandModel* model, LandNode* node)
{
	f32 pos[3];
	f32 scl[3];
	u32 ang[3];
	NJS_QUATERNION quat;

	int type = getLandNodeMotion(node, pos, ang, scl, &quat);
	njTranslateEx((const Vec3f*)pos);
	switch (type) {
	case 0:
		njRotateEx((const s32*)ang, (s32)0);
		break;
	case 1:
		njRotateEx((const s32*)ang, (s32)1);
		break;
	default:
		njQuaternionEx(&quat);
		break;
	}
	njScaleEx((const Vec3f*)scl);
}

// 0x801403D0
// Each node's matrix goes to out[node->mMatrix]
static void calcLandNodeMatrices(f32* out, const LandModel* model, int index)
{
	LandNode* nodes = model->mNodes;
	do {
		LandNode* node = &nodes[index];
		njPushMatrixEx();
		if (!(node->mFlags & 1)) {
			njTranslateEx((const Vec3f*)&node->mPosX);
		}
		if (!(node->mFlags & 2)) {
			njRotateEx((const s32*)(&node->mAngX), (s32)0);
		}
		if (!(node->mFlags & 4)) {
			njScaleEx((const Vec3f*)(&node->mSclX));
		}
		njGetMatrix(out + node->mMatrix * 12);
		if (!(node->mFlags & 0x20)) {
			calcLandNodeMatrices(out, model, node->mChild);
		}
		njPopMatrixEx();
		index = node->mSibling;
	} while (index != -1);
}

// 0x80140488
void calcLandMatrices(f32* out, LandModel* model)
{
	njPushMatrixEx();
	njUnitMatrix(0);
	calcLandNodeMatrices(out, model, 0);
	njPopMatrixEx();
}

// 0x801404D4
static void calcLandMotionNodeMatrices(f32* out, const LandModel* model, int index)
{
	LandNode* nodes = model->mNodes;
	do {
		LandNode* node = &nodes[index];
		njPushMatrixEx();
		transformLandMotionNode((LandModel*)model, node);
		njGetMatrix(out + node->mMatrix * 12);
		if (!(node->mFlags & 0x20)) {
			calcLandMotionNodeMatrices(out, model, node->mChild);
		}
		njPopMatrixEx();
		index = node->mSibling;
	} while (index != -1);
}

// 0x80140558
// motion is a Ninja NJS_MOTION
void calcLandMotionMatrices(f32* out, LandModel* model, void* motion, f32 frame)
{
	njStartMotionObj(motion, frame);
	njPushMatrixEx();
	njUnitMatrix(0);
	calcLandMotionNodeMatrices(out, model, 0);
	njPopMatrixEx();
}

// 0x801405AC
// out = a * b or count matrices of b when count is above 1
void mulLandMatrix(f32* out, f32* a, f32* b, u32 count)
{
	if (count > 1) {
		PSMTXConcatArray(a, b, out, count);
	} else {
		PSMTXConcat(a, b, out);
	}
}

// 0x801405F8
// The hook replaces setLandMaterial for each mesh
void setLandDrawHook(LandDrawHook hook)
{
	sDrawHook = hook;
}
