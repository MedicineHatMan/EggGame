#ifndef _OBJECT_YELLOW_TOBJYELLOWDOUBLEFOG_H_
#define _OBJECT_YELLOW_TOBJYELLOWDOUBLEFOG_H_
#include "stage/Stage.h"
struct TObjYellowDoubleFog;

#include "core/types.h"

extern "C" {
void createYellowDoubleFog();
void initYellowDoubleFog();
void termYellowDoubleFog();
void triggerYellowDoubleFog();
}

/**
 * @size 0x30
 */
class TObjYellowDoubleFog : public TObject {
public:
	TObjYellowDoubleFog(TObject* parent);
	virtual ~TObjYellowDoubleFog();
	virtual void draw();

	void trigger();
	void setFog(f32 rate, int low, int high);

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	int mState;      // _20 0 = choose 1 = high fog 2 = low fog
	int mBoardTimer; // _24
	int mTriggers;   // _28 set by TObjYellowSandGen and cleared every frame
	int mStormTimer; // _2C
};

extern "C" {
extern const char* gTObjYellowDoubleFogName;
extern TObjYellowDoubleFog* sYellowDoubleFogObj;
}

#endif
