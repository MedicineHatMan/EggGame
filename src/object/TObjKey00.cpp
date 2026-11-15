#include "stage/SetManager.h"
#include "stage/SetActor.h"
#include "stage/EventScript.h"
#include "Ninja/ninja.h"
#include "object/ObjScript.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "object/ObjAction.h"
#include "engine/Colli.h"
#include "io/NinjaFile.h"
#include "camera/TCamera.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "core/PlacementNew.h"
#include "enemy/boss/TEneYellowBossTama_Blue.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "sound/Sound3DPlay.h"
#include "camera/TCameraCtrl.h"
#include "ui/AeArc.h"
#include "egg/EggCreature.h"
#include "engine/GroundQuery.h"
#include "engine/MeshPool.h"
#include "engine/WorldMesh.h"
#include "object/TObjKey00.h"
struct CharaColli;
struct MeshColli;
struct MeshData;
struct MeshObject;
struct ColliContact;
struct DrawState;
struct DrawStateSlot;
struct Fx;
struct GjModel;

const f32 gKey00SetRange = 800.0f;

// 0x28 byte object that waits 60 frames and then sets the bit

extern "C" {
// 0x802C3BF8
ObjClassName gTObjKey00Name = { "TObjKey00", 0 };
}

class TObjKey00;

#define sKeys sKeyListObj

// 0x80024164
// Bit 0 is the top bit of the first byte
extern "C" void keyBitPlace(int bit, int* byteIndex, u8* mask)
{
	*byteIndex = bit / 8;
	*mask      = 1 << (7 - bit % 8);
}

// 0x8002412C
extern "C" void keySetWordBit(u32* word, int bit, int on)
{
	if (on != 0) {
		*word |= 1u << bit;
	} else {
		*word &= ~(1u << bit);
	}
}

// 0x80024198
extern "C" void initKeyBits()
{
	sKeyFlags = new u8[0x100];
	memset(sKeyFlags, 0, 0x100);
	sKeyReady = 1;
}

// 0x800241D4
extern "C" void clearKeyBits(int first, int last)
{
	int end   = (last + 1) / 8;
	int start = first / 8;
	while (start < end) {
		sKeyFlags[start] = 0;
		start++;
	}
}

// 0x80024294
extern "C" void markKeysReady()
{
	sKeyReady = 1;
}

// 0x80024268
extern "C" void clearKeyGroupMid()
{
	clearKeyBits(0x200, 0x3FF);
	markKeysReady();
}

// 0x8002423C
extern "C" void clearKeyGroupLow()
{
	clearKeyBits(0, 0x1FF);
	clearKeyGroupMid();
}

// 0x80024210
extern "C" void clearKeyGroupHigh()
{
	clearKeyBits(0x400, 0x7FF);
	clearKeyGroupLow();
}

// 0x800242A0
extern "C" int keysAreReady()
{
	return sKeyReady;
}

// 0x800242A8
extern "C" u8* keyByte(int bit, u8* mask)
{
	int index;
	if (bit < 0 || bit >= 0x800) {
		return 0;
	}
	keyBitPlace(bit, &index, mask);
	return sKeyFlags + index;
}

// 0x800242F4
extern "C" void setKeyBit(u16 bit)
{
	u8 mask;
	u8* byte;
	if (keysAreReady() == 0) {
		return;
	}
	byte = keyByte(bit, &mask);
	if (byte == 0) {
		return;
	}
	if ((*byte & mask) == 0) {
		eventScriptTrigger(bit);
	}
	*byte |= mask;
}

// 0x80024368
extern "C" void clearKeyBit(u16 bit)
{
	u8 mask;
	u8* byte;
	if (keysAreReady() == 0) {
		return;
	}
	byte = keyByte(bit, &mask);
	if (byte != 0) {
		*byte = (u8)(*byte & ~mask);
	}
}

// 0x800243C0
// 1 when the bit is set
extern "C" int keyBitIsSet(u16 bit)
{
	u8 mask;
	u8* byte = keyByte(bit, &mask);
	if (byte == 0) {
		return 0;
	}
	return (*byte & mask) == mask;
}

