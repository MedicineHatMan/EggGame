#ifndef _TMAINTASK_H
#define _TMAINTASK_H
struct TObject;

#include "stage/Stage.h"

extern "C" void objHeapFree(void*, void*);

/**
 * The object-task root. The one global is gMainTask. draw() and vfn_10()
 * walk the list at that global's +0x14 and time the two passes. vfn_14() runs
 * the two small updaters at 0x8000C360 and 0x8000C2B8.
 *
 * TexList.cpp sits between the constructor and these overrides, so the text is
 * split: 0x80014214-0x800142D4 and 0x80014600-0x80014AA4.
 *
 * @size 0x20
 */
class TMainTask : public TObject {
public:
	TMainTask();
	virtual ~TMainTask();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();   // _0C 0x80014600
	virtual void vfn_10(); // _10 0x800146B8
	virtual void vfn_14(); // _14 0x80014770

	// _1C is the draw-pass time in microseconds
};

extern TMainTask gMainTask; // 0x80279A24

extern "C" {
void objClearLayers(TMainTask* task);             // 0x80014794 empties gTL_01 to gTL_TIMESTOP
void objClearAllLayers(TMainTask* task);          // 0x80014890 and gTL_00
void objPushLayers(TMainTask* task);              // 0x800148B8 new objects go to the B layers
void objMoveChildren(TObject* to, TObject* from); // 0x80014960
void objPopLayers(TMainTask* task);               // 0x800149AC the B layers' children go back to On
}

extern "C" {
}

extern "C" {
void initObjects(TObject*);
}

extern "C" {
extern TObject* gTL_05;
extern TObject* gTL_06;
extern TObject* gTL_01;
extern TObject* gTL_02;
extern const char* gTMainTaskName;
extern u32 sMainTaskProcMask;
extern u32 sMainTaskProcMaskPrev;
extern u32 sMainTaskDrawMask;
extern u32 sMainTaskDrawMaskPrev;
extern const char* gTL_SUName;
extern const char* gTL_00Name;
extern const char* gTL_01Name;
extern const char* gTL_02Name;
extern const char* gTL_03Name;
extern const char* gTL_04Name;
extern const char* gTL_05Name;
extern const char* gTL_06Name;
extern const char* gTL_07Name;
extern const char* gTL_08Name;
extern const char* gTL_PARTICLEName;
extern const char* gTL_TIMESTOPName;
extern const char* gTL_03BName;
extern const char* gTL_04BName;
extern const char* gTL_05BName;
extern const char* gTL_06BName;
extern TObject* gTL_00;
extern TObject* gTL_03;
extern TObject* gTL_04;
extern TObject* gTL_07;
extern TObject* gTL_08;
extern TObject* gTL_PARTICLE;
extern TObject* gTL_TIMESTOP;
extern TObject* gTL_03B;
extern TObject* gTL_04B;
extern TObject* gTL_05B;
extern TObject* gTL_06B;
extern TObject* gTL_03On;
extern TObject* gTL_04On;
extern TObject* gTL_05On;
extern TObject* gTL_06On;
}

#endif
