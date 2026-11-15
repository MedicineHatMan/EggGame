#ifndef _ANIMAL_TANICLIONEPENGUIN_H
#define _ANIMAL_TANICLIONEPENGUIN_H
#include "animal/AniAttacks.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "stage/Stage.h"
struct TAniClionePenguin;

#include "core/types.h"

/**
 * @size 0x3BC
 */
class TAniClionePenguin : public TEggCreatureTypeE {
public:
	TAniClionePenguin(TObject* parent, int player, int kind, const Vec3f* pos);
	virtual ~TAniClionePenguin();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void render();
	virtual void resetBones();
	virtual void onPhase(int phase);

	void runRise();
	void runEnd();
	inline void killBubble();

	Vec3f mEnd;          // _384 where the attack finished
	Vec3f _390;          // _390
	Vec3f _39C;          // _39C
	int _3A8;            // _3A8
	int _3AC;            // _3AC
	int _3B0;            // _3B0
	f32 mBubbleSize;     // _3B4
	TAniBubble* mBubble; // _3B8
};

#include "engine/MotionPlayer.h"
extern "C" {
extern const char* gTAniClionePenguinName;
extern const f32 sClionePenguinRate0;
extern const f32 sClionePenguinRate1;
extern const f32 sClionePenguinRate2;
extern const f32 sClionePenguinRate3;
extern const f32 sClionePenguinRate4;
extern void* const sClionePenguinCallbacks[];
extern MotionEntry gClioneMotions[8];
}

#endif