// 0x80024410
extern "C" void setKeySwitch(u16 bit, int on)
{
	if (keysAreReady() == 0) {
		return;
	}
	if (on != 0) {
		setKeyBit(bit);
	} else {
		clearKeyBit(bit);
	}
}

// 0x80024464
// Packs count bits starting at first into dest
extern "C" void saveKeyBits(u8* dest, u16 first, u32 count)
{
	u8* flags = sKeyFlags;
	u8* p     = dest;
	int index = first;
	int rest;
	int i;
	int n;
	int b;
	u8 packed;
	u8 mask;
	int byte;
	n = (int)count / 8;
	for (i = 0; i < n; i++) {
		packed = 0;
		for (b = 0; b < 8; b++) {
			keyBitPlace(index, &byte, &mask);
			if (flags[byte] & mask) {
				packed = (u8)(packed | (1 << (7 - b)));
			}
			index++;
		}
		*p++ = packed;
	}
	rest = (int)count % 8;
	if (rest != 0) {
		packed = 0;
		for (b = 0; b < rest; b++) {
			keyBitPlace(index, &byte, &mask);
			if (flags[byte] & mask) {
				packed = (u8)(packed | (1 << (7 - b)));
			}
			index++;
		}
		*p = packed;
	}
}

// 0x8002457C
static inline void keyPut(int byte, u8* flags, u8* mask, int on)
{
	if (on) {
		flags[byte] |= *mask;
	} else {
		flags[byte] &= ~*mask;
	}
}

extern "C" void loadKeyBits(u8* src, u16 first, u32 count)
{
	int n;
	u8* p = src;
	int i;
	int b;
	int index = first;
	u8 packed;
	int byte;
	u8 mask;
	n = (int)count / 8;
	for (i = 0; i < n; i++) {
		packed = *p++;
		for (b = 0; b < 8; b++) {
			keyBitPlace(index, &byte, &mask);
			keyPut(byte, sKeyFlags, &mask, packed & (1 << (7 - b)));
			index++;
		}
	}
	n = (int)count % 8;
	if (n != 0) {
		packed = *p;
		for (b = 0; b < n; b++, index++) {
			keyBitPlace(index, &byte, &mask);
			keyPut(byte, sKeyFlags, &mask, packed & (1 << (7 - b)));
		}
	}
}

// 0x80026008
KeyList::KeyList()
{
	mHead  = 0;
	mTail  = 0;
	mCount = 0;
}

// 0x8002601C
KeyList::~KeyList()
{
	clear();
}

// 0x80026064
void KeyList::add(KeyNode* node)
{
	if (mHead == 0) {
		mTail = node;
		mHead = node;
	} else {
		node->mPrev  = mTail;
		mTail->mNext = node;
		mTail        = node;
	}
	node->mList = this;
	mCount++;
}

// 0x800260A4
void KeyList::remove(KeyNode* node)
{
	if (node->mPrev != 0) {
		node->mPrev->mNext = node->mNext;
	} else {
		mHead = node->mNext;
	}
	if (node->mNext != 0) {
		node->mNext->mPrev = node->mPrev;
	} else {
		mTail = node->mPrev;
	}
	node->mNext = 0;
	node->mPrev = 0;
	node->mList = 0;
	mCount--;
}

// 0x80026104
void KeyList::clear()
{
	KeyNode* node = mHead;
	while (node != 0) {
		KeyNode* next = node->mNext;
		node->mList   = 0;
		node->mNext   = 0;
		node->mPrev   = 0;
		node          = next;
	}
}

// 0x800246B8
extern "C" void initTObjKey00()
{
	sKeys = new KeyList;
}

// 0x800246F0
extern "C" void termTObjKey00()
{
	delete sKeys;
}

/**
 * Waits 60 frames, sets its switch bit, then asks to be deleted. The name is ours.
 * @size 0x28
 */
class TObjKeyTimer : public TObject {
public:
	// Inlined at each take 0x3C is 60 frames
	TObjKeyTimer(TObject* parent, u16 bit)
	    : TObject(parent)
	{
		mBit   = bit;
		mTimer = 0x3C;
	}
	virtual ~TObjKeyTimer();
	virtual void draw();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	u16 mBit;   // _20
	u16 _22;    // _22
	int mTimer; // _24 counts down from 60
};

