#include "engine/Landscape.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/Resource.h"
#include "object/ObjAction.h"
#include "engine/ShadowTex.h"
#include "ui/AeArc.h"
#include "engine/TFogCtrl.h"
#include "camera/TCamera.h"
#include "core/Mem.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "Ninja/ninja.h"
#include "object/TObject.h"
#include "io/ResManager.h"
#include "object/blue/TObjBlueSea.h"
#include "camera/TCameraCtrl.h"
#include "ui/TBlindCtrl.h"
#include "core/TMainTask.h"
struct NJS_TEXLIST;

// Loads and draws stg_*.lnd the visible part of a stage
// The main block is always drawn
// Not identified yet dtk's default fn_/lbl_ names
extern "C" {

TLandscape* sLandscapeObj;
TArc* sLandscapeArc;
TLandNightTest* sLandscapeCur;
f32 sLandscapeBlend;
int sLandscapeEnabled;
u8 sLandscapeAlpha;
}

inline void* TLandNightTest::operator new(unsigned long size, void* pool)
{
	return objHeapAlloc(pool, size);
}

inline void TLandNightTest::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

TLandscape::TLandscape()
{
	mFile         = 0;
	mVariants     = 0;
	mAnimStates   = 0;
	mAnimMatrices = 0;
}

TLandscape::~TLandscape()
{
	freeBlocks();
}

static void initMotionState(LandMotionState* state, LandModel* model, f32* matrices)
{
	state->mModel     = model;
	state->mBuffer    = model;
	state->mMotion    = 0;
	state->mMatrices  = matrices;
	state->mAnimIndex = -1;
}

static u8* setupMotion(LandMotionState* state, LandMotion* motion, u8* buffer)
{
	state->mBuffer = (LandModel*)buffer;
	state->mMotion = motion;

	buffer += getLandColorBufferSize(state->mModel, state->mMotion, 0);
	initLandColorBuffer(state->mBuffer, state->mModel, state->mMotion, 0);
	blendLandColors(state->mBuffer, state->mModel, state->mMotion, 0, 0.0f);
	return buffer;
}

static void initBlockState(LandBlock* block, LandBlockState* state)
{
	state->mBlock      = block;
	state->mVisible    = (u32*)memAlloc(block->mModel->mNodeCount << 2);
	state->mVisibleFar = (u32*)memAlloc(block->mModel->mNodeCount << 2);

	if (block->mPartCount != 0) {
		state->mParts        = (LandMotionState*)memAlloc(block->mPartCount * sizeof(LandMotionState));
		state->mPartMatrices = (f32*)memAlloc(block->mPartCount * 0x30);
	}

	initMotionState(&state->mRoot, block->mModel, 0);

	for (int i = 0; i < block->mPartCount; i++) {
		initMotionState(&state->mParts[i], block->mParts[i].mModel, state->mPartMatrices + i * 12);
		calcLandMatrices(state->mParts[i].mMatrices, state->mParts[i].mModel);

		// The part's first node is reset since its transform comes from the animation
		LandNode* node = state->mParts[i].mModel->mNodes;
		node->mPosZ    = 0.0f;
		node->mPosY    = 0.0f;
		node->mPosX    = 0.0f;
		node->mAngZ    = 0;
		node->mAngY    = 0;
		node->mAngX    = 0;
		node->mSclZ    = 1.0f;
		node->mSclY    = 1.0f;
		node->mSclX    = 1.0f;
	}

	u32 bufferSize = 0;
	int i;
	if (block->mMotion != 0) {
		bufferSize = getLandColorBufferSize(block->mModel, block->mMotion, 0);
	}
	for (i = 0; i < block->mPartCount; i++) {
		if (block->mParts[i].mMotion != 0) {
			bufferSize += getLandColorBufferSize(block->mParts[i].mModel, block->mParts[i].mMotion, 0);
		}
	}

	if (bufferSize != 0) {
		state->mMotionBuffers = memAlloc(bufferSize);
		bufferSize            = (u32)state->mMotionBuffers;
		if (block->mMotion != 0) {
			bufferSize = (u32)setupMotion(&state->mRoot, block->mMotion, (u8*)bufferSize);
		}
		u8* buffer = (u8*)bufferSize;
		for (i = 0; i < block->mPartCount; i++) {
			if (block->mParts[i].mMotion != 0) {
				buffer = setupMotion(&state->mParts[i], block->mParts[i].mMotion, buffer);
			}
		}
	}
}

