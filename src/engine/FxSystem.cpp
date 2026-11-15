#include "stage/Stage.h"
#include "engine/Fx.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "engine/TFogCtrl.h"
#include "ui/AeArc.h"
#include "engine/TPolygonAnim.h"
#include "engine/FxSystem.h"
#include "engine/FxSetup.h"
#include "core/Mem.h"
#include "animal/AniMount.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "engine/FxEmitter.h"
#include "core/TMainTask.h"
#include "engine/FxShape.h"
struct _GXTexObj;
struct NJS_TEXLIST;

extern "C" {
f32 gFxCoordArray[25][2] = {
	{ 0.0f, 0.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 1.0f },
	{ 0.5f, 0.0f },
	{ 0.0f, 0.5f },
	{ 0.5f, 0.5f },
	{ 1.0f, 0.5f },
	{ 0.5f, 1.0f },
	{ 0.25f, 0.0f },
	{ 0.75f, 0.0f },
	{ 0.0f, 0.25f },
	{ 0.25f, 0.25f },
	{ 0.5f, 0.25f },
	{ 0.75f, 0.25f },
	{ 1.0f, 0.25f },
	{ 0.25f, 0.5f },
	{ 0.75f, 0.5f },
	{ 0.0f, 0.75f },
	{ 0.25f, 0.75f },
	{ 0.5f, 0.75f },
	{ 0.75f, 0.75f },
	{ 1.0f, 0.75f },
	{ 0.25f, 1.0f },
	{ 0.75f, 1.0f },
};

}

// 0x800DA460
void fxAddLayer0(void* fx)
{
	gFxPolygonAnim->addAt0(fx);
}

// 0x800DA488
void fxAddLayer1(void* fx)
{
	gFxPolygonAnim->addAt1(fx);
}

// 0x800DA4B0
void fxAddLayer2(void* fx)
{
	gFxPolygonAnim->addAt2(fx);
}

// 0x800DA4D8
void fxFogToggle(int on)
{
	if (on == 0) {
		fogOff();
	} else {
		fogApply();
	}
}

// 0x800DA508
// Sets up the pools which hold 0x40 0x80 0x1000 and 0x400 entries
void fxSystemInit()
{
	fxSystemReset();
	fxSetRigAlloc((void* (*)(u32))memAlloc, (void (*)(void*))memFree);
	fxSetStepTable((void (**)(FxParticle*))sFxDrawFuncs);
	sFxShapeMem = memAlloc(0x5F00);
	fxShapePoolLink(sFxShapeMem, 0x40);
	sFxRigMem = memAlloc(0x6400);
	fxRigPoolLink(sFxRigMem, 0x80);
	sFxParticleMem = memAlloc(0x70000);
	fxParticlePoolLink(sFxParticleMem, 0x1000);
	sFxExtraMem = memAlloc(0xB000);
	fxExtraPoolLink(sFxExtraMem, 0x400);
	fxSetFlag40Callback((void*)fxFogToggle);
}

// 0x800DA5A8
void fxSystemShutdown()
{
	memFree(sFxShapeMem);
	memFree(sFxRigMem);
	memFree(sFxParticleMem);
	memFree(sFxExtraMem);
	fxSetupStub();
}

// 0x800DA5E8
void createTPolygonAnim()
{
	gFxPolygonAnim = new TPolygonAnim(gTL_01);
}

// 0x800DA628
int fxBankBind(int bank, u8* data)
{
	fxBankSet(bank & 0xFF, data);
	void** textures = *(void***)(data + 0x2C);
	bindGvmTextures((u8*)textures[1], (NJS_TEXLIST*)textures[0]);
	return 1;
}

// 0x800DA66C
int fxBankUnbind(int bank, u8* data)
{
	releaseTextures((NJS_TEXLIST*)(**(void***)(data + 0x2C)));
	fxBankSet(bank & 0xFF, NULL);
	return 1;
}