typedef char TObjKeyTimer_size_check[(sizeof(TObjKeyTimer) == 0x28) ? 1 : -1];

typedef char TObjKey00_size_check[(sizeof(TObjKey00) == 0x140) ? 1 : -1];

static inline int clampRange(int n, int lo, int hi)
{
	if (n < lo) {
		return lo;
	}
	if (n > hi) {
		return hi;
	}
	return n;
}

// 0x80024718
extern "C" TObject* createTObjKey00(TObject* parent, SetDesc* desc)
{
	return new TObjKey00(parent, desc);
}

// 0x8002476C
void TObjKey00::initFromDesc(SetDesc* desc)
{
	ColliShape shape;
	int kind;

	TSetActor::initFromDesc(desc);
	mModel = -1;
	mKind  = clampRange(desc->getInt(0), 0, 5);
	mUse   = clampRange(desc->getInt(1), 0, 3);
	colliShapeInit(&shape);
	kind = mKind;
	if (kind == 0) {
		shape.mType  = COLLISHAPE_SPHERE;
		shape.mSizeX = 10.0f + desc->getFloat(0);
		mWho         = desc->getInt(2) % 3;
	} else if (kind == 1) {
		shape.mType  = COLLISHAPE_BOX;
		shape.mSizeX = 10.0f + desc->getFloat(0);
		shape.mSizeY = 10.0f + desc->getFloat(1);
		shape.mSizeZ = 10.0f + desc->getFloat(2);
		mWho         = desc->getInt(2) % 3;
	} else if (kind == 2) {
		shape.mType  = COLLISHAPE_CYLINDER;
		shape.mSizeX = 10.0f + desc->getFloat(0);
		shape.mSizeY = 10.0f + desc->getFloat(1);
		mWho         = desc->getInt(2) % 3;
	} else if (kind == 3) {
		shape.mType  = COLLISHAPE_SPHERE;
		shape.mSizeX = 10.0f + desc->getFloat(0);
		mRadius      = shape.mSizeX;
		mWho         = 0;
		mModel       = 7;
		mModel2      = 7;
	} else if (kind == 4) {
		shape.mType  = COLLISHAPE_SPHERE;
		shape.mSizeX = 10.0f + desc->getFloat(0);
		mRadius      = shape.mSizeX;
		mWho         = 1;
		mModel       = 8;
		mModel2      = 8;
	} else if (kind == 5) {
		int which = clampRange(desc->getInt(2), 0, 1);
		if (which == 0) {
			shape.mType  = COLLISHAPE_SPHERE;
			shape.mSizeX = 10.0f + desc->getFloat(0);
			mRadius      = shape.mSizeX;
		} else {
			shape.mType  = COLLISHAPE_CYLINDER;
			shape.mSizeX = 10.0f + desc->getFloat(0);
			mRadius      = shape.mSizeX;
			shape.mSizeY = 2.5f + desc->getFloat(1);
		}
		mWho    = 2;
		mModel  = 0;
		mModel2 = 0;
	}
	shape.mFlags |= 0x10;
	colliCopyShapes(mShape, &shape);
	colliPlaceShapesRotY(mShape, &mPos, mRot.y, &shape);
	if (mUse == 3) {
		mHold = (int)(60.0f * (1.0f + desc->getFloat(3)));
	}
	if (mModel == -1) {
		mPiece0   = 0;
		mPiece1   = 0;
		mPiece2   = 0;
		mPiece3   = 0;
		mMaterial = 0;
	} else {
		NJS_OBJECT* entry = getStgObjModels()[mModel];
		mPiece1           = entry->child;
		mPiece0           = mPiece1->sibling;
		mPiece2           = mPiece0->sibling;
		mPiece3           = mPiece2->sibling;
		mPiece1->evalflags &= ~1u;
		mMaterial = getStgObjTextures()[mModel2];
	}
	mSwitch = clampRange(desc->getInt(3), 0, 0x7FF);
}

