#include "object/orange/TObjOrangeClockTower.h"
#include "ui/TextVec.h"
#include "camera/TCamera.h"
#include "engine/DrawTask.h"
#include "camera/EventCamArc.h"
#include "object/ObjScript.h"
#include "object/ObjCommon.h"
#include "object/ObjArchive.h"
#include "object/ObjModel.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "stage/TStageOrange.h"
#include "core/MathUtil.h"
#include "object/TObjLauncher.h"
#include "engine/DrawState.h"
#include "camera/TCameraCtrl.h"
#include "Ninja/ninja.h"
#include "ui/AeArc.h"
#include "stage/TGameState.h"
#include "stage/SetSlot.h"
#include "object/TObjKey00.h"
#include "sound/SoundCtrl.h"
#include "core/VecMath.h"

static TextVec clockFaceFxOffsets[4] = {
	textVec(0.0f, 1600.0f, 800.0f),
	textVec(300.0f, 1400.0f, 600.0f),
	textVec(0.0f, 1200.0f, 800.0f),
	textVec(-300.0f, 1400.0f, 600.0f),
};
struct FxVec : public Vec3f {
	inline FxVec(f32 nx, f32 ny, f32 nz) { set(nx, ny, nz); }
};
static FxVec clockFaceFxScale(-870.0f, 1404.0f, -1085.0f);
struct SetLink;

struct DrawStateSlot;

extern "C" {

// 0x802C4338
const char* gTObjOrangeClockTowerName = "TObjOrangeClockTower";
}

namespace ClockTower {

// 0x802C433C
const char* partsName = "ClockTower_TParts";
// 0x802C4340
const char* lampName = "ClockTower_TLamp";

// The colours the lamps cycle through
static const GXColor sLampColors[8] = {
	{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0x00, 0x00, 0xFF }, { 0xFF, 0x80, 0x40, 0xFF }, { 0xFF, 0xFF, 0x00, 0xFF },
	{ 0x00, 0xFF, 0x00, 0xFF }, { 0x00, 0x80, 0xFF, 0xFF }, { 0x7F, 0x00, 0xFF, 0xFF }, { 0xFF, 0x00, 0xFF, 0xFF },
};

} // namespace ClockTower

typedef char TObjOrangeClockTower_size_check[(sizeof(TObjOrangeClockTower) == 0x10C) ? 1 : -1];

// 0x8012A290
extern "C" TObject* createTObjOrangeClockTower(TObject* parent, SetDesc* desc)
{
	return new TObjOrangeClockTower(parent, desc);
}

// Where the clock sequence puts its effect for each face turned with the tower and the effect's scale
// Set by the static initialiser at 0x8012BEFC

static inline void** geObjMotions()
{
	return (void**)gGeObjCommon->mSet.mMotions;
}

static inline ClockTower::TParts* makeFace(TObjOrangeClockTower* tower, int model, int tex, int motion, int sorted, f32 speed)
{
	return new ClockTower::TParts(tower, getGeObjModels()[model], getGeObjTextures()[tex], (NJS_MOTION*)geObjMotions()[motion], sorted,
	                              speed);
}

// 0x8012A2E4
TObjOrangeClockTower::TObjOrangeClockTower(TObject* parent, SetDesc* desc)
    : TObjAction(parent)
{
	_00       = (u32)gTObjOrangeClockTowerName;
	mSize     = 0x10C;
	mRadius   = 1000.0f;
	mParts[0] = makeFace(this, 3, 4, 0, 0, 0.03f);
	mParts[1] = makeFace(this, 4, 5, 1, 1, 0.003f);
	mParts[2] = makeFace(this, 5, 6, 2, 0, 0.03f);
	mParts[3] = makeFace(this, 6, 7, 3, 0, 0.03f);
	mParts[4] = makeFace(this, 6, 7, 3, 0, 0.03f);
	mLamp     = new ClockTower::TLamp(this);
	mLamp->clearAll();
	setObjId(this, 0xFFFF);
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x21, 0);
	initFromDesc(desc);
	mLink.get()->attach(desc);
	setup();
}

// 0x8012A560
TObjOrangeClockTower::~TObjOrangeClockTower()
{
	notifySetLinkDeath((SetLink*)&mLink, (void*)this);
	removeDrawTask(mDrawTask);
	releaseObjId(this);
}