// 0x800DA6B0
void fxSystemReset()
{
	int i;
	sFxShapeLimit = 0x40;
	for (i = 0; i < 0x40; i++) {
		gFxBanks[i] = NULL;
	}
	gFxGroupCount = 0x10;
	for (i = 0; i < 0x10; i++) {
		gFxGroups[i].mHead  = NULL;
		gFxGroups[i].mCount = 0;
	}
	gFxRigFreeList = NULL;
	gFxShapeFreeList = NULL;
	sFxShapePool = NULL;
	sFxShapePoolCount = 0;
	sFxRigPool = NULL;
	sFxRigPoolCount = 0;
	gFxParticleFreeList = NULL;
	sFxParticlePool = NULL;
	sFxParticlePoolCount = 0;
	gFxExtraFreeList = NULL;
	sFxExtraPool = NULL;
	sFxExtraPoolCount = 0;
	gFxStepTable = NULL;
	sFxTexImage = NULL;
	gFxParticleExtraCount = 0;
	gFxParticleCount = 0;
}

// 0x800DA758
void fxSetupStub()
{
}

// 0x800DA75C
void fxShapePoolLink(void* pool, int count)
{
	int i;
	FxShape* s = (FxShape*)pool;
	for (i = 0; i < count - 1; i++) {
		s[i].mNext = &s[i + 1];
	}
	s[count - 1].mNext = NULL;
	sFxShapePool = pool;
	sFxShapePoolCount = count;
	gFxShapeFreeList = (FxShape*)pool;
}

// 0x800DA7B0
void fxRigPoolLink(void* pool, int count)
{
	int i;
	FxEmitter* s = (FxEmitter*)pool;
	for (i = 0; i < count - 1; i++) {
		s[i].mNext = &s[i + 1];
	}
	s[count - 1].mNext = NULL;
	sFxRigPool = pool;
	sFxRigPoolCount = count;
	gFxRigFreeList = (FxEmitter*)pool;
}

// 0x800DA804
void fxParticlePoolLink(void* pool, int count)
{
	int i;
	FxParticle* s = (FxParticle*)pool;
	for (i = 0; i < count - 1; i++) {
		s[i].mNext = &s[i + 1];
	}
	s[count - 1].mNext = NULL;
	sFxParticlePool = pool;
	sFxParticlePoolCount = count;
	gFxParticleFreeList = (FxParticle*)pool;
}

// 0x800DA854
void fxExtraPoolLink(void* pool, int count)
{
	int i;
	FxExtra* s = (FxExtra*)pool;
	for (i = 0; i < count - 1; i++) {
		s[i].mNext = &s[i + 1];
	}
	s[count - 1].mNext = NULL;
	sFxExtraPool = pool;
	sFxExtraPoolCount = count;
	gFxExtraFreeList = (FxExtra*)pool;
}

// 0x800DA8A4
void fxBankSet(u32 bank, void* data)
{
	gFxBanks[bank] = (FxBank*)data;
}

// 0x800DA8B8
Fx* fxSpawn(int groupNo, int bankNo, int id)
{
	FxBank* bank = gFxBanks[bankNo];
	if (bank == NULL) {
		return NULL;
	}
	int index = id - bank->mFirst;
	if (index < 0 || index >= bank->mCount) {
		return NULL;
	}
	u32* desc = &bank->mDescs[index * 2];
	if (desc[1] == 0) {
		return NULL;
	}
	u32 kind = desc[0];
	Fx* fx = gFxFactories[kind](bank, index);
	if (fx != NULL) {
		FxGroup* group = &gFxGroups[groupNo];
		if (group->mHead != NULL) {
			fx->mPrev = group->mHead->mPrev;
			fx->mNext = NULL;
			group->mHead->mPrev->mNext = fx;
			group->mHead->mPrev = fx;
		} else {
			fx->mPrev = fx;
			group->mHead = fx;
			fx->mNext = NULL;
		}
		group->mCount++;
		if (fx->mFlags & 2) {
			fx->mStop = 0;
		}
		fx->mIndex  = index;
		fx->mBank   = bankNo;
		fx->mOwner  = NULL;
		fx->mGroup  = group;
		fx->mFollow = NULL;
		fx->mEvent     = 0;
		if (kind == 0) {
			fx->mUpdate(fx);
		}
	}
	return fx;
}