// 0x80024C88
void TObjKey00::debugPrint(void*)
{
	switch (mKind) {
	case 0:
		njPrintC(0xA0013, ":MODEL NONE(SPHERE)");
		njPrintC(0xA0015, ":TARGET TYPE");
		njPrintC(0xA0017, "+10.0:RADIUS");
		break;
	case 1:
		njPrintC(0xA0013, ":MODEL NONE(CUBE)");
		njPrintC(0xA0015, ":TARGET TYPE");
		njPrintC(0xA0017, "+10.0:WIDTH");
		njPrintC(0xA0018, "+10.0:HEIGHT");
		njPrintC(0xA0019, "+10.0:DEPTH");
		break;
	case 2:
		njPrintC(0xA0013, ":MODEL NONE(CYLINDER)");
		njPrintC(0xA0015, ":TARGET TYPE");
		njPrintC(0xA0017, "+10.0:RADIUS");
		njPrintC(0xA0018, "+10.0:HEIGHT");
		break;
	case 3:
		njPrintC(0xA0013, ":MODEL NORMAL");
		break;
	case 4:
		njPrintC(0xA0013, ":MODEL PLAYER");
		break;
	case 5:
		njPrintC(0xA0013, ":MODEL EGG");
		njPrintC(0xA0015, ":COLLI TYPE");
		break;
	}

	switch (mUse) {
	case 0:
		njPrintC(0xA0014, ":TYPE_1TIME");
		break;
	case 1:
		njPrintC(0xA0014, ":TYPE_ON_OFF");
		break;
	case 2:
		njPrintC(0xA0014, ":TYPE_PUSH");
		break;
	case 3:
		njPrintC(0xA0014, ":TYPE_ALL_ON_TIMER");
		break;
	}

	njPrintC(0xA0016, ":KEY_ID");
	njPrintD(0x120016, mSwitch, 3);

	switch (mKind) {
	case 0:
		njPrintC(0xA0017, "+10.0:RADIUS");
		break;
	case 1:
	case 3:
	case 4:
		njPrintC(0xA0017, "+10.0:WIDTH");
		njPrintC(0xA0018, "+10.0:HEIGHT");
		njPrintC(0xA0019, "+10.0:DEPTH");
		break;
	case 2:
		njPrintC(0xA0017, "+10.0:RADIUS");
		njPrintC(0xA0018, "+10.0:HEIGHT");
		break;
	case 5:
		njPrintC(0xA0017, "+10.0:RADIUS");
		njPrintC(0xA0018, "+2.5:HEIGHT");
		break;
	}

	if (mUse == 3) {
		njPrintC(0xA001A, "+1.0:HOLD TIME(SEC)");
	}

	njPrintC(0x5000F, "(GREEN:1TIME RED:ON_OFF BLUE:PUSH)");
	njPrintC(0x30011, "TARGET TYPE:(0:ALL 1:PLAYER 2:BALL)");
}

// 0x80024F98
TObjKey00::TObjKey00(TObject* parent, SetDesc* desc)
    : TSetActor(parent, desc)
{
	_00    = (u32)gTObjKey00Name.mName;
	mSize  = 0x140;
	mShape = 0;
	mShape = colliCreate(1, 1);
	colliSetOwner(mShape, (u16)mObjId);
	colliSetMasks(mShape, 4, 255, 255, 0xF6);
	initFromDesc(desc);
	mDraw   = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	mActor  = 0;
	mEffect = 0;
	build();
}

// 0x80025088
TObjKey00::~TObjKey00()
{
	sKeys->remove(&mNode);
	if (mEffect != 0) {
		fxStop(mEffect);
	}
	if (mActor != 0) {
		meshColliFree(mActor);
	}
	colliRetire(mShape);
	mShape = 0;
	removeDrawTask(mDraw);
}