// 0x8012A5F4
void TObjOrangeClockTower::initFromDesc(SetDesc* desc)
{
	TObjAction::initFromDesc(desc);
	mSwitch    = (s16)desc->getInt(0);
	mSwitchEnd = (s16)(mSwitch + 3);
	desc->getPos(&mPos);
	desc->getRot(&mRot);
	mtxSetTransV(mMatrix, &mPos);
	mtxRotateY(mMatrix, mMatrix, mRot.y);
	Vec3f lift;
	lift.x = 0.0f;
	lift.y = 500.0f;
	lift.z = 0.0f;
	vecAdd(&mTop, &mPos, &lift);
	mParts[0]->place(&mPos, mRot.y);
	mParts[1]->place(&mPos, mRot.y);
	mParts[2]->place(&mPos, mRot.y);
	mParts[3]->place(&mPos, mRot.y);
	mParts[4]->place(&mPos, mRot.y + 0x8000);
	mLamp->place(&mPos, mRot.y);
	for (u16 i = 0; i < 4; i++) {
		mIds[i]   = desc->getInt(i);
		mRates[i] = desc->getFloat(i);
	}
}

// 0x8012A828
void TObjOrangeClockTower::debugPrint(void*)
{
	njPrintC(0xA0005, ":-1004.66");
	njPrintC(0xA0006, ":210.88");
	njPrintC(0xA0007, ":-1409.97");
	njPrintC(0xA000A, ":0");
	njPrint(0xA000B, ":%x", 0xF7B);
	njPrintC(0xA000C, ":0");
	njPrint(0xA0013, ":SW ID(%d-%d)", mSwitch, mSwitchEnd);
	njPrintC(0xA0014, "");
	njPrintC(0xA0015, "");
	njPrintC(0xA0016, "");
	njPrintC(0xA0017, "");
	njPrintC(0xA0018, "");
	njPrintC(0xA0019, "");
	njPrintC(0xA001A, "");
}

// 0x8012A94C
int TObjOrangeClockTower::sendToHandle(ObjMsg* msg)
{
	if (msg->mType == 199) {
		mSlot = ((EventCamMsg*)msg)->mArg;
		if (mSlot < 0 || mSlot > 3) {
			return 0;
		}
		mClock.set(msg);
		return 1;
	}
	return 0;
}

// 0x8012A9AC
void TObjOrangeClockTower::setup()
{
	mFx    = 0;
	mChime = 0;
	mTimer = 0.0f;
	setLinkFree((SetLink*)&mLink);
	if (switchSet(0)) {
		mLamp->light(0);
	}
	if (switchSet(1)) {
		mLamp->light(2);
	}
	if (switchSet(2)) {
		mLamp->light(4);
	}
	if (switchSet(3)) {
		mLamp->light(6);
	}
	if (switchesSet() != 0) {
		mLamp->lightAll();
		mLamp->start();
		mParts[0]->start();
		mParts[1]->start();
		mParts[2]->start();
		mParts[3]->start();
		mParts[4]->start();
		setAct(3);
	} else {
		setAct(0);
	}
}

// 0x8012AAD4
int TObjOrangeClockTower::switchSet(int index)
{
	return keyBitIsSet((u16)(mSwitch + index));
}

// 0x8012AB00
int TObjOrangeClockTower::switchesSet()
{
	for (u16 id = (u16)mSwitch; id <= (u16)mSwitchEnd; id++) {
		if (keyBitIsSet(id) == 0) {
			return 0;
		}
	}
	return 1;
}

// 0x8012AB60
int TObjOrangeClockTower::faceReady(u16 index)
{
	mClock.isActive();
	if ((int)index == mSlot) {
		return 1;
	}
	return keyBitIsSet((u16)(mSwitch + index)) == 1;
}

// 0x8012ABC8
int TObjOrangeClockTower::facesReady()
{
	mClock.isActive();
	for (u16 i = 0; i < 4; i++) {
		if (faceReady(i) == 0) {
			return 0;
		}
	}
	return 1;
}

// 0x8012AC30
void TObjOrangeClockTower::draw()
{
	if (objDebugEnabled(this) != 0) {
		for (TObject* child = mChild; child != 0; child = child->mNext) {
			child->mFlags |= 4;
		}
		return;
	}
	if (!isInRange(this, &mPos, 4000.0f) && mClock.isActive() == 0) {
		for (TObject* child = mChild; child != 0; child = child->mNext) {
			child->mFlags |= 8;
		}
		return;
	}
	for (TObject* child = mChild; child != 0; child = child->mNext) {
		child->mFlags &= ~8;
	}
	switch (mAct) {
	case 0:
		stateWait();
		break;
	case 1:
		stateFace();
		break;
	case 2:
		stateChime();
		break;
	case 3:
		stateDone();
		break;
	}
}