// 0x800DAA10
Fx* fxSpawnFollow(int groupNo, int bankNo, int id, f32* matrix)
{
	Fx* fx = fxSpawn(groupNo, bankNo, id);
	if (fx != NULL) {
		fx->mFollow = matrix;
	}
	return fx;
}

// 0x800DAA48
Fx* fxSpawnAt(int groupNo, int bankNo, int id, const f32* matrix)
{
	Fx* fx = fxSpawn(groupNo, bankNo, id);
	if (fx != NULL) {
		PSMTXCopy(matrix, fx->mMatrix);
	}
	return fx;
}

// 0x800DAA8C
void fxResume(Fx* fx)
{
	fx->mFlags &= ~0x4000000;
}

// 0x800DAA9C
void fxPause(Fx* fx)
{
	fx->mFlags |= 0x4000000;
}

// 0x800DAAAC
// Lets the effect finish at the place it's at now
void fxStop(Fx* fx)
{
	if (fx != NULL) {
		fx->mStop = 1;
		if (fx->mFollow != NULL) {
			PSMTXCopy(fx->mFollow, fx->mMatrix);
			fx->mFollow = NULL;
		}
	}
}

// 0x800DAAFC
void fxRemove(Fx* fx)
{
	if (fx != NULL) {
		FxGroup* group = fx->mGroup;
		if (fx->mPrev == fx) {
			group->mHead = NULL;
		} else if (group->mHead == fx) {
			group->mHead = fx->mNext;
			fx->mPrev->mNext = NULL;
			if (fx->mNext != NULL) {
				fx->mNext->mPrev = fx->mPrev;
			}
		} else {
			fx->mPrev->mNext = fx->mNext;
			if (fx->mNext != NULL) {
				fx->mNext->mPrev = fx->mPrev;
			} else {
				group->mHead->mPrev = fx->mPrev;
			}
		}
		fx->mGroup->mCount--;
		fx->mRemove(fx);
	}
}

// 0x800DABBC
// Removes the effects from one bank
void fxRemoveBank(int bank)
{
	FxGroup* groups = gFxGroups;
	gFxDebugFlags = 2;
	for (int i = 0; i < gFxGroupCount; i++) {
		Fx* fx = groups[i].mHead;
		while (fx != NULL) {
			Fx* next = fx->mNext;
			if (fx->mBank == bank) {
				fxRemove(fx);
			}
			fx = next;
		}
	}
	gFxDebugFlags = 0;
}

// 0x800DAC44
// Flags an owner's effects for removal
void fxFlagOwner(void* owner)
{
	for (int i = 0; i < gFxGroupCount; i++) {
		Fx* fx;
		Fx* next;
		for (fx = gFxGroups[i].mHead; fx != NULL; fx = next) {
			next = fx->mNext;
			if (fx->mOwner == owner) {
				fx->mFlags |= 0x40000000;
			}
		}
	}
}

// 0x800DACA0
// Removes an owner's effects now
void fxRemoveOwner(void* owner)
{
	FxGroup* groups = gFxGroups;
	gFxDebugFlags = 1;
	for (int i = 0; i < gFxGroupCount; i++) {
		Fx* fx = groups[i].mHead;
		while (fx != NULL) {
			Fx* next = fx->mNext;
			if (fx->mOwner == owner) {
				fxRemove(fx);
			}
			fx = next;
		}
	}
	gFxDebugFlags = 0;
}