// 0x8002512C
void TObjKey00::build()
{
	ObjCommonData* pack;
	MeshData* model;

	mNode.mObj = this;
	sKeys->add(&mNode);
	if (mModel > -1) {
		mLight0.mKind  = 0x10;
		mLight0.mColor = 0xFF0000FF;
		mLight1.mKind  = 0x10;
		mLight1.mColor = 0x800000FF;
	}
	if (keyBitIsSet((u16)mSwitch) == 1) {
		mLink.get()->setData((void*)1);
		mOpen = 1;
	} else {
		mLink.get()->setData((void*)0);
		mOpen = 0;
	}
	mHoldFrame = 0;
	mScale     = 0.0f;
	mLatched   = 0;
	mSpin      = 0;
	mtxSetTransV(mMtx, &mPos);
	mtxRotateZYX((f32*)mMtx, (const f32*)mMtx, (const int*)&mRot);
	if (mModel != -1) {
		pack   = gStgObjCommon;
		model  = (MeshData*)pack->mSet.mMeshes[3];
		mActor = (MeshObject*)meshColliCreate();
		if (mRot.x != 0 || mRot.z != 0) {
			meshSetState100(mActor, 1);
		}
		meshColliSetData(mActor, model);
		meshColliSetMatrix((MeshColli*)mActor, (const f32*)mMtx);
		fitBounds();
	}
	if (mOpen == 0) {
		showEffect();
	}
}

// 0x80025280
// Kinds 3 and 4 use effect 0xE kind 5 uses 0xD
void TObjKey00::showEffect()
{
	f32 pitched[12];

	fxStop(mEffect);
	mtxRotateX(pitched, mMtx, -0x4000);
	switch (mKind) {
	case 3:
	case 4:
		mEffect = fxSpawnAt(1, 7, 0xE, pitched);
		break;
	case 5:
		mEffect = fxSpawnAt(1, 7, 0xD, pitched);
		break;
	default:
		mEffect = 0;
		break;
	}
}

// 0x8002531C
// The take plays 0xF for kind 5 and 0x10 for kinds 3 and 4 and drops the looping effect
void TObjKey00::playTake()
{
	f32 pitched[12];

	fxStop(mEffect);
	mEffect = 0;
	mtxRotateX(pitched, mMtx, -0x4000);
	switch (mKind) {
	case 3:
	case 4:
		fxSpawnAt(1, 7, 0x10, pitched);
		break;
	case 5:
		fxSpawnAt(1, 7, 0xF, pitched);
		break;
	}
}

// 0x800253AC
static inline Vec3f keyNeg(const Vec3f& v)
{
	Vec3f r;
	r.x = -v.x;
	r.y = -v.y;
	r.z = -v.z;
	return r;
}

void TObjKey00::fitBounds()
{
	Vec3f size;

	size.set(1.0f, 1.0f, 1.0f);
	gjScaleVector(mRadius, &size, &size);

	Vec3f high;
	high.x = size.x;
	high.y = size.y;
	high.z = size.z;
	Vec3f low;
	Vec3f t = keyNeg(size);
	low.x   = t.x;
	low.y   = t.y;
	low.z   = t.z;

	gjPushUnitMatrix();
	njTranslateEx(&mPos);
	njCalcPointEx(&high, &high);
	njCalcPointEx(&low, &low);
	njPopMatrixEx();

	f32 box[6];
	boxAroundLine(box, &low, &high);
	meshColliSetBox(mActor, (const f32*)box);
}

// 0x80025498
// 1 when every other key on this switch and use is already open
int TObjKey00::lastToOpen()
{
	KeyNode* node = sKeys->mHead;
	while (node != 0) {
		if (mSwitch == node->mObj->mSwitch && mUse == node->mObj->mUse && node->mObj->mOpen == 0) {
			return 0;
		} else {
			node = node->mNext;
		}
	}
	return 1;
}

// 0x800254F0
// 1 when a different key on this switch and use is open
int TObjKey00::siblingOpen()
{
	TObjKey00* other;
	KeyNode* node = sKeys->mHead;
	while (node != 0) {
		other = node->mObj;
		if (mSwitch == other->mSwitch && mUse == other->mUse && this != other && other->mOpen == 1) {
			return 1;
		}
		node = node->mNext;
	}
	return 0;
}

