#ifndef _ENEMY_TENEGROUPCOLLISION_H_
#define _ENEMY_TENEGROUPCOLLISION_H_
#include "enemy/ObjEnemy.h"
#include "stage/SetManager.h"
#include "stage/Stage.h"
struct TEneGroupCollision;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTEneGroupCollision(TObject*, SetDesc*);
}

class TEneGroupCollision : public TObjEnemy {
public:
	enum {
		// mRestart
		RESTART_OFF = 0, // goes once it has let the group out
		RESTART_ON  = 1, // the same and the group may start again
		RESTART_NOT = 2, // waits for the group to be beaten then is gone for good
	};

	enum {
		// the shape's mType
		SHAPE_SPHERE   = 0,
		SHAPE_BOX      = 1,
		SHAPE_CYLINDER = 2,
	};

	TEneGroupCollision(TObject* parent, SetDesc* desc);
	virtual ~TEneGroupCollision();

	inline void* operator new(u32 size) { return objHeapAlloc(gObjHeap, size); }
	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void readEnemyDesc(SetDesc* desc); // _38
	virtual void vfn_3C();                     // _3C prints debug text
	virtual int canAct();                      // _44
	virtual void vfn_6C();                     // _6C
	virtual void updateBody();                 // _70
	virtual void syncCollider();               // _78
	virtual void onActionPhase(int action, int phase); // _7C
	virtual void vfn_B0();                     // _B0

	void init(SetDesc* desc);

	int mCount;   // _158 how many the group lets out
	s16 mGroup;   // _15C
	int mState;   // _160 0 waiting for a touch 1 waiting for the group
	int mRestart; // _164
};

#endif
