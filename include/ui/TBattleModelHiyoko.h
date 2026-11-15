#ifndef _TBATTLEMODELHIYOKO_H
#define _TBATTLEMODELHIYOKO_H
struct GjActor;

#include "engine/MotionPlayer.h"
struct DrawTask;
#include "stage/Stage.h"
#include "ui/TBattleModelPlayer.h"

extern "C" void objHeapFree(void*, void*);

/**
 * Chick model on the battle screen. mActor is the shape buffer the motion is drawn into.
 * render() draws that actor, then a second model (the shadow) with the z mode bit set.
 *
 * @size 0xB0
 */
class TBattleModelHiyoko : public TObject {
public:
	TBattleModelHiyoko(TObject* parent, const Vec3f* pos, int* angle);
	virtual ~TBattleModelHiyoko();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C

	void setup();        // 0x80198960
	void updateMatrix(); // 0x80198B00
	void stepMotion();   // 0x80198B44
	void play(s16 id);   // 0x80198C00

	GjActor* mActor;      // _20
	f32 mMtx[12];         // _24
	Vec3f mPos;           // _54
	BattleAngle mAngle;   // _60
	DrawTask* mDraw;      // _6C
	MotionPlayer mMotion; // _70
	s16 mMotionId;        // _AC
	u16 _AE;              // _AE
};

/**
 * A still model from the same archive (model 2), drawn at a scale. It shares the
 * "TBattleModelHiyoko" name and the block pointer. The name is ours.
 *
 * @size 0x74
 */
class TBattleModelHiyokoProp : public TObject {
public:
	TBattleModelHiyokoProp(TObject* parent, const Vec3f* pos, int* angle);
	virtual ~TBattleModelHiyokoProp();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C

	void setup();        // 0x80198DE0
	void updateMatrix(); // 0x80198EAC

	void* mBlock;       // _20
	f32 mMtx[12];       // _24
	Vec3f mPos;         // _54
	BattleAngle mAngle; // _60
	f32 mScale;         // _6C
	DrawTask* mDraw;    // _70
};

class TObject;
struct Vec3f;

extern "C" {
TBattleModelHiyoko* createTBattleModelHiyoko(TObject*, const Vec3f*, const int*);
void freeBattleModelHiyoko();
void loadBattleModelHiyoko();
}

extern "C" {
TBattleModelHiyokoProp* createTBattleModelHiyokoProp(TObject*, const Vec3f*, const int*);
}

extern "C" {
extern const char* gTBattleModelHiyokoName;
extern f32 sBattleHiyokoMtx[12];
extern const MotionEntry sBattleModelHiyokoMotions[];
}

#endif
