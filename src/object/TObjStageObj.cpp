#include "stage/SetManager.h"
struct DrawTask;
#include "stage/SetManager.h"
#include "stage/SetManager.h"
#include "stage/SetManager.h"
#include "stage/SetManager.h"
#include "stage/SetManager.h"
#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "object/ObjArchive.h"
#include "object/ObjScript.h"
#include "object/ObjCommon.h"
#include "egg/ObjEgg.h"
#include "object/ObjModel.h"
#include "engine/ModelDraw.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "stage/StageObjUtil.h"
#include "object/ObjAction.h"
#include "core/dolphin.h"
#include "camera/TCamera.h"
#include "engine/Colli.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "sound/Sound3DPlay.h"
#include "engine/FxSystem.h"
#include "camera/TCamPath.h"
#include "egg/EggCreature.h"
#include "core/MathUtil.h"
#include "engine/GroundQuery.h"
#include "stage/SetSlot.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "engine/TParticle.h"
#include "core/VecMath.h"
#include "engine/LightSet.h"
#include "object/TObject.h"
#include "object/TObjStageObj.h"
#include "Ninja/ninja.h"
struct MeshColli;
struct MeshData;
struct MeshObject;
struct SetLink;
struct Fx;

// StageModelInfo::mFlags
enum StageModelFlags {
	STAGEMODEL_BREAK_PIECES = 0x1,  // breaks into pieces
	STAGEMODEL_LATE_DRAW    = 0x2,  // drawn in task list 0x22
	STAGEMODEL_FALLS        = 0x4,  // falls until it finds the ground
	STAGEMODEL_NO_PUSH      = 0x8,  // its shapes block nothing
	STAGEMODEL_HAS_FX       = 0x10,
	STAGEMODEL_BREAK_SHATTER = 0x20, // breaks by shattering the model
	STAGEMODEL_NO_FADE      = 0x40,
	STAGEMODEL_SHAPE_FLAG   = 0x200,
};

// One loaded model list
/** A contact on the collision's list at +0x80 */
struct StageContact {
	StageContact* mNext; // _00
	u8 _04[8];
	Vec3f mPos;          // _0C
};

/** What a model's node looks like when walking it to find its bounds */
struct StageModelNode {
	u8 _00[4];
	f32* mModel;           // _04 a sphere x y z r at _14-_20 of this
	Vec3f mPos;            // _08
	u8 _14[0x18];
	StageModelNode* mChild; // _2C
	StageModelNode* mNext;  // _30
};

static const char* sClassName = "TObjStageObj";

extern "C" {
extern const int sStageObjBreakSounds[] = {
	74, 75, 76, 77, 112, 626,
}; // the break sounds
}

enum StageObjAct {
	STAGEOBJ_IDLE   = 0,
	STAGEOBJ_WOBBLE = 1, // touched its motion runs at full speed
	STAGEOBJ_BROKEN = 2,
};

typedef char TObjStageObj_size_check[(sizeof(TObjStageObj) == 0x1CC) ? 1 : -1];

inline void TObjStageObj::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

const f32 gStageObjSetRange = 900.0f;

// 0x80066D60
extern "C" TObject* createTObjStageObj(TObject* parent, SetDesc* desc)
{
	return new TObjStageObj(parent, desc);
}

