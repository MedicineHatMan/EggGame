#include "object/green/TObjGreenGrassGroup.h"
struct DrawTask;
#include "sound/SeHolder.h"
#include "object/ObjAction.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjCommon.h"
#include "engine/Colli.h"
#include "core/MathUtil.h"
#include "object/green/GreenObjBank.h"
#include "camera/TCamera.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "core/VecMath.h"
#include "core/SdkCalls.h"
struct CharaColli;
struct ColliContact;

inline void TObjGreenGrassGroup::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x8017F184
extern "C" TObjGreenGrassGroup* createTObjGreenGrassGroup(TObject* parent, Vec3f* pos)
{
	return new TObjGreenGrassGroup(parent, pos);
}

const char* gTObjGreenGrassGroupName = "TObjGreenGrassGroup";

// 0x8017F1D8
TObjGreenGrassGroup::TObjGreenGrassGroup(TObject* parent, const Vec3f* pos)
    : TSetActor(parent)
{
	_00       = (u32)gTObjGreenGrassGroupName;
	mSize     = 0x12C;
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mShape    = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks((CharaColli*)mShape, 4, 0xFF, 0xFF, 0xF6);
	mPos     = *pos;
	mHitFrom = *pos;
	mRot.x   = 0;
	mRot.y   = 0;
	mRot.z   = 0;
	makeShape(pos);
	mGrassFlags = 0;
	createBlades();
	readCamera();
	sortBlades();
}

// 0x8017F2D8
TObjGreenGrassGroup::~TObjGreenGrassGroup()
{
	destroyBlades();
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDrawTask);
}

// 0x8017F354
void TObjGreenGrassGroup::makeShape(const Vec3f* pos)
{
	ColliShape shape;

	colliShapeInit(&shape);
	colliShapeSetSphere(&shape);
	colliShapeSetRadius(&shape, 23.0f);
	shape.mFlags |= 0x10;
	colliShapeSetPos(&shape, pos);
	colliCopyShapes(mShape, &shape);
}

// 0x8017F3C0
// The first contact from a shape without flag 0x10
GrassContact* TObjGreenGrassGroup::findContact()
{
	GrassContact* contact = (GrassContact*)mShape->mContacts;
	while (contact != 0) {
		if ((colliGetShape((CharaColli*)contact->mColli, contact->mShape)->mFlags & 0x10) == 0) {
			return contact;
		}
		contact = contact->mNext;
	}
	return 0;
}

// 0x8017F420
void TObjGreenGrassGroup::createBlades()
{
	for (int i = 0; i < 20; i++) {
		mBlades[i] = createGrassBlade(25.0f, &mPos, i);
	}
}

// 0x8017F478
void TObjGreenGrassGroup::destroyBlades()
{
	for (int i = 0; i < 20; i++) {
		if (mBlades[i] != 0) {
			delete mBlades[i];
			mBlades[i] = 0;
		}
	}
}

// 0x8017F4D8
int TObjGreenGrassGroup::isAllCut()
{
	for (int i = 0; i < 20; i++) {
		if (mBlades[i]->isCut() == 0) {
			return 0;
		}
	}
	return 1;
}

// 0x8017F538
void TObjGreenGrassGroup::resetBlades()
{
	for (int i = 0; i < 20; i++) {
		mBlades[i]->reset();
	}
}

// 0x8017F584
void TObjGreenGrassGroup::pushBlades(f32 radius, const Vec3f* pos)
{
	if (distXZ(&mPos, pos) < 25.0f + radius) {
		for (int i = 0; i < 20; i++) {
			mBlades[i]->push(radius, pos);
		}
	}
}

// 0x8017F608
// The sound can't play nothing ever sets the flag it tests
void TObjGreenGrassGroup::cutBlades(TObjAction* other)
{
	int sound = 0;

	for (int i = 0; i < 20; i++) {
		if (mBlades[i]->cut(other) == 0) {
			sound = 0;
		}
	}
	if (sound != 0) {
		seStartAt(0x182, &mPos);
	}
}

// 0x8017F680
void TObjGreenGrassGroup::bendBlades(TObjAction* other)
{
	for (int i = 0; i < 20; i++) {
		mBlades[i]->bend(other);
	}
}

// 0x8017F6D4
int TObjGreenGrassGroup::countUncut()
{
	int count = 0;

	if ((mGrassFlags & GRASS_CUT) == 0) {
		for (int i = 0; i < 20; i++) {
			if (mBlades[i]->isCut() == 0) {
				count++;
			}
		}
	}
	return count;
}

// 0x8017F740
void TObjGreenGrassGroup::updateBlades()
{
	for (int i = 0; i < 20; i++) {
		mBlades[i]->update();
	}
}

