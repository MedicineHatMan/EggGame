#include "stage/Stage.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "egg/ObjEgg.h"
#include "Ninja/ninja.h"
#include "object/ObjAction.h"
#include "egg/EggCreature.h"
#include "enemy/boss/TEneOrangeBossFloor.h"
#include "enemy/boss/EneBoss.h"
#include "engine/TFogCtrl.h"
#include "enemy/boss/TEneOrangeBossMap.h"
#include "ui/AeArc.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "core/TMainTask.h"
struct NJS_MOTION;
struct NJS_OBJECT;

extern "C" {
}

typedef char TEneOrangeBossFloor_size_check[(sizeof(TEneOrangeBossFloor) == 0xB0) ? 1 : -1];

extern "C" {
TEneOrangeBossFloor* sOrangeFloor; // 0x802C56A0

// 0x8013C86C
void orangeFloorClear()
{
	sOrangeFloor = 0;
}

// 0x8013C878
// Lets the floor go
void orangeFloorEnd()
{
	if (sOrangeFloor == 0) {
		return;
	}
	sOrangeFloor->mFlags |= 1;
	sOrangeFloor = 0;
}

// 0x8013C89C
// From the floor's dtor
void orangeFloorForget(void* floor)
{
	if (sOrangeFloor != floor) {
		return;
	}
	sOrangeFloor = 0;
}

// 0x8013C8B4
void orangeFloorStart()
{
	if (sOrangeFloor == 0) {
		sOrangeFloor = createTEneOrangeBossFloor(gTL_04);
	}
}

// 0x8013C8E8
void orangeFloorSetPhase(int phase)
{
	if (sOrangeFloor != 0) {
		sOrangeFloor->setPhase(phase);
	}
}

// 0x8013C91C
void orangeFloorPhase0()
{
	orangeFloorSetPhase(0);
}

// 0x8013C940
void orangeFloorPhase1()
{
	orangeFloorSetPhase(1);
}

// 0x8013C964
void orangeFloorPhase2()
{
	orangeFloorSetPhase(2);
}

// 0x8013C988
TEneOrangeBossFloor* createTEneOrangeBossFloor(TObject* parent)
{
	return new TEneOrangeBossFloor(parent);
}
}

// 0x802497F8
// The map model of each floor piece
extern const int gOrangeFloorModels[3] = { 1, 2, 4 };
const char* gTEneOrangeBossFloorName   = "TEneOrangeBossFloor";

// 0x8013C9D4
TEneOrangeBossFloor::TEneOrangeBossFloor(TObject* parent)
    : TObject(parent)
{
	int angle;
	int i;

	_00   = (u32)gTEneOrangeBossFloorName;
	mSize = 0xB0;
	for (i = 0; i < 8; i++) {
		f32 sn;
		f32 cs;
		angle = (i << 16) / 8;
		njSinCos(angle, &sn, &cs);
		mPoint[i].x = 220.0f * sn;
		mPoint[i].y = 50.0f;
		mPoint[i].z = 220.0f * cs;
		mAngle[i]   = angle;
	}
	mIndex  = 0;
	mFrame  = 0.0f;
	mDraw   = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x19, 0);
	mMotion = *orangeBossMapMotion(0);
}

// 0x8013CB0C
TEneOrangeBossFloor::~TEneOrangeBossFloor()
{
	orangeFloorForget(this);
	removeDrawTask(mDraw);
}

// 0x8013CB78
void TEneOrangeBossFloor::setPhase(int phase)
{
	mIndex = phase;
}

// 0x8013CB80
// Phase 1 loops the motion
void TEneOrangeBossFloor::draw()
{
	if (mIndex == 1) {
		f32 last = mMotion->nbFrame - 1.0f;
		mFrame += 0.1f;
		if (mFrame >= last) {
			mFrame -= last;
		}
	}
}

// 0x8013CBE8
void TEneOrangeBossFloor::vfn_10()
{
	mFlags &= ~0x10;
}

// 0x8013CBF8
void TEneOrangeBossFloor::render()
{
	Vec3f eye;
	Vec3f at;
	Vec3f point;
	at.x = 0.0f;
	at.y = 50.0f;
	at.z = 0.0f;
	njCalcPointEx(&at, &eye);
	at.y -= 50.0f;
	njSetTexture(orangeBossMapTextures()[0]);
	NJS_OBJECT* piece = orangeBossMapModels()[gOrangeFloorModels[mIndex]];
	fogOff();
	for (int i = 0; i < 8; i++) {
		njCalcPointEx(&mPoint[i], &point);
		if (mIndex == 1 || point.z - eye.z < 150.0f) {
			njPushMatrixEx();
			njTranslateEx(&at);
			gjRotateY(mAngle[i] + 0x8000);
			gjDrawObject(piece);
			njPopMatrixEx();
		}
	}
	if (mIndex == 1) {
		piece = orangeBossMapModels()[0];
		njPushMatrixEx();
		njTranslateEx(&at);
		gjDrawMotion(piece, (void*)mMotion, mFrame);
		njPopMatrixEx();
	}
	fogApply();
}
