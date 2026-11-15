#ifndef _SOUND_TMUSICADX_H_
#define _SOUND_TMUSICADX_H_
#include "stage/Stage.h"
struct TMusicADX;

#include "core/types.h"

extern "C" {
void musicServer(int);
}

/**
 * @size 0x54
 */
class TMusicADX : public TObject {
public:
	TMusicADX(TObject* parent);
	virtual ~TMusicADX();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void play(const char* name);
	void stop();
	void applyVolume(f32 volume);
	void fadeTo(u16 frames, f32 volume);
	void pause();
	void resume();

	void* mPlayer;    // _20
	void* mWork;      // _24
	char mName[0x20]; // _28
	s8 mState;        // _48 0 playing 1 waiting for the drive 2 drive error 3 stopped
	u16 mFade;        // _4A frames left
	f32 mVolume;      // _4C
	f32 mTarget;      // _50
};

extern "C" {
extern const char* gTMusicADXName;
}

#endif