// 0x80066DB4
// Int 1 picks the bank 0 is the stage list and 1 the common list as it adds one
// "BOX" "METAL_BOX" and "WARA" ignore both and take a fixed common model
// Float 0 adds to a scale of 1 for models with no collision
void TObjStageObj::initFromDesc(SetDesc* desc)
{
	TSetActor::initFromDesc(desc);

	mBank     = (desc->getInt(1) + 1) % 2;
	mDropItem = ((SetDescObj*)desc)->get3E();
	mIndex    = desc->getInt(0);

	StageModelList* list = mBank == 0 ? gCommonModels : gStageModels;
	int index = mIndex;
	if (index < 0) {
		index = 0;
	} else {
		int max = list->mCount - 1;
		if (index > max) {
			index = max;
		}
	}
	mIndex = index;
	mInfo  = list->mModels[mIndex];

	if (strcmp(mInfo->mName, "BOX") == 0) {
		mBank  = 0;
		mIndex = 1;
	} else if (strcmp(mInfo->mName, "METAL_BOX") == 0) {
		mBank  = 0;
		mIndex = 2;
	} else if (strcmp(mInfo->mName, "WARA") == 0) {
		mBank  = 0;
		mIndex = 0;
	}
	list  = mBank == 0 ? gCommonModels : gStageModels;
	mInfo = list->mModels[mIndex];

	mBreakBank = desc->getInt(2);
	if (mBreakBank != 0) {
		mBreakBank = 8;
	} else {
		mBreakBank = 7;
	}
	mBreakId = desc->getInt(3);
	if (mBreakId == 0) {
		mBreakId = 0x38;
	}

	if (mInfo->mColli == -1) {
		mScale = 1.0f + desc->getFloat(0);
	} else {
		mScale = 1.0f;
	}
	mModelFlags = mInfo->mFlags;
	mModelId    = mInfo->mModel;
	mMotionId   = mInfo->mMotion;
	mColliId    = mInfo->mColli;

	if (mMotionId != -1) {
		mTrack.set(gGeObjCommon->mFix.mMotions[mMotionId], 1, 1.0f);
		mTrack.mRate = 0.2f;
	}

	computeBounds();
	updateMatrix();

	ObjCommonData* pack = mBank != 0 ? gGeObjCommon : gStgObjCommon;
	mDraw.setModel(pack->mFix.mModels[mModelId]);
	mDraw.mTexList   = pack->mFix.mTexLists[0];
	mDraw.mMatrix    = mMatrix;
	mDraw.mDrawState = &mDrawState;
	if (mMotionId != -1) {
		mDraw.mTrack = &mTrack;
	}
	createShapes();
}

// 0x800670C8
// The lines are the set-file labels
void TObjStageObj::debugPrint(void*)
{
	const char* banks[2] = { "BANK_COMMON", "BANK_STAGE" };
	njPrintC(0xA0014, banks[mBank]);
	njPrintC(0xA0013, mInfo->mName);
	njPrint(0xA0015, ":PABANK_BRK=%s", mBreakBank == 8 ? "STAGE" : "COMMON");
	njPrint(0xA0016, ":PAID_BRK=%d", mBreakId);
	njPrintC(0xA0017, "+1.0:SCALE");
}

// 0x80067188
// No model marks the object to be deleted
TObjStageObj::TObjStageObj(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00   = (u32)sClassName;
	mSize = 0x1CC;
	initFromDesc(desc);
	setLinkDone((SetLink*)&mLink);
	if ((mModelFlags & STAGEMODEL_LATE_DRAW) != 0) {
		mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	} else {
		mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	}
	if ((mModelFlags & STAGEMODEL_NO_FADE) != 0) {
		mLight.mTimer = 0;
	}
	mColli = 0;
	if (mInfo == 0) {
		this->mFlags |= 1;
		return;
	} else {
		vfn_74();
	}
}

// 0x800672B4
TObjStageObj::~TObjStageObj()
{
	if (mColli != 0) {
		meshColliFree(mColli);
	}
	if (mShape != 0) {
		colliRetire(mShape);
		mShape = 0;
	}
	removeDrawTask(mDrawTask);
}

// 0x80067368
// Resets the state the update functions keep and attaches the collision and an effect when the model has them
inline f32 minFloat(f32 a, f32 b)
{
	if (a < b) {
		return a;
	}
	return b;
}

void TObjStageObj::vfn_74()
{
	mAct         = STAGEOBJ_IDLE;
	mGroundTimer = 0;
	mFrames      = 0;
	mOnGround    = 0;
	mGroundPos   = mPos;

	if (mColliId != -1) {
		ObjCommonData* pack = mBank != 0 ? gGeObjCommon : gStgObjCommon;
		MeshData* data = (MeshData*)pack->mFix.mMeshes[mColliId];
		mColli = (MeshObject*)meshColliCreate();
		if ((mModelFlags & STAGEMODEL_FALLS) != 0) {
			meshSetFlag2(mColli, 1);
		}
		if (mRot.x != 0 || mRot.z != 0) {
			meshSetState100(mColli, 1);
		}
		meshColliSetData(mColli, data);
		meshColliSetMatrix(mColli, mMatrix);
		updateColliBox();
	}

	mFx = 0;
	if ((mModelFlags & STAGEMODEL_HAS_FX) != 0) {
		mFx = particleGenById(this, &mPos, mInfo->mFx[0], mInfo->mFx[1]);
	}
	mLightSet = lightSetFindArea(&mPos);

	mDrawDist = minFloat(850.0f, mInfo->mDrawDist);
}

static void growBounds(StageModelNode* node, Vec3f* low, Vec3f* high, const Vec3f* parent);