// 0x800DAD28
// Removes everything
void fxRemoveAll()
{
	FxGroup* group;
	int i;
	FxGroup* groups = gFxGroups;
	gFxDebugFlags = 2;
	for (i = 0; i < gFxGroupCount; i++) {
		group = &groups[i];
		Fx* fx = group->mHead;
		while (fx != NULL) {
			Fx* next = fx->mNext;
			fx->mRemove(fx);
			fx = next;
		}
		group->mHead  = NULL;
		group->mCount = 0;
	}
	gFxDebugFlags = 0;
}

// 0x800DADBC
void fxSetRigAlloc(void* (*alloc)(u32), void (*free)(void*))
{
	gFxRigAlloc = alloc;
	gFxRigRelease = free;
}

// 0x800DADC8
void fxSetStepTable(void (**table)(FxParticle*))
{
	gFxStepTable = table;
}

// 0x800DADD0
void fxSetFollow(Fx* fx, f32* matrix)
{
	fx->mFollow = matrix;
}

// 0x800DADD8
void fxSetOwner(Fx* fx, void* owner)
{
	fx->mOwner = owner;
}

// 0x800DADE0
// Updates one group
void fxUpdateGroup(int groupNo)
{
	Fx* fx = gFxGroups[groupNo].mHead;
	while (fx != NULL) {
		Fx* next = fx->mNext;
		if (fx->mFlags & 0x40000000) {
			fxRemove(fx);
		} else if (!(fx->mFlags & 0x4000000) && fx->mUpdate != NULL) {
			fx->mUpdate(fx);
		}
		fx = next;
	}
}

// 0x800DAE58
// Draws one group
void fxDrawGroup(int groupNo)
{
	FxGroup* group = &gFxGroups[groupNo];
	if (group->mCount != 0) {
		fxDrawBegin(group);
		// The head is read as word groupNo * 4 of the table lwzx not through group which keeps the DOL's code
		for (Fx* fx = ((Fx**)gFxGroups)[groupNo * 4]; fx != NULL; fx = fx->mNext) {
			if (!(fx->mFlags & 0xC0000000) && fx->mDraw != NULL) {
				fx->mDraw(fx);
			}
		}
		fxDrawEnd();
	}
}

// 0x800DAEE0
void fxSetView(const f32* view)
{
	PSMTXCopy(view, (f32*)gFxRenderState);
}

// 0x800DAF08
void fxSetFlag40Callback(void* fn)
{
	gFxFlag40Callback = (void (*)(int))fn;
}

// 0x800DAF10
void fxShow(Fx* fx)
{
	fx->mFlags &= 0x7FFFFFFF;
}

// 0x800DAF20
void fxHide(Fx* fx)
{
	fx->mFlags |= 0x80000000;
}

// 0x800DAF30
void fxSetCurrentMtx(int matrix)
{
	if ((u32)matrix != gFxLastMatrixSlot) {
		gFxLastMatrixSlot = matrix;
		GXSetCurrentMtx(matrix);
	}
}

// 0x800DAF60
// Sets the vertex layout if it changed
void fxSetVtxLayout(u32 mode)
{
	if (mode == gFxLastBlendMode) {
		return;
	}
	switch (mode) {
	case 0:
		GXClearVtxDesc();
		GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
		GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
		break;
	case 1:
		GXClearVtxDesc();
		GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
		GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
		break;
	case 2:
		GXClearVtxDesc();
		GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
		GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
		GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
		GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
		break;
	case 3:
		GXClearVtxDesc();
		GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
		GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
		GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
		break;
	case 4:
		GXClearVtxDesc();
		GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
		GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
		GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
		break;
	case 5:
		GXClearVtxDesc();
		GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
		GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
		GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
		break;
	case 6:
		GXClearVtxDesc();
		GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
		GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
		GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
		break;
	case 7:
		GXClearVtxDesc();
		GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
		GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
		GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
		GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
		break;
	}
}

// 0x800DB104
void fxSetCoordArray(const void* array, u8 stride)
{
	if (gFxLastArray != array) {
		gFxLastArray = (void*)array;
		GXSetArray(GX_VA_TEX0, array, stride);
	}
}

