#include "stage/Stage.h"
#include "Ninja/ninja.h"
#include "engine/MotionPose.h"
#include "object/ObjAction.h"
#include "enemy/boss/TEneLastBoss.h"
#include "enemy/EnemyFx.h"
#include "enemy/boss/TEneLastBossDemo_Darkness.h"

extern "C" {
}

typedef char TEneLastBossDemo_Darkness_size_check[(sizeof(TEneLastBossDemo_Darkness) == 0x68) ? 1 : -1];

const char* gTEneLastBossDemo_DarknessName = "TEneLastBossDemo_Darkness";

// The two particle ids one per hand
static const int kEffect[2] = { 67, 68 };

// 0x80184850
TEneLastBossDemo_Darkness::TEneLastBossDemo_Darkness(TObject* parent)
    : TObject(parent)
{
	_00    = (u32)gTEneLastBossDemo_DarknessName;
	mSize  = 0x68;
	motionObjInit(&mBank);
	mTable = *motionTEneLastBoss(2);
	mTime  = 0.0f;
	for (int i = 0; i < 2; i++) {
		mFx[i] = 0;
	}
}

// 0x801848DC
TEneLastBossDemo_Darkness::~TEneLastBossDemo_Darkness()
{
	int i;

	for (i = 1; i >= 0; i--) {
		if (mFx[i] != 0) {
			((TObject*)mFx[i])->mFlags |= 1;
			mFx[i] = 0;
		}
	}
}

// 0x80184974
void TEneLastBossDemo_Darkness::draw()
{
	MotionNodePose sample;
	Vec3f point;
	f32 limit;
	int i;

	limit = (u32)(*(u32*)((u8*)mTable + 4) - 1);
	mTime += 1.0f;
	if (mTime > limit) {
		mTime = limit;
	}
	motionGetNodePose(&sample, &mBank, mTable, mTime);
	motionPoseGetPos(&sample, &point);
	for (i = 0; i < 2; i++) {
		particleKeepAt(&mFx[i], &point, 1, kEffect[i]);
	}
}