// 0x800674B0
void TObjStageObj::computeBounds()
{
	mBoxLow.x  = 0.0f;
	mBoxLow.y  = 0.0f;
	mBoxLow.z  = 0.0f;
	mBoxHigh.x = 0.0f;
	mBoxHigh.y = 0.0f;
	mBoxHigh.z = 0.0f;

	ObjCommonData* pack = mBank != 0 ? gGeObjCommon : gStgObjCommon;
	growBounds((StageModelNode*)pack->mFix.mModels[mModelId], &mBoxLow, &mBoxHigh, &gVecZero);

	Vec3f margin;
	margin.x = 10.0f;
	margin.y = 10.0f;
	margin.z = 10.0f;
	vecAdd(&mBoxHigh, &mBoxHigh, &margin);
	vecNegate(&margin, &margin);
	vecAdd(&mBoxLow, &mBoxLow, &margin);

	vecSub(&mCentre, &mBoxHigh, &mBoxLow);
	vecScale(0.5f, &mCentre, &mCentre);
	mRadius = vecLength(&mCentre);
	vecAdd(&mCentre, &mCentre, &mBoxLow);
}

// 0x800675A8
// Grows the box to hold every node of the model
static void growBounds(StageModelNode* node, Vec3f* low, Vec3f* high, const Vec3f* parent)
{
	while (node != 0) {
		Vec3f pos;
		Vec3f centre;
		Vec3f offset;
		offset.set(node->mPos.x, node->mPos.y, node->mPos.z);
		vecAdd(&pos, &offset, parent);
		f32* model = node->mModel;
		if (model != 0) {
			f32 radius = model[8];
			Vec3f local;
			local.set(model[5], model[6], model[7]);
			vecAdd(&centre, &pos, &local);
			if (centre.x - radius < low->x) {
				low->x = centre.x - radius;
			}
			if (centre.y - radius < low->y) {
				low->y = centre.y - radius;
			}
			if (centre.z - radius < low->z) {
				low->z = centre.z - radius;
			}
			if (centre.x + radius > high->x) {
				high->x = centre.x + radius;
			}
			if (centre.y + radius > high->y) {
				high->y = centre.y + radius;
			}
			if (centre.z + radius > high->z) {
				high->z = centre.z + radius;
			}
		}
		growBounds(node->mChild, low, high, &pos);
		node = node->mNext;
	}
}

// 0x80067700
void TObjStageObj::updateMatrix()
{
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	mtxRotateX(mMatrix, mMatrix, mRot.x);
	mtxScale(mScale, mScale, mScale, mMatrix, mMatrix);

	vecScale(mScale, &mWorldCentre, &mCentre);
	vecRotateY(&mWorldCentre, &mWorldCentre, mRot.y);
	vecAdd(&mWorldCentre, &mWorldCentre, &mPos);
	mWorldRadius = mRadius * mScale;
}

// 0x800677AC
void TObjStageObj::updateColliBox()
{
	f32 box[6];
	boxAroundBox(box, mMatrix, &mBoxLow, &mBoxHigh);
	meshColliSetBox(mColli, box);
}

// 0x800677F4
// Any contact starts the wobble
int TObjStageObj::checkContacts()
{
	if (mShape == 0) {
		return 0;
	}
	StageContact* contact = (StageContact*)mShape->mContacts;
	int touched = 0;
	while (contact != 0) {
		touched = 1;
		if (mMotionId != -1 && mAct != STAGEOBJ_WOBBLE) {
			mTrack.mRate = 1.0f;
			mAct = STAGEOBJ_WOBBLE;
		}
		if (colliContactHasBit8((ColliContact*)contact) != 0
		    && ((mModelFlags & STAGEMODEL_BREAK_PIECES) != 0 || (mModelFlags & STAGEMODEL_BREAK_SHATTER) != 0)
		    && findObjById(colliContactOwner((ColliContact*)contact))->getKind() != 0) {
			Vec3f hit;
			Vec3f drop = *getPos();
			dropItem(mDropItem, &drop);

			if ((mModelFlags & STAGEMODEL_BREAK_PIECES) != 0) {
				hit = contact->mPos;
				shatterBank(mScale, this, &hit, &mPieceSize, mInfo->mPieces);
			} else {
				ObjCommonData* pack = mBank != 0 ? gGeObjCommon : gStgObjCommon;
				hit = contact->mPos;
				shatterModel(this, &hit, getRotY(), pack->mFix.mModels[mModelId], pack->mFix.mTexLists[0], mScale, 1.5f);
			}
			sePlay3D(sStageObjBreakSounds[mInfo->mSound], &mPos);
			fxSpawnAt(1, mBreakBank, mBreakId, mMatrix);
			if (mColli != 0) {
				meshColliFree(mColli);
				mColli = 0;
			}
			TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
			if (other != 0) {
				struct { ObjMsg m; u8 pad[6]; } a;
				a.m.mType = 0x2A;
				other->sendToHandle(&a.m);
			}
			if (other != 0) {
				ObjMsg b;
				b.mType = 0x74;
				other->sendToHandle(&b);
			}
			mAct = STAGEOBJ_BROKEN;
			return 1;
		}
		contact = contact->mNext;
	}
	return touched;
}

