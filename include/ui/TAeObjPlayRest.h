#ifndef _TAEOBJPLAYREST_H
#define _TAEOBJPLAYREST_H

#include "ui/TAeLayer.h"
struct DrawTask;

/**
 * @size 0x44
 */
struct PlayRestMsg {
	u16 mType;   // _00 0xB1
	u8 _02[2];
	u16 mPlayer; // _04 ignored unless under 4
	u16 mRest;   // _06
	u16 mChar;   // _08
};

class TAeObjPlayRest : public TAeObject {
public:
	TAeObjPlayRest(TObject* parent, void* data, void* assets);
	virtual ~TAeObjPlayRest();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();    // _0C
	virtual void render();  // _1C
	virtual int vfn_20(s16* code); // _20 0x801004A4 in AeObjControl.cpp

	int onRest(s16* code);
	void setRest(u16 rest, u16 chara);
	void show();
	void hide();

	DrawTask* mTask;      // _34
	TAeLayer* mLayer; // _38
	u16 mChar;        // _3C 0 to 3
	u16 mRest;        // _3E 0 to 99
	int mTimer;       // _40 name is ours
};

class TAeObjCoinNum;
class TObject;

extern "C" {
TAeObjCoinNum* createCoinNum(TObject*);
}

extern "C" {
extern const int gAeDigitCells[10];
extern const char* gTAeObjPlayRestName;
extern const int gRestCharCells[4];
}

#endif