// 0x800DB140
void fxSetCull(int mode)
{
	if (mode != gFxLastMode) {
		gFxLastMode = mode;
		GXSetCullMode(mode);
	}
}

// 0x800DB170
// Mode 3 tests alpha The others differ in depth settings
void fxSetZMode(int mode, u8 ref)
{
	int m = mode & 0xFF;
	switch (m) {
	case 3:
			GXSetAlphaCompare(GX_GREATER, ref, GX_AOP_AND, GX_ALWAYS, 0);
		break;
	default:
			GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
		break;
	}
	if ((u8)mode != gFxLastZMode.mMode) {
		gFxLastZMode.mMode = mode;
		switch (m) {
		case 0:
			GXSetZCompLoc(GX_FALSE);
			GXSetZMode(GX_DISABLE, GX_LESS, GX_DISABLE);
			break;
		case 1:
			GXSetZCompLoc(GX_TRUE);
			GXSetZMode(GX_ENABLE, GX_LESS, GX_ENABLE);
			break;
		case 2:
			GXSetZCompLoc(GX_FALSE);
			GXSetZMode(GX_ENABLE, GX_LESS, GX_DISABLE);
			break;
		case 3:
			GXSetZCompLoc(GX_TRUE);
			GXSetZMode(GX_ENABLE, GX_LESS, GX_ENABLE);
			break;
		}
	}
}

// 0x800DB280
void fxSetTevColor(const u8* color)
{
	if (color[0] != gFxLastColor[0] || color[1] != gFxLastColor[1] || color[2] != gFxLastColor[2]
	    || color[3] != gFxLastColor[3]) {
		gFxLastColor[0] = color[0];
		gFxLastColor[1] = color[1];
		gFxLastColor[2] = color[2];
		gFxLastColor[3] = color[3];
		GXSetTevColor(GX_TEVREG0, *(GXColor*)color);
	}
}

// 0x800DB4AC
void fxLoadExtraTex()
{
	if (sFxTexImage != NULL) {
		u8 tex[0x20];
		GXInitTexObj((_GXTexObj*)tex, sFxTexImage, sFxTexWidth, sFxTexHeight, sFxTexFormat, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GXLoadTexObj((_GXTexObj*)tex, GX_TEXMAP7);
	}
}

// 0x800DB314
// Sets how a TEV stage combines its inputs from a table of modes

void fxSetTevMode(int stage, int mode)
{
	int cin[4];
	int ain[4];
	u8* state = gFxRenderState;
	if (gFxVertexColor[stage] == (u32)mode) {
		return;
	}
	gFxVertexColor[stage] = mode;
	if ((u32)stage == 0) {
		for (int i = 0; i < 4; i++) {
			cin[i] = sFxTevColorModes[mode][i];
			ain[i] = sFxTevAlphaModes[mode][i];
			if (cin[i] == 0xA) {
				((int*)(state + 0x270))[i] = cin[i];
				((int*)(state + 0x260))[i] = 2;
			} else {
				((int*)(state + 0x270))[i] = cin[i];
				((int*)(state + 0x260))[i] = cin[i];
			}
			if (ain[i] == 5) {
				((int*)(state + 0x290))[i] = ain[i];
				((int*)(state + 0x280))[i] = 1;
			} else {
				((int*)(state + 0x290))[i] = ain[i];
				((int*)(state + 0x280))[i] = ain[i];
			}
		}
		gFxRenderReady = -1;
	} else {
		for (int i = 0; i < 4; i++) {
			cin[i] = sFxTevColorModes[mode][i] == 0xA ? 0 : sFxTevColorModes[mode][i];
			ain[i] = sFxTevAlphaModes[mode][i] == 5 ? 0 : sFxTevAlphaModes[mode][i];
		}
		GXSetTevColorIn(stage, cin[0], cin[1], cin[2], GX_CC_ZERO);
		GXSetTevAlphaIn(stage, ain[0], ain[1], ain[2], GX_CA_ZERO);
	}
}
