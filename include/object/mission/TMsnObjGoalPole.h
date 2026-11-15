#ifndef _OBJECT_MISSION_TMSNOBJGOALPOLE_H_
#define _OBJECT_MISSION_TMSNOBJGOALPOLE_H_
struct ColliShape;
struct TArc;
#include "egg/ObjEgg.h"
#include "engine/CharaColli.h"
#include "engine/DrawTask.h"
#include "engine/FlagCloth.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct SetDesc;
struct TMsnObjGoalPole;
struct TObject;

#include "core/types.h"

extern "C" {
void initGoalArc();
void loadGoalArc();
void releaseGoalArc();
int scriptLoadGoal(void*);
}

extern "C" {
TObject* createTMsnObjGoalPole(TObject*, SetDesc*);
}

class TMsnObjGoalPole : public TSetActor {
public:
	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }
	TMsnObjGoalPole(TObject* parent, SetDesc* desc);

	virtual ~TMsnObjGoalPole();

	virtual void draw();   // _0C the update
	virtual void vfn_10(); // _10
	virtual void render(); // _1C
	virtual int getKind(); // _2C
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);

	void checkFirst();
	void checkSecond();
	void drawCloth();
	void calcNormals();

	ObjModel mModel;         // _64
	ObjPlace mPlace;         // _84
	FlagCloth* mCloth;       // _E0
	HitVec mNormals[4][4];   // _E4 one per cloth point
	DrawTask* mDrawTask;     // _1A4
	int mState;              // _1A8
	int mWindTarget;         // _1AC
	int mWindAngle;          // _1B0
	Vec3f mCenter;           // _1B4
	f32 mRadius;             // _1C0
	CharaColli* mClothColli; // _1C4
};

#include "Ninja/njtypes.h"
extern "C" {
extern NJS_OBJECT* sGoalModel;
extern NJS_TEXLIST* sGoalTexList;
extern ColliShape sMsnGoalPoleShape[1];
extern TArc* sGoalArc;
extern NJS_TEXLIST* sGoalFlagTexList;
}

#endif