// 0x80067A6C
// A falling object only looks down once a second while it is on the ground
void TObjStageObj::updateIdle()
{
	checkContacts();
	if (mMotionId != -1) {
		mTrack.update();
	}
	if ((mModelFlags & STAGEMODEL_FALLS) != 0) {
		if (mOnGround != 0) {
			mGroundTimer++;
			if (mGroundTimer > 60) {
				mOnGround    = checkGround();
				mGroundTimer = 0;
			}
		} else {
			fall();
			updateMatrix();
			if (mColli != 0) {
				meshColliSetMatrix(mColli, mMatrix);
				updateColliBox();
			}
			updateShapes();
			mOnGround = checkGround();
		}
	}
}

// 0x80067B34
// Half a second after the last touch it slows back down
void TObjStageObj::updateWobble()
{
	int touched = checkContacts();
	mTrack.update();
	if (mFrames > 30) {
		mTrack.mRate = 0.2f;
		if (touched == 0) {
			mFrames = 0;
			mAct    = STAGEOBJ_IDLE;
		}
	} else {
		mFrames++;
	}
}

// 0x80067BA0
void TObjStageObj::updateBroken()
{
	colliDisable(mShape);
	if (objCountChildren(this) == 0) {
		mFlags |= 1;
	}
}

// 0x80067BEC
void TObjStageObj::fall()
{
	mFrames += 1;
	f32 step = -0.16333334f * mFrames;
	f32 toGround = mGroundPos.y - mPos.y;
	if (!(toGround < step)) {
		mFrames = 0;
		step = toGround;
	}
	mPos.y += step;
}

// 0x80067C58
// Whether the ground is right below it and where
int TObjStageObj::checkGround()
{
	EggHits hits;
	Vec3f pos = mPos;
	pos.y += 5.0f;
	if (meshColliProbe((EggHit*)&hits, (const Vec3f*)&pos, -1) != 0) {
		mGroundPos = hits.mHits[0].mPos;
		return 5.0f == pos.y - mGroundPos.y;
	}
	return 0;
}

// 0x80067D18
void TObjStageObj::createShapes()
{
	if (mShape != 0) {
		colliRetire(mShape);
		mShape = 0;
	}
	int count = mInfo->mShapeCount;
	if (count != 0) {
		int mask = 0xFF;
		if ((mModelFlags & STAGEMODEL_NO_PUSH) != 0) {
			mask = 0;
		}
		mShape = colliCreate(count, 1);
		colliSetOwner(mShape, (u16)mObjId);
		colliSetMasks(mShape, 4, 0xFF, mask, 0xD4);
		updateShapes();
	} else {
		mPieceSize.x = 15.0f;
		mPieceSize.y = 15.0f;
		mPieceSize.z = 15.0f;
	}
}

