#include "animal/AniAttacks.h"
struct DrawTask;
#include "io/Resource.h"
#include "engine/DrawTask.h"
#include "stage/StageObjUtil.h"
#include "enemy/boss/EneBoss.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "object/TObjLauncher.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "sound/Sound3DPlay.h"
#include "Ninja/ninja.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "engine/DrawState.h"
#include "engine/TParticle.h"
#include "object/TObject.h"
#include "animal/TAniIceRock.h"
struct DrawStateSlot;
struct DrawState;
struct NJS_OBJECT;
struct CharaColli;

const char* gTAniIceRockName = "TAniIceRock";

// 0x8013BE9C
TAniIceRock::TAniIceRock(TObject* parent, const Vec3f* pos, u16 ownerId)
    : TObject(parent), mPos(*pos)
{
	_00               = (u32)gTAniIceRockName;
	mSize             = 0xCC;
	mDrawTask         = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	if (aniAssetAttach(2, &mArc) != 0) {
		TArc* arc = mArc.mFile->mArc;
		if (arc != 0) {
			mAssets = (void**)findArcExport(arc, "AniModel");
			int pick = (int)(10.0f * (3.0517578e-5f * rand())) % 3;
			mObject  = (NJS_OBJECT*)(((void***)mAssets)[0][2 + pick]);
			mTexList = (NJS_TEXLIST*)((void**)mAssets[5])[0];
		}
	} else {
		mFlags |= 1;
	}
	mDrawState          = *getDefaultDrawState();
	mDrawState.mFlags   = 0x200;
	mDrawState.mAmbient = 0xFF;
	mRise               = -20.0f;
	mRotY               = (int)(65536.0f * (3.0517578e-5f * rand()));
	mScale              = 1.0f;
	mTimer              = 0x1E;
	mDone               = 0;
	mHolderId           = 0;
	mFlags2             = 0;

	AniParams* params = (AniParams*)gAnimalParams[2];
	if (params == 0) {
		mFlags |= 1;
		return;
	}
	f32* values = params->mValues;
	if ((int)values[4] == 0) {
		colliShapeMakeSphere(&mShapeData, &gVecZero, 0, values[5]);
	} else {
		colliShapeMakeCylinder(&mShapeData, &gVecZero, 0, values[5], values[6]);
	}
	mColli = (CharaColli*)colliCreate(1, 1);
	if (mColli != 0) {
		colliSetOwner(mColli, (u16)ownerId);
		colliSetMasks(mColli, 0x20, 0xFF, 0xFF, 0x10);
		colliCopyShapes(mColli, &mShapeData);
		colliPlaceShapes(mColli, &mPos, &mShapeData);
		colliShapeSetBit8(mColli, 0);
	}
	mState = 0;
}

// 0x8013C19C
TAniIceRock::~TAniIceRock()
{
	aniAssetDetach(&mArc);
	colliFree(mColli);
	removeDrawTask(mDrawTask);
}

// 0x8013C214
void TAniIceRock::draw()
{
	switch (mState) {
	case 0:
		runRise();
		break;
	case 1:
		runGone();
		break;
	}
}

// 0x8013C258
void TAniIceRock::runRise()
{
	if (mRise < 0.0f) {
		mRise += 4.0f;
	} else {
		mRise = 0.0f;
		if ((mFlags2 & 1) != 0) {
			if (mTimer-- == 0) {
				Vec3f size;
				size.x = 15.0f;
				size.y = 15.0f;
				size.z = 15.0f;
				f32 m[12];
				mtxSetRotY(m, mRotY);
				mtxMultVecSR(m, &size, &size);
				IceBurst pieces;
				pieces.mModels   = (void**)mAssets[0];
				pieces.mTexture  = mTexList;
				pieces.mFirst    = 5;
				pieces.mVariants = 3;
				pieces.mCount    = 10;
				pieces.mScale    = 2.0f;
				pieces.mSpeed    = 3.0f;
				burst(&size, &pieces);
				particleOnceByIdDefault(&mPos, 0, 0x99);
				if ((mFlags2 & 2) != 0) {
					camShakeHard(mHolderId);
					sePlay3D(0x1B2, &mPos);
				}
				mFlags |= 0x10;
				mState = 1;
			}
		} else if (fadeAmbient(&mDrawState, 0, 8) != 0) {
			mFlags |= 1;
		}
	}

	if (mTimer-- == 0) {
		colliDisable(mColli);
	} else {
		Vec3f at = mPos;
		at.y += mRise;
		colliGetShapes(mColli)->mPos = at;
		f32 size = mShapeData.mSizeX * mScale;
		colliGetShapes(mColli)->mSizeX = size;
	}
}

// 0x8013C438
void TAniIceRock::runGone()
{
	if (objCountChildren(this) == 0) {
		mDone = 1;
	}
}

// 0x8013C474
void TAniIceRock::burst(const Vec3f* size, const IceBurst* pieces)
{
	Vec3f centre;
	centre = mPos;
	f32 lowX  = -size->x;
	f32 lowZ  = -size->z;
	f32 lowY  = 0.5f * size->y;
	f32 widthX = size->x - lowX;
	f32 widthY = 2.0f * size->y - lowY;
	f32 widthZ = size->z - lowZ;
	f32 rate   = centre.y - 100.0f;
	f32 extra  = pieces->mSpeed - 1.0f;
	int variants = pieces->mVariants;
	int count    = pieces->mCount;
	for (int i = 0; i < count; i++) {
		Vec3f at;
		at.x = centre.x + (widthX * (3.0517578e-5f * rand()) + lowX);
		at.y = centre.y + (widthY * (3.0517578e-5f * rand()) + lowY);
		at.z = centre.z + (widthZ * (3.0517578e-5f * rand()) + lowZ);
		Vec3f out;
		gjSubVector(&out, &at, &centre);
		Vec3f dir;
		dir = out;
		if (njUnitVector(&dir) < 1e-6f) {
			dir = gVecZero;
		}
		gjScaleVector(extra * (3.0517578e-5f * rand()) + 1.0f, &dir, &dir);
		int pick  = rand() % variants;
		void* mem = objHeapAlloc(gObjHeap, 0xE4);
		if (mem != 0) {
			new (mem) TObjLauncher(this, rate, pieces->mScale, &at, &dir, pieces->mModels[pieces->mFirst + pick], pieces->mTexture);
		}
	}
}

// 0x8013C760
void TAniIceRock::vfn_10()
{
	Vec3f view;
	if (cameraIsVisible(1000.0f, 50.0f, &mPos, &view) != 0) {
		mDrawTask->mSortKey = drawTaskDepthKey(view.z);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8013C7D8
void TAniIceRock::render()
{
	DrawStateSlot* slot = getDrawStateSlot();
	njSetTexture(mTexList);
	njPushMatrixEx();
	njTranslateEx(&mPos);
	gjRotateY(mRotY);
	njScale(0, mScale, mScale, mScale);
	gjTranslate(0.0f, mRise, 0.0f);
	beginDrawState(slot, (const DrawState*)&mDrawState);
	gjDrawObject(mObject);
	endDrawState((DrawStateSlot*)slot);
	njPopMatrixEx();
}
