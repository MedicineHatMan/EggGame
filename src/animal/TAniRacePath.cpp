#include "egg/EggCreature.h"
#include "egg/ObjEgg.h"
#include "animal/TAnimalRacer.h"
#include "stage/TStageScore.h"
#include "animal/TAniRacePath.h"
#include "animal/AniMount.h"
#include "stage/TGameState.h"
#include "object/TObjKey00.h"
#include "sound/SoundCtrl.h"
#include "core/TMainTask.h"
struct ObjMsg;

// What the stage's set data gives aniRaceSetup
struct AniRaceDesc {
	u16 mRailId;   // _00
	u8 _02[2];
	f32 mSpeed;    // _04
	u16 mCourse;   // _08
	u16 mLaps;     // _0A
	u16 mRank;     // _0C
	u8 _0E[2];
	f32 mScale;    // _10
	Vec3f mStart;  // _14
};

typedef char TAniRacePath_size_check[(sizeof(TAniRacePath) == 0x5C) ? 1 : -1];

static TAniRacePath* sPath;  // 0x802C58B8
static TAnimalRacer* sRacer;         // 0x802C58BC
static u16 sRailId;          // 0x802C58C0
static f32 sSpeed;           // 0x802C58C4
static f32 sScale;           // 0x802C58C8
static u16 sCourse;          // 0x802C58CC
static u16 sRank;            // 0x802C58CE
static u16 sLaps;            // 0x802C58D0

// 0x8017AA90
extern "C" void aniRaceNop0()
{
}

// 0x8017AA94
extern "C" void aniRaceNop1()
{
}

// 0x8017AA98
extern "C" TAniRacePath* aniRaceMakePath()
{
	return new TAniRacePath(gTL_05);
}

int sAniRaceGlobal = -1; // 0x802C4580
const char* gTAniRacePathName = "TAniRacePath";

// 0x8017AAD8
TAniRacePath::TAniRacePath(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTAniRacePathName;
	mSize = 0x5C;
	if (sRailId == -1) {
		return;
	}
	mRail.setPath(railFind(sRailId));
	reset();
	mState  = 0;
	mPlace  = 0;
	mTiming = 0;
	mClock  = 0;
	if (keyBitIsSet(0xFA) != 0) {
		mPlace = 2;
		mState = 5;
		if (sRacer != 0) {
			sRacer->setMode(5);
		}
	}
}

// 0x8017ABA0
TAniRacePath::~TAniRacePath()
{
	aniRaceClearPath(this);
}

// 0x8017AC10
void TAniRacePath::draw()
{
	switch (mState) {
	case 0:
		break;
	case 1:
		runRail();
		break;
	case 4:
		runPlaced();
		break;
	case 5:
		if ((int)mPlace == 2) {
			u16 msg[2];
			msg[0] = 0xA9;
			sendGameMsg((ObjMsg*)msg);
		} else {
			u16 msg[2];
			msg[0] = 0xA7;
			sendGameMsg((ObjMsg*)msg);
		}
		break;
	}
	if (mTiming != 0) {
		mClock += 0x10;
	}
}

// 0x8017ACBC
void TAniRacePath::reset()
{
	mState = 2;
	mPos.x = 0.0f;
	mPos.y = 0.0f;
	mPos.z = 0.0f;
	mSpeed = sSpeed;
}

// 0x8017ACE0
void TAniRacePath::toStart()
{
	mState = 3;
	mSpeed = sSpeed;
}

// 0x8017ACF4
void TAniRacePath::runRail()
{
	mRail.advance(mSpeed);
	mRail.getPos(&mPos);
}

// 0x8017AD34
void TAniRacePath::runPlaced()
{
	if ((s8)mPlace == 2) {
		if (sRacer != 0) {
			sRacer->setMode(4);
		}
	} else if ((s8)mPlace == 1) {
		if (sRacer != 0) {
			sRacer->setMode(3);
		}
	}
}

// 0x8017AD94
void TAniRacePath::finish()
{
	scoreAddTime(mClock);
	mState = 5;
}

// 0x8017ADCC
extern "C" int aniRaceSetup(AniRaceDesc* desc)
{
	sRailId      = desc->mRailId;
	sSpeed       = desc->mSpeed;
	sCourse      = desc->mCourse;
	sRank        = desc->mRank;
	sLaps        = desc->mLaps;
	sScale       = desc->mScale;
	sRacePathStart = desc->mStart;
	return 1;
}