// 0x8012AD4C
void TObjOrangeClockTower::setAct(int act)
{
	mAct = act;
}

// 0x8012AD54
void TObjOrangeClockTower::stateWait()
{
	if (mClock.isActive() == 0) {
		return;
	}
	mHand     = -1;
	int ready = 1;
	for (u16 i = 0; i < 4; i++) {
		if (faceReady(i) == 0) {
			ready = 0;
			continue;
		}
		if (mLamp->isLit((u16)(i << 1)) != 0) {
			continue;
		}
		Vec3f at  = mPos;
		Vec3f off = clockFaceFxOffsets[i];
		at.y      = 0.0f;
		vecRotateY(&off, &off, mRot.y);
		vecAdd(&at, &at, &off);
		mFx = createTCamFix(0, 3, &clockFaceFxScale, &at);
		((TCamera*)mFx)->setCut();
		mHand  = (s16)(i << 1);
		mTimer = 0.0f;
		_C0 = _BC = _B8 = 0.0f;
		setAct(1);
		return;
	}
	if (ready) {
		mChime = eventCamArcPlay(1.0f, 0.0f, 0x100);
		if (mChime != 0) {
			mChime->clearAutoDelete();
			((TCamera*)mChime)->setCut();
			setAct(2);
		}
	}
}

// 0x8012AEDC
void TObjOrangeClockTower::stateFace()
{
	s16 hand  = mHand;
	int frame = (int)(0.5f + mTimer);
	if (hand >= 0) {
		switch (frame) {
		case 0:
			if (facesReady() != 0) {
				sePlayFor(0x2B4, 0x4B0);
			}
			break;
		case 0x3C:
			mLamp->light(hand);
			if (facesReady() == 0) {
				sePlay(0x248);
			}
			break;
		case 0x78:
			if (facesReady() != 0) {
				mHand  = -1;
				mTimer = 0.0f;
				break;
			}
			if (mFx != 0) {
				((TObject*)mFx)->mFlags |= 1;
				mFx = 0;
			}
			mClock.release();
			s16 msg = 0x8C;
			sendGameMsg((ObjMsg*)&msg);
			setAct(0);
			return;
		}
	} else {
		switch (frame) {
		case 0x1E:
			mLamp->light(1);
			break;
		case 0x3C:
			mLamp->light(3);
			break;
		case 0x5A:
			mLamp->light(5);
			break;
		case 0x78:
			mLamp->light(7);
			break;
		case 0x96:
			mLamp->light(8);
			break;
		case 0xD2:
			mLamp->mSpeed = 0.099999994f;
			mLamp->start();
			mParts[0]->start();
			mParts[1]->start();
			mParts[2]->start();
			mParts[3]->start();
			mParts[4]->start();
			if (mFx != 0) {
				((TObject*)mFx)->mFlags |= 1;
				mFx = 0;
			}
			setAct(0);
			return;
		}
	}
	mTimer += 1.0f;
}

// 0x8012B10C
void TObjOrangeClockTower::stateChime()
{
	if (mChime->isDone() != 0) {
		mChime->mFlags |= 1;
		mChime = 0;
		mClock.release();
		mLamp->mSpeed = 0.005f;
		s16 msg       = 0x8C;
		sendGameMsg((ObjMsg*)&msg);
		setAct(3);
	}
	mTimer += 1.0f;
}

// 0x8012B19C
void TObjOrangeClockTower::stateDone()
{
}

// 0x8012B1A0
void TObjOrangeClockTower::vfn_10()
{
	if (mirrorIsReflecting() != 0) {
		mFlags |= 0x10;
		mDrawTask->mFlags |= 1;
	} else if (cameraIsVisible(6000.0f, mRadius, &mTop, 0) != 0) {
		mFlags &= ~0x10;
		mDrawTask->mFlags &= 0xFFFE;
	} else {
		mFlags |= 0x10;
		mDrawTask->mFlags |= 1;
	}
}

// 0x8012B24C
void TObjOrangeClockTower::render()
{
}