// 0x80067DC8
// Builds the shapes from the record at the current scale
void TObjStageObj::updateShapes()
{
	int count = mInfo->mShapeCount;
	if (count == 0) {
		return;
	}
	ColliShape* shapes = colliGetShape(mShape, 0);
	for (int i = 0; i < count; i++) {
		StageShapeInfo* info  = mInfo->mShapes[i];
		int type = info->mType;
		f32 x = info->mSizeX;
		f32 y = info->mSizeY;
		f32 z = info->mSizeZ;
		colliShapeInit(&shapes[i]);
		switch (type) {
		case 0:
			shapes[i].mType  = 0;
			shapes[i].mSizeX = shapes[i].mSizeY = shapes[i].mSizeZ = x * mScale;
			shapes[i].mPos.x = mScale * mInfo->mShapes[i]->mPos.x;
			shapes[i].mPos.y = mScale * mInfo->mShapes[i]->mPos.y;
			shapes[i].mPos.z = mScale * mInfo->mShapes[i]->mPos.z;
			break;
		case 1:
			shapes[i].mType  = 1;
			shapes[i].mSizeX = x * mScale;
			shapes[i].mSizeY = y * mScale;
			shapes[i].mSizeZ = z * mScale;
			shapes[i].mPos.x = mScale * mInfo->mShapes[i]->mPos.x;
			shapes[i].mPos.y = mScale * mInfo->mShapes[i]->mPos.y;
			shapes[i].mPos.z = mScale * mInfo->mShapes[i]->mPos.z;
			break;
		case 2:
			shapes[i].mType  = 2;
			shapes[i].mSizeX = shapes[i].mSizeZ = x * mScale;
			shapes[i].mSizeY = y * mScale;
			shapes[i].mPos.x = mScale * mInfo->mShapes[i]->mPos.x;
			shapes[i].mPos.y = mScale * mInfo->mShapes[i]->mPos.y;
			shapes[i].mPos.z = mScale * mInfo->mShapes[i]->mPos.z;
			break;
		}
		if (((mModelFlags & STAGEMODEL_NO_PUSH) == 0 && mColliId == -1) || mColliId != -1) {
			shapes[i].mFlags |= 0x10;
		}
		if ((mModelFlags & STAGEMODEL_SHAPE_FLAG) != 0) {
			shapes[i].mFlags |= 8;
		}
		if ((mModelFlags & STAGEMODEL_BREAK_PIECES) != 0 || (mModelFlags & STAGEMODEL_BREAK_SHATTER) != 0) {
			shapes[i].mFlags |= 0x800;
		}
	}
	colliPlaceShapesRotY(mShape, &mPos, mRot.y, shapes);
	colliRefreshBounds(mShape);
	mPieceSize.x = shapes->mSizeX;
	mPieceSize.y = shapes->mSizeY;
	mPieceSize.z = shapes->mSizeZ;
}

// 0x800680BC
// The model's hit record says who can hit it by the attacker's kind
int TObjStageObj::onHit(const TObjAction* other, int, int* result)
{
	resetHitInfo((HitInfo*)result);
	((HitInfo*)result)->mDamage = mInfo->mHit->mDamage;
	if ((mInfo->mHit->mKinds & 1) != 0 && ((TObjAction*)other)->getKind() == 0) {
		return 1;
	}
	if ((mInfo->mHit->mKinds & 2) != 0 && ((TObjAction*)other)->getKind() == 1) {
		((HitInfo*)result)->mFlags.setBit(0xD);
		return 1;
	}
	if ((mInfo->mHit->mKinds & 4) != 0 && ((TObjAction*)other)->getKind() == 2) {
		return 1;
	}
	return 0;
}

// 0x800681BC
// Between the draw distance and 950 it just waits
void TObjStageObj::draw()
{
	if (!isInRange(this, &mPos, 100.0f + mDrawDist)) {
		if (!isInRange(this, &mPos, 950.0f)) {
			if (mAct != STAGEOBJ_BROKEN) {
				setLinkFree((SetLink*)&mLink);
			}
			mFlags |= 1;
		}
		return;
	}
	switch (mAct) {
	case STAGEOBJ_IDLE:
		updateIdle();
		break;
	case STAGEOBJ_WOBBLE:
		updateWobble();
		break;
	case STAGEOBJ_BROKEN:
		updateBroken();
		break;
	}
}

// 0x80068284
// Flag 2 of the model's flags sorts it by depth and the rest are lit
void TObjStageObj::vfn_10()
{
	if ((mInfo->mFlags & 2) != 0) {
		Vec3f view;
		if (mLight.checkView(mDrawDist, mWorldRadius, &mWorldCentre, &view) != 0 && mAct != STAGEOBJ_BROKEN) {
			mDrawTask->mSortKey = drawTaskDepthKey(view.z);
			mDrawTask->mFlags &= ~1;
		} else {
			mDrawTask->mFlags |= 1;
		}
	} else if (mLight.isVisible(mDrawTask, mDrawDist, &mWorldCentre, mWorldRadius, 0) != 0 && mAct != STAGEOBJ_BROKEN) {
		mDrawTask->mFlags &= ~1;
		setDrawAmbient(&mLight, &mDrawState);
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x80068380
void TObjStageObj::render()
{
	lightSetSelect(mLightSet);
	mDraw.draw();
	if (objDebugEnabled(this) != 0 && mShape != 0) {
		colliDrawShapes(mShape);
	}
}