// 0x80025550
// mWho 0 accepts either body 1 the player 2 the egg
int TObjKey00::touched()
{
	int count = colliGetContactCount(mShape);
	int i;
	for (i = 0; i < count; i++) {
		ColliContact* hit = colliGetContact(mShape, i);
		ColliShape* cur   = colliContactShape(hit);
		if ((cur->mFlags & 0x10) != 0) {
			continue;
		}
		if (colliInGroup((CharaColli*)(hit->mOther), 1) != 0) {
			if (mWho == 0 || mWho == 1) {
				return 1;
			}
		} else if (colliInGroup((CharaColli*)(hit->mOther), 8) != 0) {
			if (mWho == 0 || mWho == 2) {
				return 1;
			}
		}
	}
	return 0;
}

inline void TObjKey00::spawnTimer()
{
	new TObjKeyTimer(this, mSwitch);
}

// 0x8002561C
void TObjKey00::useTouch()
{
	if (((u32)mLink.get()->getData() & 1) != 0) {
		return;
	}
	if (touched() == 0) {
		return;
	}
	playTake();
	mOpen = 1;
	if (mKind >= 3) {
		sePlay3D(0x4E, &mPos);
	}
	spawnTimer();
	mLink.get()->setData((void*)1);
}

// 0x800256D4
void TObjKey00::useToggle()
{
	if (touched() != 0) {
		if (mLatched == 0) {
			if (((u32)mLink.get()->getData() & 1) != 0) {
				mOpen = 0;
				clearKeyBit((u16)mSwitch);
				mLink.get()->setData((void*)0);
				showEffect();
				sePlay3D(0x50, &mPos);
			} else {
				playTake();
				mOpen = 1;
				if (mKind >= 3) {
					sePlay3D(0x4E, &mPos);
				}
				spawnTimer();
				mLink.get()->setData((void*)1);
			}
			mLatched = 1;
		}
	} else {
		mLatched = 0;
	}
}

// 0x800257E0
void TObjKey00::useHold()
{
	if (touched() != 0) {
		if (mLatched == 0) {
			mOpen    = 1;
			mLatched = 1;
			if (mKind >= 3) {
				sePlay3D(0x4E, &mPos);
			}
			spawnTimer();
			mLink.get()->setData((void*)1);
			if (mModel > -1) {
				playTake();
			}
		}
	} else if (mOpen == 1) {
		mOpen    = 0;
		mLatched = 0;
		clearKeyBit((u16)mSwitch);
		mLink.get()->setData((void*)0);
		showEffect();
	}
}

// 0x800258DC
void TObjKey00::useTimed()
{
	if (keyBitIsSet((u16)mSwitch) == 1) {
		return;
	}
	if (touched() != 0 && mOpen == 0) {
		if (mModel > -1) {
			playTake();
		}
		mHoldFrame = 0;
		mOpen      = 1;
		mLink.get()->setData((void*)1);
		if (mKind >= 3) {
			sePlay3D(0x4E, &mPos);
		}
		if (lastToOpen() != 0) {
			spawnTimer();
		}
	} else if (mOpen == 1) {
		int frame  = mHoldFrame;
		mHoldFrame = frame + 1;
		if (frame > mHold) {
			mHoldFrame = 0;
			mOpen      = 0;
			mLink.get()->setData((void*)0);
			showEffect();
			if (mKind >= 3) {
				sePlay3D(0x50, &mPos);
			}
		}
	}
}

// 0x80025A28
void TObjKey00::draw()
{
	u8 shade;

	if (debugEditActive() != 0) {
		return;
	}
	if (isInRange(this, &mPos, 850.0f) == 0) {
		if (mUse == 3) {
			if (keyBitIsSet((u16)mSwitch) == 1) {
				mFlags |= 1;
				return;
			}
			if (siblingOpen() == 0) {
				mFlags |= 1;
				return;
			}
		} else {
			mFlags |= 1;
			return;
		}
	}
	switch (mUse) {
	case 0:
		useTouch();
		break;
	case 1:
		useToggle();
		break;
	case 2:
		useHold();
		break;
	case 3:
		useTimed();
		break;
	}
	if ((u32)mLink.get()->getData() & 1) {
		mScale         = -1.0f;
		mLight0.mColor = 0x00FF00FF;
		mLight1.mColor = 0xA0A0A0FF;
	} else {
		mScale = 0.0f;
		mSpin += 0x800;
		mSpin &= 0xFFFF;
		f32 s          = 0.5f * (1.0f + njSin(mSpin));
		mLight0.mColor = 0xFF0000FF;
		shade          = 160.0f * s + 80.0f;
		mLight1.mColor = (shade << 24) | (shade << 16) | (shade << 8) | 0xFF;
	}
}