// 0x8017F78C
void TObjGreenGrassGroup::drawBlades()
{
	if ((mGrassFlags & GRASS_SPARSE) != 0) {
		njDrawTexture3DExStart(1);
	} else {
		njDrawTexture3DExStart(0);
	}
	gjLoadTextureNum(4, 1, 1, 0);
	for (int i = 0; i < 20; i++) {
		if ((mGrassFlags & GRASS_SPARSE) != 0) {
			if (i % 2 == 0) {
				mBlades[mOrder[i]]->drawFaded(0x5F000000);
			}
		} else {
			mBlades[mOrder[i]]->draw();
		}
	}
	njDrawTexture3DExEnd();
}

// 0x8017F85C
void TObjGreenGrassGroup::draw()
{
	if (objDebugEnabled(this) != 0) {
		return;
	}
	if ((mGrassFlags & GRASS_CUT) != 0) {
		if (isAllCut() == 0) {
			mGrassFlags &= ~GRASS_CUT;
		}
	} else {
		GrassContact* contact = findContact();
		if (contact != 0) {
			HitInfo hit;
			if (colliContactInGroup8((ColliContact*)contact) != 0 && colliContactHasBit8((ColliContact*)contact) != 0) {
				TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
				if (other != 0) {
					cutBlades(other);
					if (isAllCut() != 0) {
						mGrassFlags |= GRASS_CUT;
					}
				}
			} else {
				TObjAction* other = findObjById(colliContactOwner((ColliContact*)contact));
				if (other != 0) {
					ObjMsg msg;
					msg.mType = 0x3C;
					if ((other->sendToHandle(&msg) & 0x8000) != 0) {
						mHitFrom = *other->getPos();
						bendBlades(other);
					}
				}
			}
		}
	}
	updateBlades();
}

// 0x8017F9B0
void TObjGreenGrassGroup::vfn_10()
{
	if (objDebugEnabled(this) != 0) {
		mDrawTask->mFlags &= ~1;
		return;
	}

	Vec3f view;
	ColliShape* shape = colliGetShape(mShape, 0);
	if (cameraIsVisible(2000.0f, shape->mSizeX, &mPos, &view) != 0) {
		if (hasCameraMoved() != 0) {
			sortBlades();
		}
		mDrawTask->mFlags &= ~1;

		if ((mGrassFlags & GRASS_NEAR) != 0) {
			Vec3f top;
			top.x = 0.0f;
			top.y = 25.31f;
			top.z = 0.0f;
			f32 reach = 25.0f + distXZ(&mCameraPos, &top);

			GrassLine line;
			line.mStart = mCameraPos;
			line.mDir.x = top.x - line.mStart.x;
			line.mDir.y = top.y - line.mStart.y;
			line.mDir.z = top.z - line.mStart.z;
			Vec3f onLine;
			njDistanceP2L((const Vec3f*)&mPos, (const NJS_LINE*)&line, &onLine);
			if (distXZ(&mCameraPos, &onLine) <= reach) {
				mGrassFlags |= GRASS_SPARSE;
				moveDrawTask(mDrawTask, 0x22);
			} else {
				mGrassFlags &= ~GRASS_SPARSE;
				moveDrawTask(mDrawTask, 0x21);
			}
		} else {
			mGrassFlags &= ~GRASS_SPARSE;
			moveDrawTask(mDrawTask, 0x21);
		}
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8017FB50
void TObjGreenGrassGroup::render()
{
	njSetTexture(getGreenObjTextures()[0]);
	njSetTextureNum(4);
	drawBlades();
}

// 0x8017FB94
void TObjGreenGrassGroup::readCamera()
{
	camGetPos(camGetDrawIndex(), &mCameraPos);
}

// 0x8017FBC8
int TObjGreenGrassGroup::hasCameraMoved()
{
	Vec3f pos;

	camGetPos(camGetDrawIndex(), &pos);
	if (vecDistanceSq(&mCameraPos, &pos) > 0.0f) {
		mCameraPos = pos;
		return 1;
	}
	return 0;
}

// 0x8017FC38
// An insertion sort of the blades by distance from the camera
void TObjGreenGrassGroup::sortBlades()
{
	f32 dist[20];
	int i;

	for (i = 0; i < 2; i++) {
		dist[i] = distXZ(&mCameraPos, &mBlades[i]->mPos);
	}
	if (dist[0] > dist[1]) {
		mOrder[0] = 0;
		mOrder[1] = 1;
	} else {
		mOrder[0] = 1;
		mOrder[1] = 0;
	}
	for (i = 2; i < 20; i++) {
		dist[i]  = distXZ(&mCameraPos, &mBlades[i]->mPos);
		mOrder[i] = i;
		for (int j = 0; j < i; j++) {
			if (dist[mOrder[j]] >= dist[i]) {
				memmove(&mOrder[j] + 1, &mOrder[j], (i - j) * 4);
				mOrder[j] = i;
				break;
			}
		}
	}
}
