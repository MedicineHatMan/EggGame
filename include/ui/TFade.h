#ifndef _UI_TFADE_H_
#define _UI_TFADE_H_
struct TAlphaFade;
#include "engine/DrawTask.h"
#include "stage/Stage.h"

#include "core/types.h"

extern "C" {
void alphaFadeClear();
void alphaFadeDelete();
void fadeCircleClose();
void fadeCircleOpen();
void fadeModel00();
void fadeModel01();
void fadeModel10();
void fadeModel11();
void fadeToBlack();
void fadeToWhite();
}

extern "C" {
void alphaFadeDim(int);
}

/**
 * @size 0x30
 */
class TWhiteFadeInOut : public TObject {
public:
	TWhiteFadeInOut(TObject* parent, int toWhite);
	virtual ~TWhiteFadeInOut();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	static void drawTask(TWhiteFadeInOut* fade);

	u16 mFrames;    // _20
	DrawTask* mTask;    // _24
	f32 mAlpha;     // _28
	f32 mTarget;    // _2C
};

/**
 * @size 0x30
 */
class TBlackFadeInOut : public TObject {
public:
	TBlackFadeInOut(TObject* parent, int toBlack);
	virtual ~TBlackFadeInOut();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	static void drawTask(TBlackFadeInOut* fade);

	u16 mFrames;    // _20
	DrawTask* mTask;    // _24
	f32 mAlpha;     // _28
	f32 mTarget;    // _2C
};

/**
 * @size 0x30
 */
class TAlphaFade : public TObject {
public:
	TAlphaFade(TObject* parent, int clearing);
	virtual ~TAlphaFade();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	static void drawTask(TAlphaFade* fade);

	DrawTask* mTask;   // _20
	int mClearing; // _24
	u8 _28[4];     // _28
	f32 mAlpha;    // _2C
};

extern "C" {
extern const char* gTWhiteFadeInOutName;
extern const char* gTBlackFadeInOutName;
extern const char* gTCircleFadeInOutName;
extern const char* gTAlphaFadeName;
extern TAlphaFade* sAlphaFadeObj;
}

#endif