namespace ClockTower {

// 0x8012B250
TParts::TParts(TObjAction* parent, NJS_OBJECT* model, NJS_TEXLIST* texture, NJS_MOTION* motion, int sorted, f32 speed)
    : TObject(parent)
{
	mModel   = model;
	mTexture = texture;
	mMotion  = motion;
	mFrame   = 0.0f;
	mSpeed   = speed;
	mBits    = 0;
	_00      = (u32)partsName;
	mSize    = 0x70;
	place(parent->getPos(), (f32)parent->getRotY());
	mSorted  = sorted;
	int list = 0x21;
	if (mSorted != 0) {
		list = 0x22;
	}
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, list, 0);
}

// 0x8012B36C
TParts::~TParts()
{
	removeDrawTask(mDrawTask);
}

// 0x8012B3D4
void TParts::place(const Vec3f* pos, f32 rotY)
{
	mtxSetTransV(mMatrix, pos);
	mtxRotateY(mMatrix, mMatrix, (int)rotY);
}

// 0x8012B428
void TParts::start()
{
	mBits |= 1;
}

// 0x8012B438
void TParts::draw()
{
	if (mBits & 1) {
		mFrame += mSpeed;
		if (mFrame > (f32)mMotion->nbFrame) {
			mFrame -= (f32)mMotion->nbFrame;
		}
	}
}

