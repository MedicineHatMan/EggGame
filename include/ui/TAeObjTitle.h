#ifndef _TAEOBJTITLE_H
#define _TAEOBJTITLE_H
struct TArc;

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * Title screen. Layer id 0xF9 on draw list 0x3E. A second layer pointer at _3C starts empty.
 * draw() steps both unless 0x80018E20 says to hold, and calls showPrompt once the first
 * clip finishes while the byte at _44 is still 0.
 *
 * @size 0x48
 */
class TAeObjTitle : public TAeObject {
public:
	TAeObjTitle(TObject* parent, void* data, void* assets);
	virtual ~TAeObjTitle();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 empty
	virtual void render(); // _1C

	void showPrompt();
	void pressed();

	DrawTask* mTask;   // _34
	TAeLayer* mLayer;  // _38
	TAeLayer* mLayer2; // _3C
	u32 _40;           // _40
	s8 mState;         // _44 name is ours
	u8 _45[3];         // _45
};

extern "C" {
TAeObjTitle* createAeTitle();
void freeAeTitle();
void loadAeTitle();
}

#include "Ninja/njtypes.h"
extern "C" {
extern const char* gTAeObjTitleName;
extern TArc* sAeTitleArc;
extern void* sAeTitleAe;
extern NJS_TEXLIST* sAeTitleTexList;
}

#endif