static void linkAnims(LandBlockState* state, LandAnimSet* anims, LandAnimState* animStates)
{
	LandAnimNode* node = anims->mNodes;
	for (u32 i = 0; i < anims->mNodeCount; i++, node++) {
		if (node->mAnim != -1) {
			for (int j = 0; j < state->mBlock->mPartCount; j++) {
				if (node->mPartId == state->mBlock->mParts[j].mId) {
					state->mParts[j].mAnimIndex = i;
					if (animStates[i].mModel == 0) {
						animStates[i].mModel = state->mParts[j].mModel;
					}
				}
			}
		}
	}
}

void TLandscape::init(LandFile* file)
{
	mFile = file;
	syMemSet(&mMain, 0, sizeof(LandBlockState));

	if (file->mMain != 0) {
		initBlockState(file->mMain, &mMain);
		mMain.mTexList = file->mTexList;
	}

	if (file->mVariantCount != 0) {
		mVariants = (LandBlockState*)memAlloc(file->mVariantCount * sizeof(LandBlockState));
		syMemSet(mVariants, 0, file->mVariantCount * sizeof(LandBlockState));
		for (int i = 0; i < file->mVariantCount; i++) {
			initBlockState(file->mVariants[i], &mVariants[i]);
			mVariants[i].mTexList = file->mTexList;
		}
	}

	LandAnimSet* anims = file->mAnims;
	if (anims != 0) {
		mAnimMatrices = (f32*)memAlloc(anims->mNodeCount * 0x30);
		mAnimStates   = (LandAnimState*)memAlloc(anims->mNodeCount * sizeof(LandAnimState));

		LandAnimNode* node = anims->mNodes;
		for (u32 i = 0; i < anims->mNodeCount; i++, node++) {
			if (node->mAnim == -1) {
				mAnimStates[i].mFrame    = 0.0f;
				mAnimStates[i].mSpeed    = 0.0f;
				mAnimStates[i].mModel    = 0;
				mAnimStates[i].mHeader   = 0;
				mAnimStates[i].mMatrices = 0;
			} else {
				LandAction* action       = anims->mAnims[node->mAnim].mAction;
				mAnimStates[i].mFrame    = 0.0f;
				mAnimStates[i].mSpeed    = 0.1f;
				mAnimStates[i].mModel    = 0;
				mAnimStates[i].mHeader   = action->mMotion;
				mAnimStates[i].mMatrices = mAnimMatrices + i * 12;
			}
		}

		if (file->mMain != 0) {
			linkAnims(&mMain, anims, mAnimStates);
		}
		if (file->mVariantCount != 0) {
			for (int i = 0; i < file->mVariantCount; i++) {
				linkAnims(&mVariants[i], anims, mAnimStates);
			}
		}
	}

	if (file->mGvm != 0) {
		numberTextures(file->mTexList);
		bindGvmTextures((u8*)file->mGvm, file->mTexList);
	}
}

LandBlockState* TLandscape::getVariant(int index)
{
	if (index < 0 || index >= mFile->mVariantCount) {
		return 0;
	}
	return &mVariants[index];
}

static void freeIfSet(void* mem)
{
	if (mem != 0) {
		memFree(mem);
	}
}

static void freeBlockState(LandBlockState* state)
{
	freeIfSet(state->mMotionBuffers);
	freeIfSet(state->mParts);
	freeIfSet(state->mPartMatrices);
	freeIfSet(state->mVisibleFar);
	freeIfSet(state->mVisible);
}

void TLandscape::freeBlocks()
{
	if (mFile != 0) {
		if (mFile->mTexList != 0) {
			releaseTextures(mFile->mTexList);
		}
		freeBlockState(&mMain);
		if (mVariants != 0) {
			for (int i = 0; i < mFile->mVariantCount; i++) {
				freeBlockState(&mVariants[i]);
			}
			memFree(mVariants);
		}
		if (mAnimStates != 0) {
			memFree(mAnimStates);
		}
		if (mAnimMatrices != 0) {
			memFree(mAnimMatrices);
		}
	}
}