// 0x8012B4A4
void TParts::vfn_10()
{
	if ((mParent->mFlags & 0x10) == 0) {
		if (mSorted != 0) {
			Vec3f pos;
			mtxGetPos(mMatrix, &pos);
			mtxMultVec(gjGetMatrix(), &pos, &pos);
			mDrawTask->mSortKey = drawTaskDepthKey(pos.z);
		}
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8012B538
void TParts::render()
{
	const DrawState* state = getDefaultDrawState();
	beginDrawState(getDrawStateSlot(), state);
	njSetTexture(mTexture);
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	gjDrawMotion(mModel, mMotion, mFrame);
	njPopMatrixEx();
	endDrawState(getDrawStateSlot());
}

// 0x802C4344
// the colour lit lamps draw with
static GXColor sLampColor = { 0xFF, 0xFF, 0xFF, 0xFF };

// 0x8012B5A8
void TLamp::setColor(GXColor* color)
{
	sLampColor.r = color->r;
	sLampColor.g = color->g;
	sLampColor.b = color->b;
	sLampColor.a = color->a;
}

// 0x8012B5D0
GXColor* TLamp::getColor()
{
	return &sLampColor;
}

// 0x802CA130
// Not const though it sits in .sdata2 the ctor reads it after the vtable store
__declspec(section ".sdata2") static GXColor sLampWhite = { 0xFF, 0xFF, 0xFF, 0xFF };

// 0x8012B5D8
TLamp::TLamp(TObjAction* parent)
    : TObject(parent)
{
	LampLight* light;
	NJS_OBJECT* node;
	NJS_TEXLIST* texture;
	u16 i;
	LampLight* lt2;
	NJS_OBJECT* glow;
	NJS_TEXLIST* tex2;
	u16 j;
	*(u32*)&mColor = *(const u32*)&sLampWhite;
	mPhase         = 0.0f;
	mSpeed         = 0.005f;
	mBits          = 0;
	_00            = (u32)lampName;
	mSize          = 0x6C;
	mLights        = new LampLight[LAMP_COUNT];
	node           = getGeObjModels()[7]->child;
	texture        = getGeObjTextures()[8];
	for (i = 0; node != 0; node = node->sibling) {
		light                  = &mLights[i++];
		light->mObject         = *node;
		light->mObject.sibling = 0;
		light->mTexture        = texture;
		light->mBits           = 0;
	}
	mGlows = new LampLight[LAMP_COUNT];
	glow   = getGeObjModels()[8]->child;
	tex2   = getGeObjTextures()[9];
	for (j = 0; glow != 0; glow = glow->sibling) {
		lt2                  = &mGlows[j++];
		lt2->mObject         = *glow;
		lt2->mObject.sibling = 0;
		lt2->mTexture        = tex2;
		lt2->mBits           = 0;
	}
	place(parent->getPos(), (f32)parent->getRotY());
	mDrawTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	setColor(&mColor);
}

// 0x8012B858
TLamp::~TLamp()
{
	removeDrawTask(mDrawTask);
	delete[] mGlows;
	delete[] mLights;
}

// 0x8012B8D0
void TLamp::place(const Vec3f* pos, f32 rotY)
{
	mtxSetTransV(mMatrix, pos);
	mtxRotateY(mMatrix, mMatrix, (int)rotY);
}

// 0x8012B924
int TLamp::isLit(u16 index)
{
	if (index < LAMP_COUNT) {
		return mLights[index].mBits & 1;
	}
	return 0;
}

// 0x8012B950
void TLamp::light(u16 index)
{
	if (index < LAMP_COUNT) {
		mLights[index].mBits |= 1;
		mGlows[index].mBits |= 1;
	}
}

// 0x8012B98C
void TLamp::lightAll()
{
	for (u16 i = 0; i < LAMP_COUNT; i++) {
		mLights[i].mBits |= 1;
		mGlows[i].mBits |= 1;
	}
}

// 0x8012B9D0
void TLamp::clearAll()
{
	for (u16 i = 0; i < LAMP_COUNT; i++) {
		mLights[i].mBits &= ~1;
		mGlows[i].mBits &= ~1;
	}
}

// 0x8012BA14
void TLamp::start()
{
	mBits |= 1;
}

// 0x8012BA24
void TLamp::cycle()
{
	mPhase += mSpeed;
	if ((int)mPhase > 7) {
		mPhase -= 8.0f;
	}
	int index = (int)mPhase;
	int next  = (index + 1) % 8;
	f32 t     = mPhase - (f32)index;
	mColor.r  = (u8)lerpF(sLampColors[index].r, sLampColors[next].r, t);
	mColor.g  = (u8)lerpF(sLampColors[index].g, sLampColors[next].g, t);
	mColor.b  = (u8)lerpF(sLampColors[index].b, sLampColors[next].b, t);
	mColor.a  = 0xFF;
	setColor(&mColor);
}

// 0x8012BBE4
void TLamp::draw()
{
	if (mBits & 1) {
		cycle();
	}
}

// 0x8012BC10
void TLamp::vfn_10()
{
	if ((mParent->mFlags & 0x10) == 0) {
		Vec3f pos;
		mtxGetPos(mMatrix, &pos);
		mtxMultVec(gjGetMatrix(), &pos, &pos);
		mDrawTask->mSortKey = drawTaskDepthKey(pos.z);
		mDrawTask->mFlags &= ~1;
	} else {
		mDrawTask->mFlags |= 1;
	}
}

// 0x8012BC98
void TLamp::setupTev(int stage, int coord, int map, int channel)
{
	GXSetTevKColor(GX_KCOLOR0, *(GXColor*)&*getColor());
	GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0);
	GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
	GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
	GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GXSetTevOrder(stage, coord, map, channel);
}

// 0x8012BD6C
void TLamp::render()
{
	void* funcs[8];
	ObjDrawState state;
	u16 i;
	u16 j;
	state.mFlags = DRAWSTATE_BLEND;
	state.mBlend = 1;
	njPushMatrixEx();
	njMultiMatrix(0, mMatrix);
	njSetTexture(mLights[0].mTexture);
	for (i = 0; i < LAMP_COUNT; i++) {
		if (mLights[i].mBits & 1) {
			beginDrawState(getDrawStateSlot(), &state);
			funcs[3] = (void*)setupTev;
			gjSetNumPostTevStages(1);
			gjChangeDrawObjIF(funcs, 0x20);
		} else {
			beginDrawState(getDrawStateSlot(), getDefaultDrawState());
			funcs[3] = 0;
			gjSetNumPostTevStages(0);
			gjChangeDrawObjIF(funcs, 0x20);
		}
		gjDrawObject(&mLights[i].mObject);
		endDrawState(getDrawStateSlot());
	}
	beginDrawState(getDrawStateSlot(), &state);
	njSetTexture(mGlows[0].mTexture);
	funcs[3] = (void*)setupTev;
	gjSetNumPostTevStages(1);
	gjChangeDrawObjIF(funcs, 0x20);
	for (j = 0; j < LAMP_COUNT; j++) {
		if (mGlows[j].mBits & 1) {
			gjDrawObject(&mGlows[j].mObject);
		}
	}
	endDrawState(getDrawStateSlot());
	funcs[3] = 0;
	gjChangeDrawObjIF(funcs, 0x20);
	njPopMatrixEx();
}

} // namespace ClockTower

static inline Vec3f makeVec3f(f32 x, f32 y, f32 z)
{
	Vec3f v;
	v.set(x, y, z);
	return v;
}

// Unused and stripped at link
template <typename T>
void clockTowerKeep(T* obj)
{
	obj->mFlags |= 0;
}

extern "C" void* clockTowerKeepRef()
{
	void (*keep)(TObject*) = &clockTowerKeep<TObject>;
	return (void*)keep;
}

// 0x80282860
// and 0x80282890 set by the static initialiser at 0x8012BEFC
