#ifndef _TAEOBJMISSIONICON_H
#define _TAEOBJMISSIONICON_H
struct TArc;

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * The message codes 0xBE and 0xBF carry two ints: a count, or minutes then seconds.
 *
 * @size 0xC
 */
struct MissionIconMsg {
	s16 mCode;  // _00
	int mValue; // _04 the count or the minutes
	int mSub;   // _08 the seconds
};

/**
 * Message code 0xC1 carries how much of the gauge is left, 0 to 1.
 *
 * @size 0x8
 */
struct MissionIconRateMsg {
	s16 mCode;  // _00
	f32 mRate;  // _04
};

/**
 * Mission icon. The fourth constructor argument picks one of nine setups.
 * draw() steps that setup. render() draws the layer at _34.
 * vfn_20 dispatches message codes 0xBE-0xC3.
 * Further vtable slots from +0x24 onward point into 0x80100138, which is one function
 * in this run and is not split out here.
 *
 * @size 0x7C
 */
class TAeObjMissionIcon : public TAeObject {
public:
	TAeObjMissionIcon(TObject* parent, void* data, void* assets, u32 kind);
	virtual ~TAeObjMissionIcon();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();           // _0C
	virtual void render();         // _1C
	virtual int vfn_20(s16* code); // _20

	TAeLayer* mLayer;    // _34 the frame every other layer is bound into
	TAeLayer* mMarks[4]; // _38 one per counted item or the parts of the gauge
	TAeLayer* mFills[4]; // _48 what each mark shows when it is filled
	DrawTask* mTask;         // _58
	u32 _5C;             // _5C
	int mFrame;          // _60 the time left in frames
	f32 mPulse;          // _64 0 to 1
	f32 mPulseStep;      // _68
	int mKind;           // _6C
	int mCount;          // _70
	int mSound;          // _74 the sePlay handle
	u32 _78;             // _78
};

extern "C" {
TAeObjMissionIcon* createMissionIcon(u32);
void freeMissionIcon();
void loadMissionIcon();
void missionIconBindCounter(TAeObjMissionIcon*);
void missionIconBindMarks(TAeObjMissionIcon*, int);
int missionIconOnC0(TAeObjMissionIcon*, s16*);
int missionIconOnCount(TAeObjMissionIcon*, MissionIconMsg*);
int missionIconOnHide(TAeObjMissionIcon*, s16*);
int missionIconOnRate(TAeObjMissionIcon*, MissionIconRateMsg*);
int missionIconOnShow(TAeObjMissionIcon*, s16*);
int missionIconOnTime(TAeObjMissionIcon*, MissionIconMsg*);
void missionIconPulse(TAeObjMissionIcon*, f32);
int missionIconSetCount0(TAeObjMissionIcon*, int);
int missionIconSetCount1(TAeObjMissionIcon*, int);
int missionIconSetCount2(TAeObjMissionIcon*, int);
int missionIconSetCount3(TAeObjMissionIcon*, int);
int missionIconSetCount6(TAeObjMissionIcon*, int);
int missionIconSetCount7(TAeObjMissionIcon*, int);
int missionIconSetDigit(TAeObjMissionIcon*, u32, int, const int*);
int missionIconSetDigits2(TAeObjMissionIcon*, u32, u32, int, const int*);
void missionIconSetGaugeLevel(TAeObjMissionIcon*, int);
void missionIconSetup0(TAeObjMissionIcon*);
void missionIconSetup1(TAeObjMissionIcon*);
void missionIconSetup2(TAeObjMissionIcon*);
void missionIconSetup3(TAeObjMissionIcon*);
void missionIconSetup4(TAeObjMissionIcon*);
void missionIconSetup5(TAeObjMissionIcon*);
void missionIconSetup6(TAeObjMissionIcon*);
void missionIconSetup7(TAeObjMissionIcon*);
void missionIconSetup8(TAeObjMissionIcon*);
void missionIconStep(TAeObjMissionIcon*);
void missionIconStepCounter(TAeObjMissionIcon*);
void missionIconStepGauge(TAeObjMissionIcon*);
}

#include "Ninja/njtypes.h"
extern "C" {
extern TArc* sMissionIconArc;
extern void* sMissionIconAe;
extern NJS_TEXLIST* sMissionIconTexList;
extern int sMissionIconGauge;
}

#endif