void TLandscape::updateAnims()
{
	if (mFile->mAnims != 0) {
		for (u32 i = 0; i < mFile->mAnims->mNodeCount; i++) {
			if (mAnimStates[i].mModel != 0) {
				calcLandMotionMatrices(mAnimStates[i].mMatrices, mAnimStates[i].mModel, mAnimStates[i].mHeader, mAnimStates[i].mFrame);
				mAnimStates[i].mFrame += mAnimStates[i].mSpeed;
				if (mAnimStates[i].mFrame >= (f32)mAnimStates[i].mHeader[1]) {
					mAnimStates[i].mFrame -= (f32)mAnimStates[i].mHeader[1];
				}
			}
		}
	}
}

void initLandscape()
{
	sLandscapeArc     = 0;
	sLandscapeObj     = 0;
	sLandscapeCur     = 0;
	sLandscapeBlend   = 0.0f;
	sLandscapeEnabled = 1;
}

void loadLandscape(const char* name)
{
	loadArc(name, &sLandscapeArc);
	if (sLandscapeArc != 0) {
		sLandscapeObj = new TLandscape;
		sLandscapeObj->init((LandFile*)sLandscapeArc->mData);
	}
}

void freeLandscape()
{
	if (sLandscapeArc != 0) {
		delete sLandscapeObj;
		releaseResource(sLandscapeArc);
	}
	sLandscapeArc = 0;
	sLandscapeCur = 0;
}

void createLandscapeObject()
{
	sLandscapeCur = new (gObjHeap) TLandNightTest(gTL_01);
}

void setLandscapeBlend(f32 blend, int frames)
{
	if (sLandscapeCur != 0) {
		sLandscapeCur->setBlend(blend, frames);
	}
	sLandscapeBlend = blend;
}

f32 getLandscapeBlend()
{
	if (sLandscapeCur != 0) {
		return sLandscapeCur->mBlend;
	}
	return 0.0f;
}

extern "C" int setLandscapeMode(char* mode)
{
	switch (*mode) {
	case 0:
		setLandscapeBlend(0.0f, 0);
		sLandscapeBlend = 0.0f;
		break;
	case 1:
		sLandscapeBlend = 1.0f;
		setLandscapeBlend(1.0f, 0);
		break;
	}
	return 1;
}

static void drawOpaque();
static void drawAlpha();

TLandNightTest::TLandNightTest(TObject* parent)
    : TObject(parent)
{
	_00         = (u32)gTLandNightTestName;
	mSize       = sizeof(TLandNightTest);
	mDraw       = addDrawTask(this, (void (*)(void*))drawOpaque, 0x21, 0);
	mDrawAlpha  = addDrawTask(this, (void (*)(void*))drawAlpha, 0x22, 0);
	mBlend      = sLandscapeBlend;
	mBlendTimer = 0;
	applyBlend(mBlend);
}

TLandNightTest::~TLandNightTest()
{
	removeDrawTask(mDraw);
	removeDrawTask((DrawTask*)mDrawAlpha);
}

extern "C" void setLandscapeFlag(int enabled)
{
	sLandscapeEnabled = enabled;
}

void TLandNightTest::setBlend(f32 blend, int frames)
{
	mBlendTarget = blend;
	mBlendTimer  = frames;
	if (frames == 0 && mBlend != mBlendTarget) {
		mBlend = mBlendTarget;
		applyBlend(mBlend);
	}
}

static void setBlockFrame(f32 frame, LandBlockState* state)
{
	if (state->mRoot.mMotion != 0) {
		blendLandColors(state->mRoot.mBuffer, state->mRoot.mModel, state->mRoot.mMotion, 0, frame);
	}
	for (int i = 0; i < state->mBlock->mPartCount; i++) {
		LandMotionState* part = &state->mParts[i];
		if (part->mMotion != 0) {
			blendLandColors(part->mBuffer, part->mModel, part->mMotion, 0, frame);
		}
	}
}

