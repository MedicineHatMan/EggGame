#ifndef _TOBJPURPLEBLIZZARD_H
#define _TOBJPURPLEBLIZZARD_H
struct Fx;

#include "stage/Stage.h"
#include "Ninja/ninja.h"

extern "C" void objHeapFree(void*, void*);

/**
 * Bank 8 particle effects kept in front of the camera.
 * Kind 0 loads indices 0 and 1, kind 1 loads 2 and 3, and kind 2 loads index 4
 * with a null second pointer. Any other kind stores two nulls. Kind 2 sits 50
 * above the point 100 ahead of the camera. The others turn so Z follows the wind.
 * The first effect is then offset by -200 along that wind and 50 up, and the
 * second by -20.
 *
 * @size 0x2C
 */
class TObjPurpleBlizzard : public TObject {
public:
	TObjPurpleBlizzard(TObject* parent, int kind);
	virtual ~TObjPurpleBlizzard();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw(); // _0C

	void freeEffects(); // name is ours

	u8 mKind;   // _20
	u8 _21[3];  // _21
	Fx* mFx;  // _24 particle effect Its matrix is at +0x2C
	Fx* mFx2; // _28 second effect null for kind 2
};

class TStage;

extern "C" {
void blizzardExit();
void blizzardInit();
TStage* createPurpleBossStage();
TStage* createPurpleStage();
void initCage();
void termCage();
}

extern "C" {
int scriptBlizzard(int*);
}

extern "C" {
extern u32 gTObjPurpleBlizzardName;
}

extern "C" {
extern NJS_OBJECT* gCageShadowModel;
extern NJS_TEXLIST* gCageTexList;
}

#endif
