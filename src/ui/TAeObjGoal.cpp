#include "ui/TAeObjGoal.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "egg/EggCreature.h"
#include "ui/AeArc.h"
#include "ui/AeObjControl.h"
#include "core/TMainTask.h"
struct TArc;

// 0x801D0108
// loads ae_goal.arc 0x801D0174 releases it 0x801D01C0 builds one
// 0x801CF560-0x801D0108 is not this class

typedef char TAeObjGoal_size_check[(sizeof(TAeObjGoal) == 0x3C) ? 1 : -1];

const char* gTAeObjGoalName = "TAeObjGoal";

// 0x801D0108
int loadGoal()
{
	u32* header;

	aeLoadArc((const char*)("ae_goal.arc"), (TArc**)&sAeGoalArc);
	if (sAeGoalArc != 0) {
		header = *(u32**)(sAeGoalArc + 0x4C);
		relocateAe((void*)(sAeGoalData = header[0]));
		sAeGoalTex = header[1];
		bindGvmTextures((u8*)header[2], (NJS_TEXLIST*)sAeGoalTex);
		return 1;
	}
	return 0;
}

// 0x801D0174
void aeGoalFree()
{
	if (sAeGoalArc != 0) {
		releaseTextures((NJS_TEXLIST*)sAeGoalTex);
		sAeGoalTex = 0;
		aeFreeArc((TArc**)&sAeGoalArc);
		sAeGoalArc  = 0;
		sAeGoalData = 0;
	}
}

// 0x801D01C0
TAeObjGoal* createGoal()
{
	if (sAeGoalArc == 0) {
		return 0;
	}
	return new TAeObjGoal((TObject*)gTL_07, (void*)sAeGoalData, (void*)sAeGoalTex);
}

// 0x801D021C
TAeObjGoal::TAeObjGoal(TObject* parent, void* data, void* assets)
    : TAeObject(parent, data, assets)
{
	_00    = (u32)gTAeObjGoalName;
	mSize  = 0x3C;
	mLayer = createLayer(0xB, 0x10);
	mTask  = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x3C, 0);
	_30    = 0;
}

// 0x801D02A0
TAeObjGoal::~TAeObjGoal()
{
	aeGoalFree();
	removeDrawTask(mTask);
}

// 0x801D030C
void TAeObjGoal::draw()
{
	if (mLayer != 0) {
		if (mLayer->isDone() != 0) {
			_30 = 1;
			mFlags |= 1;
		} else {
			mLayer->update();
		}
	}
}

// 0x801D0370
void TAeObjGoal::render()
{
	if (mLayer != 0) {
		mLayer->present();
	}
}
