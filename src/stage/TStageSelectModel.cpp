#include "ui/MenuModelArc.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "io/LangFile.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "stage/TStageSelectModel.h"
#include "engine/FxSystem.h"
#include "object/TObjLauncher.h"
#include "egg/EggCreature.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "ui/AeArc.h"
struct NJS_OBJECT;
struct Fx;

typedef char TStageSelectModel_size_check[(sizeof(TStageSelectModel) == 0x140) ? 1 : -1];

// One node of the index-1 model
struct SelectNode {
	u8 _00[8];     // _00
	Vec3f mPos;    // _08
	int mAngle[3]; // _14
	u8 _20[0xC];   // _20
	SelectNode* mChild; // _2C
	SelectNode* mNext;  // _30
};

const char* gTStageSelectModelName = "TStageSelectModel";

TArc* gMenuModelArc;
MenuModelSet* gMenuModelSet;
extern "C" {
BinFile* sKeikokuFile; // mes_keikoku.bin
BinFile* sMissionTextFile; // mission_text.bin
int sKeikokuCount;
char** sKeikokuMsgs;
int sMissionTextCount;
char** sMissionTextMsgs;
}

void loadLandModelArc()
{
	loadArc("menu_landmodel.arc", &gMenuModelArc);
	gMenuModelSet = (MenuModelSet*)gMenuModelArc->mData;
	bindGvmTextures((u8*)gMenuModelSet->mGvms[0], (NJS_TEXLIST*)gMenuModelSet->mTexLists[0]);
	bindGvmTextures((u8*)gMenuModelSet->mGvms[1], (NJS_TEXLIST*)gMenuModelSet->mTexLists[1]);
}

void loadBattleModelArc()
{
	loadArc("menu_battlemodel.arc", &gMenuModelArc);
	gMenuModelSet = (MenuModelSet*)gMenuModelArc->mData;
	bindGvmTextures((u8*)gMenuModelSet->mGvms[0], (NJS_TEXLIST*)gMenuModelSet->mTexLists[0]);
}

void freeLandModelArc()
{
	releaseTextures((NJS_TEXLIST*)gMenuModelSet->mTexLists[1]);
	releaseTextures((NJS_TEXLIST*)gMenuModelSet->mTexLists[0]);
	releaseResource(gMenuModelArc);
}

void freeBattleModelArc()
{
	releaseTextures((NJS_TEXLIST*)gMenuModelSet->mTexLists[0]);
	releaseResource(gMenuModelArc);
}

// 0x80179420
extern "C" TStageSelectModel* createTStageSelectModel(TObject* parent, const Vec3f* pos, const int* angle, int index, u16 priority)
{
	return new TStageSelectModel(parent, pos, (int*)angle, index, priority);
}

/** The three angles copied as one */
struct AngleTriple {
	int v[3];
};

// 0x8017948C
// angle is not const its loads wait for the pos stores
TStageSelectModel::TStageSelectModel(TObject* parent, const Vec3f* pos, int* angle, int index, u16 priority)
    : TObject(parent)
{
	mIndex = index;
	*(SetRot*)&mPos = *(const SetRot*)pos;
	*(AngleTriple*)mAngle = *(AngleTriple*)angle;
	_7C  = 0;
	_9C  = 0;
	_00  = (u32)gTStageSelectModelName;
	mSize = 0x140;
	mDraw = addDrawTask(this, (void (*)(void*))drawObjectTask, (u16)priority, 0);
	setup();
}

// 0x8017954C
TStageSelectModel::~TStageSelectModel()
{
	setMaterial(0);
	fxRemove((Fx*)mBurst);
	mBurst = 0;
	fxRemoveOwner(mMtxA);
	mFx = 0;
	fxRemoveOwner(mMtxB);
	mFxB = 0;
	removeDrawTask(mDraw);
}