void TLandNightTest::applyBlend(f32 blend)
{
	if (sLandscapeObj != 0) {
		setBlockFrame(1.0f - blend, &sLandscapeObj->mMain);
	}
	setBlueSeaColor(blend);
}

void TLandNightTest::draw()
{
	f32 old = mBlend;
	if (mBlendTimer != 0) {
		mBlend += (mBlendTarget - old) / (f32)mBlendTimer;
		mBlendTimer--;
	}
	if (old != mBlend) {
		applyBlend(mBlend);
	}
	if (sLandscapeObj != 0) {
		sLandscapeObj->updateAnims();
	}
}

static int fadeHook(LandDrawState* state)
{
	int result = setLandMaterial(state);
	if (state->mMaterial != state->mLastMaterial) {
		int twoSided;
		if (state->mEntry->mTexMode == 0) {
			GXSetNumTexGens(1);
			twoSided = 0;
		} else {
			GXSetNumTexGens(2);
			twoSided = 1;
		}
		GXSetTexCoordGen2(twoSided, GX_TG_MTX3X4, GX_TG_POS, GX_TEXMTX5, GX_FALSE, GX_PTIDENTITY);
		GXSetNumTevStages(2);
		GXSetTevDirect(GX_TEVSTAGE1);
		GXSetTevOrder(GX_TEVSTAGE1, twoSided, GX_TEXMAP5, GX_COLOR_NULL);
		GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ZERO);
		GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
		GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	}
	return result;
}

static void blendHook(LandDrawState* state)
{
	u8 color0;
	u8 color1;
	u8 color2;
	u8 color3;
	LandMaterial* mat;
	u32 flags;
	int blend;
	int src;
	int dst;

	switch (state->mEntry->mTexMode) {
	case 1:
		mat = state->mEntry->mMaterial;
		break;
	default:
		setLandMaterial(state);
		return;
	}

	flags  = mat->mFlags;
	color0 = mat->mColor[0];
	color1 = mat->mColor[1];
	color2 = mat->mColor[2];
	color3 = mat->mColor[3];
	blend  = mat->mBlend;
	src    = mat->mSrc;
	dst    = mat->mDst;

	mat->mFlags |= 0x20;
	mat->mFlags &= ~1;
	mat->mColor[3] = sLandscapeAlpha;
	mat->mBlend    = 1;
	mat->mSrc      = 4;
	mat->mDst      = 5;
	state->mForce  = 1;

	setLandMaterial(state);

	mat->mFlags    = flags;
	mat->mColor[0] = color0;
	mat->mColor[1] = color1;
	mat->mColor[2] = color2;
	mat->mColor[3] = color3;
	mat->mBlend    = blend;
	mat->mSrc      = src;
	mat->mDst      = dst;
}

void TLandNightTest::vfn_10()
{
	TLandscape* land = sLandscapeObj;
	if (land != 0) {
		LandModel* model = land->mMain.mRoot.mModel;
		syMemSet4(land->mMain.mVisible, 0, model->mNodeCount);
		if (sLandscapeEnabled != 0) {
			syMemSet4(land->mMain.mVisibleFar, 0, model->mNodeCount);
			TObjAction* camera = findObjById(0);
			if (camera != 0) {
				Vec3f* pos = camera->getPos();
				Vec3f eye;
				eye.x = pos->x;
				eye.y = pos->y;
				eye.z = pos->z;
				u8 diff[0xC];
				for (int i = 0; i < (int)model->mNodeCount; i++) {
					gjSubVector((Vec3f*)diff, (const Vec3f*)&model->mNodes[i].mCenter, (const Vec3f*)&eye);
					if (njScalor((Vec3f*)diff) < 10.0f + model->mNodes[i].mRadius) {
						land->mMain.mVisible[i] = 1;
					} else {
						land->mMain.mVisibleFar[i] = 1;
					}
				}
			}
		}
	}
}