// 0x8017AE20
extern "C" int aniRaceSetupMake()
{
	aniRaceMake();
	return 1;
}

// 0x8017AE44
extern "C" int aniRaceStart()
{
	if (sPath != 0 && sRacer != 0) {
		sPath->mState = 1;
		sePlay(0x38);
		return 1;
	}
	return 0;
}

// 0x8017AE94
extern "C" void aniRaceInit()
{
	sPath           = 0;
	sRacer          = 0;
	sAniRaceGlobal    = -1;
	sRacePathStart.x  = 0.0f;
	sRacePathStart.y  = 0.0f;
	sRacePathStart.z  = 0.0f;
	sRailId         = 0xFFFF;
	sSpeed          = 2.0f;
	sScale          = 1.0f;
	sCourse         = 0;
	aniRaceNop0();
	racerInit();
}

// 0x8017AF04
extern "C" void aniRaceEnd()
{
	racerExit();
	aniRaceNop1();
	sPath           = 0;
	sRacer          = 0;
	sAniRaceGlobal    = -1;
	sRacePathStart.x  = 0.0f;
	sRacePathStart.y  = 0.0f;
	sRacePathStart.z  = 0.0f;
	sRailId         = 0xFFFF;
	sSpeed          = 2.0f;
	sScale          = 1.0f;
	sCourse         = 0;
}

// 0x8017AF74
extern "C" void aniRaceSetPath(TAniRacePath* path)
{
	sPath = path;
}

// 0x8017AF7C
extern "C" void aniRaceClearPath(TAniRacePath* path)
{
	if (sPath == path) {
		sPath = 0;
	}
}

// 0x8017AF94
extern "C" void aniRaceSetRacer(TAnimalRacer* racer)
{
	sRacer = racer;
}

// 0x8017AF9C
extern "C" void aniRaceClearRacer(TAnimalRacer* racer)
{
	if (sRacer == racer) {
		sRacer = 0;
	}
}

// 0x8017AFB4
extern "C" void aniRaceSetGlobal(int value)
{
	sAniRaceGlobal = value;
}

// 0x8017AFBC
extern "C" void aniRaceMake()
{
	aniRaceSetRacer(createTAnimalRacer(&sRacePathStart, sScale));
	aniRaceSetPath(aniRaceMakePath());
}

// 0x8017AFF4
extern "C" void aniRaceRun()
{
	if (sPath != 0) {
		sPath->mState = 1;
	}
}

// 0x8017B00C
extern "C" void aniRaceReset()
{
	if (sPath != 0) {
		sPath->reset();
	}
}

// 0x8017B038
extern "C" void aniRaceToStart()
{
	if (sPath != 0) {
		sPath->toStart();
	}
}

// 0x8017B064
extern "C" void aniRacePlaced()
{
	if (sPath != 0) {
		TAniRacePath* path = sPath;
		if (path->mState != 5) {
			path->mTiming = 0;
			path->mState  = 4;
		}
	}
}

// 0x8017B090
extern "C" void aniRaceFinish()
{
	if (sPath != 0) {
		sPath->finish();
	}
}

// 0x8017B0BC
extern "C" void aniRaceSetSpeed(f32 speed)
{
	if (sPath != 0) {
		sPath->mSpeed = speed;
	}
}

// 0x8017B0D0
extern "C" const Vec3f* aniRaceGetPos()
{
	if (sPath != 0) {
		return &sPath->mPos;
	}
	Vec3f zero;
	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;
	return &zero;
}

// 0x8017B104
extern "C" int aniRaceHasState()
{
	if (sPath != 0) {
		return sPath->mState != 0;
	}
	return 0;
}

// 0x8017B12C
extern "C" void aniRacePlaceFirst()
{
	if (sPath != 0) {
		sPath->mPlace = 1;
	}
}

// 0x8017B144
extern "C" void aniRacePlaceSecond()
{
	if (sPath != 0) {
		sPath->mPlace = 2;
		TAniRacePath* path = sPath;
		path->mClock  = 0;
		path->mTiming = 1;
	}
}

// 0x8017B170
extern "C" u16 aniRaceGetCourse()
{
	return sCourse;
}

// 0x8017B178
extern "C" u16 aniRaceGetRank()
{
	return sRank;
}

// 0x8017B180
extern "C" u16 aniRaceGetLaps()
{
	return sLaps;
}

// 0x8017B188
// The static init of this file an empty blr in .ctors
struct AniRaceStatic {
	AniRaceStatic() { }
};
static AniRaceStatic sAniRaceStatic;