// 0x801795EC
void TStageSelectModel::setup()
{
	updateMatrix();
	mScale = 1.0f;
	_24    = 0;
	mSaved = *(u32*)(*(u8**)gMenuModelSet->mTexLists[0] + 0x1AC);
	mPlaying = 1;
	mFrame   = 0.0f;
	mRate    = 1.0f;
	mBurst   = 0;
	mFx      = 0;
	mFxB     = 0;
	mLightColor.x = mLightColor.y = mLightColor.z = 1.0f;
	mAmbient.x = mAmbient.y = mAmbient.z = 0.5f;
	mLightX = 0.5f;
	mLightY = -1.0f;
	mLightZ = 0.0f;
}

// 0x8017968C
void TStageSelectModel::draw()
{
	updateMatrix();
	if (mPlaying != 0) {
		mFrame += mRate;
		u32 count = *(u32*)((u8*)gMenuModelSet->mMotions[mIndex] + 4);
		if (mFrame > (f32)count) {
			mFrame -= (f32)count;
		}
	}
}

// 0x80179728
void TStageSelectModel::vfn_10() { }

// 0x8017972C
void TStageSelectModel::render()
{
	gjSetAmbient(mAmbient.x, mAmbient.y, mAmbient.z);
	gjSetLightVector(0, mLightX, mLightY, mLightZ);
	gjSetLightColor(0, mLightColor.x, mLightColor.y, mLightColor.z);
	gjSetLightSwitch(0, 1);
	gjLoadLight(0);
	if (mIndex < gMenuModelSet->mCount) {
		f32 view[12];
		mtxConcat(view, (const f32*)gjGetMatrix(), mMtx);
		njPushMatrix(view);
		njSetTexture(gMenuModelSet->mTexLists[0]);
		if (mPlaying != 0) {
			gjDrawMotion((NJS_OBJECT*)gMenuModelSet->mModels[mIndex], gMenuModelSet->mMotions[mIndex], mFrame);
		} else if (mIndex == 0xB) {
			njZBufferMode(0);
			gjDrawObject((NJS_OBJECT*)gMenuModelSet->mModels[mIndex]);
			njZBufferMode(1);
		} else {
			gjDrawObject((NJS_OBJECT*)gMenuModelSet->mModels[mIndex]);
		}
		njPopMatrixEx();
	}
}

// 0x80179864
void TStageSelectModel::updateMatrix()
{
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX(mMtx, mMtx, mAngle);
	mtxCopy(mMtxA, mMtx);
	mtxCopy(mMtxB, mMtx);
	mtxScale(mScale, mScale, mScale, mMtx, mMtx);
	if (mIndex == 1) {
		applyNodes((SelectNode*)gMenuModelSet->mModels[mIndex], 0);
	}
}

// 0x80179900
// Depth 7 writes mMtxA depth 0xD writes mMtxB
void TStageSelectModel::applyNodes(SelectNode* node, int depth)
{
	do {
		if (depth == 7) {
			f32 tmp[12];
			Vec3f pos;
			SetRot rot;
			pos.set(node->mPos.x, node->mPos.y, node->mPos.z);
			rot.set(node->mAngle[0], node->mAngle[1], node->mAngle[2]);
			mtxSetTransV(tmp, &pos);
			mtxRotateZYX(tmp, tmp, (int*)&rot);
			mtxConcat(mMtxA, mMtxA, tmp);
		} else if (depth == 0xD) {
			f32 tmp[12];
			Vec3f pos;
			SetRot rot;
			pos.set(node->mPos.x, node->mPos.y, node->mPos.z);
			rot.set(node->mAngle[0], node->mAngle[1], node->mAngle[2]);
			mtxSetTransV(tmp, &pos);
			mtxRotateZYX(tmp, tmp, (int*)&rot);
			mtxConcat(mMtxB, mMtxB, tmp);
		}
		depth += 1;
		if (node->mChild != 0) {
			applyNodes(node->mChild, depth);
		}
		njPopMatrixEx();
		node = node->mNext;
	} while (node != 0);
}

// 0x80179A28
void TStageSelectModel::attachBurst(const f32* mtx)
{
	mBurst = fxSpawnAt(1, 6, 0xC, mtx);
}

// 0x80179A68
void TStageSelectModel::spawnIndex2()
{
	if (mIndex == 2 && mFx == 0) {
		mFx = fxSpawnFollow(1, 6, 4, mMtxA);
		fxSetOwner(mFx, (void*)mMtxA);
	}
}