static void drawBlockOpaque(LandBlockState* state)
{
	f32 base[12];
	f32 mtx[12];

	njGetMatrix(base);
	njSetTexture(state->mTexList);
	drawLandModel(state->mRoot.mBuffer, base, state->mVisible, 1, 0);
	drawLandModel(state->mRoot.mBuffer, base, state->mVisible, 4, 0);

	if (sLandscapeEnabled != 0) {
		shadowTexGet()->load(5);
		shadowTexGet()->loadMtx(0x2D);
		njSetTexture(state->mTexList);
		setLandDrawHook((LandDrawHook)fadeHook);
		drawLandModel(state->mRoot.mBuffer, base, state->mVisibleFar, 1, 0);
		drawLandModel(state->mRoot.mBuffer, base, state->mVisibleFar, 4, 0);
		setLandDrawHook(0);
	}

	for (int i = 0; i < state->mBlock->mPartCount; i++) {
		LandMotionState* part = &state->mParts[i];
		mulLandMatrix(mtx, base, part->mMatrices, 1);
		if (part->mAnimIndex >= 0) {
			mulLandMatrix(mtx, mtx, sLandscapeObj->mAnimMatrices + part->mAnimIndex * 12, 1);
		}
		drawLandModel(part->mBuffer, mtx, 0, 1, 0);
		drawLandModel(part->mBuffer, mtx, 0, 4, 0);
	}
}

static void drawBlockAlpha(LandBlockState* state)
{
	f32 base[12];
	f32 mtx[12];

	njGetMatrix(base);
	njSetTexture(state->mTexList);
	drawLandModel(state->mRoot.mBuffer, base, state->mVisible, 2, 0);

	if (sLandscapeEnabled != 0) {
		shadowTexGet()->load(5);
		shadowTexGet()->loadMtx(0x2D);
		njSetTexture(state->mTexList);
		setLandDrawHook((LandDrawHook)fadeHook);
		drawLandModel(state->mRoot.mBuffer, base, state->mVisibleFar, 2, 0);
		setLandDrawHook(0);
	}

	for (int i = 0; i < state->mBlock->mPartCount; i++) {
		LandMotionState* part = &state->mParts[i];
		mulLandMatrix(mtx, base, part->mMatrices, 1);
		if (part->mAnimIndex >= 0) {
			mulLandMatrix(mtx, mtx, sLandscapeObj->mAnimMatrices + part->mAnimIndex * 12, 1);
		}
		drawLandModel(part->mBuffer, mtx, 0, 2, 0);
	}
}

static void drawVariant(LandBlockState* state)
{
	f32 base[12];
	f32 mtx[12];

	njGetMatrix(base);
	njSetTexture(state->mTexList);
	drawLandModel(state->mRoot.mBuffer, base, 0, 1, 0);

	for (int i = 0; i < state->mBlock->mPartCount; i++) {
		LandMotionState* part = &state->mParts[i];
		mulLandMatrix(mtx, base, part->mMatrices, 1);
		if (part->mAnimIndex >= 0) {
			mulLandMatrix(mtx, mtx, sLandscapeObj->mAnimMatrices + part->mAnimIndex * 12, 1);
		}
		drawLandModel(part->mBuffer, mtx, 0, 1, 0);
	}
}

static void drawOpaque()
{
	if (sLandscapeObj != 0) {
		drawBlockOpaque(&sLandscapeObj->mMain);
		if (blindIsOn(camGetDrawIndex()) == 0) {
			fogOff();
		}

		f32 blend              = sLandscapeCur->mBlend;
		LandBlockState* first  = sLandscapeObj->getVariant(0);
		LandBlockState* second = sLandscapeObj->getVariant(1);

		if (blend == 1.0f) {
			if (second == 0) {
				if (first != 0) {
					drawVariant(first);
				}
			} else {
				drawVariant(second);
			}
		} else if (blend == 0.0f) {
			if (first != 0) {
				drawVariant(first);
			}
		} else {
			sLandscapeAlpha = blend * 255.0f;
			if (second == 0) {
				if (first != 0) {
					drawVariant(first);
				}
			} else if (first != 0) {
				drawVariant(first);
				setLandDrawHook(blendHook);
				drawVariant(second);
				setLandDrawHook(0);
			}
		}
		fogApply();
	}
}

static void drawAlpha()
{
	if (sLandscapeObj != 0) {
		drawBlockAlpha(&sLandscapeObj->mMain);
	}
}
