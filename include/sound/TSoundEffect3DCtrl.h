#ifndef _SOUND_TSOUNDEFFECT3DCTRL_H
#define _SOUND_TSOUNDEFFECT3DCTRL_H
#include "Ninja/ninja.h"
#include "io/nt_arc.h"
#include "stage/Stage.h"
struct Sound3DList;
struct TSoundEffect3DCtrl;

#include "core/types.h"
#include "sound/SoundHandle.h"

/**
 * @size 0xC
 */
struct Sound3DList {
	Sound3DList();
	~Sound3DList();

	void append(SoundHandle* handle);
	void remove(SoundHandle* handle);
	void clear();

	SoundHandle* mTail; // _00
	SoundHandle* mHead; // _04
	int mCount;         // _08
};

/**
 * @size 0x48
 */
class TSoundEffect3DCtrl : public TObject {
public:
	TSoundEffect3DCtrl(TObject* parent);
	virtual ~TSoundEffect3DCtrl();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();

	void setMono(int mono);
	void setListener(const Vec3f* pos, const Vec3f* dir);
	void calc(f32 range, const Vec3f* pos, int near, int far, u8* volume, f32, u8* pan);
	int play(SoundHandle* handle, int id, int group, int mode, const Vec3f* pos, int near, int far, u16 extra, f32 range, f32);
	void stop(SoundHandle* handle);
	void pauseAll();
	void resumeAll();

	int mMono;         // _20
	Sound3DList mList; // _24
	Vec3f mPos;        // _30
	Vec3f mDir;        // _3C
};

extern "C" {
extern const char* gTSoundEffect3DCtrlName;
}

#endif