// 0x80179ACC
void TStageSelectModel::spawnIndex5()
{
	if (mIndex == 5 && mFx == 0) {
		mFx = fxSpawnFollow(1, 6, 5, mMtxA);
		fxSetOwner(mFx, mMtxA);
	}
}

// 0x80179B30
void TStageSelectModel::spawnIndex4()
{
	if (mIndex == 4 && mFx == 0) {
		mFx = fxSpawnFollow(1, 6, 9, mMtxA);
		fxSetOwner(mFx, mMtxA);
	}
}

// 0x80179B94
void TStageSelectModel::spawnIndex1()
{
	if (mIndex == 1 && mFx == 0) {
		mFx = fxSpawnFollow(1, 6, 0, mMtxA);
		fxSetOwner(mFx, mMtxA);
		mFxB = fxSpawnFollow(1, 6, 2, mMtxB);
		fxSetOwner(mFxB, (void*)mMtxB);
	}
}

// 0x80179C1C
void TStageSelectModel::spawnIndex6()
{
	if (mIndex == 6 && mFx == 0) {
		mFx = fxSpawnFollow(1, 6, 0x12, mMtxA);
		fxSetOwner(mFx, mMtxA);
	}
}

// 0x80179C80
// 0 restores the saved word
void TStageSelectModel::setMaterial(int kind)
{
	switch (kind) {
	case 0:
		*(u32*)(*(u8**)gMenuModelSet->mTexLists[0] + 0x1AC) = mSaved;
		break;
	case 1:
		*(u32*)(*(u8**)gMenuModelSet->mTexLists[0] + 0x1AC) = *(u32*)(*(u8**)gMenuModelSet->mTexLists[1] + 8);
		break;
	case 2:
		*(u32*)(*(u8**)gMenuModelSet->mTexLists[0] + 0x1AC) = *(u32*)(*(u8**)gMenuModelSet->mTexLists[1] + 0x14);
		break;
	case 3:
		*(u32*)(*(u8**)gMenuModelSet->mTexLists[0] + 0x1AC) = *(u32*)(*(u8**)gMenuModelSet->mTexLists[1] + 0x20);
		break;
	case 4:
		*(u32*)(*(u8**)gMenuModelSet->mTexLists[0] + 0x1AC) = *(u32*)(*(u8**)gMenuModelSet->mTexLists[1] + 0x2C);
		break;
	}
}

// 0x80179D58
// Turns the two message files into pointer tables
extern "C" void loadStageSelectText()
{
	u32* data;
	u32 text;
	int i;
	u32 size;

	loadSizedResource(getLangFileName("mes_keikoku.bin"), &sKeikokuFile);
	data = (u32*)sKeikokuFile->mData;
	text = *data;
	text += (u32)data++;
	size = *data++;
	sKeikokuMsgs = (char**)data;
	sKeikokuCount = size >> 2;
	for (i = 0; i < sKeikokuCount; i++) {
		sKeikokuMsgs[i] += text + 8;
	}

	loadSizedResource(getLangFileName("mission_text.bin"), &sMissionTextFile);
	data = (u32*)sMissionTextFile->mData;
	text = *data;
	text += (u32)data++;
	size = *data++;
	sMissionTextMsgs = (char**)data;
	sMissionTextCount = size >> 2;
	for (i = 0; i < sMissionTextCount; i++) {
		sMissionTextMsgs[i] += text + 8;
	}
}

// 0x80179E4C
// Name is ours
extern "C" void freeStageSelectText()
{
	if (sKeikokuFile != 0) {
		releaseResource(sKeikokuFile);
		sKeikokuFile = 0;
	}
	if (sMissionTextFile != 0) {
		releaseResource(sMissionTextFile);
		sMissionTextFile = 0;
	}
}

// 0x80179E98
// Name is ours
extern "C" char* getStageSelectWarning(int index)
{
	if (index < sKeikokuCount) {
		return sKeikokuMsgs[index];
	}
	return 0;
}

// 0x80179EBC
// Name is ours
extern "C" char* getStageSelectMission(int index)
{
	if (index < sMissionTextCount) {
		return sMissionTextMsgs[index];
	}
	return 0;
}