// 0x80025BD0
void TObjKey00::vfn_10()
{
	if (cameraIsVisible(800.0f, 10.0f, getPos(), 0) != 0) {
		mDraw->mFlags &= ~1u;
	} else {
		mDraw->mFlags |= 1;
	}
}

// 0x80025C44
void TObjKey00::render()
{
	DrawStateSlot* slot;

	if (objDebugEnabled(this) != 0) {
		GXColor color;
		if (keyBitIsSet((u16)mSwitch) == 1) {
			color = sKey00Color0;
		} else {
			switch (mUse) {
			case 0:
				color = sKey00Color2;
				break;
			case 1:
				color = sKey00Color1;
				break;
			case 2:
				color = sKey00Color3;
				break;
			case 3:
				color = sKey00Color4;
				break;
			}
		}
		njPushMatrixEx();
		njTranslateEx(&mPos);
		gjRotateY(mRot.y);
		switch (colliGetShapes(mShape)->mType) {
		case 0:
			debugSphere((const Vec3f*)&gVecZero, &color, colliGetShapes(mShape)->mSizeX);
			break;
		case 1: {
			Vec3f size;
			size.x = colliGetShapes(mShape)->mSizeX;
			size.y = colliGetShapes(mShape)->mSizeY;
			size.z = colliGetShapes(mShape)->mSizeZ;
			gjScaleVector(2.0f, &size, &size);
			debugCube((const Vec3f*)&gVecZero, (const Vec3f*)&size, &color);
			break;
		}
		case 2:
			debugCylinder((const Vec3f*)&gVecZero, &color, colliGetShapes(mShape)->mSizeX, colliGetShapes(mShape)->mSizeY);
			break;
		default:
			debugSphere((const Vec3f*)&gVecZero, &color, colliGetShapes(mShape)->mSizeX);
			break;
		}
		njPopMatrixEx();
	}
	switch (mKind) {
	case 0:
	case 1:
	case 2:
		return;
	default:
		slot = getDrawStateSlot();
		njPushMatrixEx();
		njMultiMatrix(0, mMtx);
		njSetTexture(mMaterial);
		beginDrawState(slot, getDefaultDrawState());
		njPushMatrixEx();
		gjTranslate(0.0f, mScale, 0.0f);
		gjDrawModel(mPiece1->model);
		njPopMatrixEx();
		gjDrawModel(mPiece0->model);
		endDrawState(slot);
		beginDrawState(slot, (const DrawState*)&mLight1);
		gjDrawModel(mPiece2->model);
		endDrawState(slot);
		beginDrawState(slot, (const DrawState*)&mLight0);
		gjDrawModel(mPiece3->model);
		endDrawState(slot);
		njPopMatrixEx();
		break;
	}
}

// 0x80025F5C
TObjKeyTimer::~TObjKeyTimer()
{
}

// 0x80025FB8
// The bit is set on the frame the timer reaches 0
void TObjKeyTimer::draw()
{
	u32 left = mTimer;
	mTimer   = left - 1;
	if (left == 0) {
		setKeyBit(mBit);
		mFlags |= 1;
	}
}

// 0x80026130
Vec3f* TSetActor::getPos()
{
	return &mPos;
}

// 0x80026138
void TSetActor::setPos(const Vec3f* pos)
{
	mPos.x = pos->x;
	mPos.y = pos->y;
	mPos.z = pos->z;
}

// 0x80026154
int TSetActor::getKind()
{
	return 3;
}

// 0x8002615C
int TSetActor::getRotY()
{
	return mRot.y;
}

// 0x80026164
SetLink* TSetActor::getLink()
{
	return mLink.get();
}
