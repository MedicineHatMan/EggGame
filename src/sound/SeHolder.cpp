#include "sound/SeHolder.h"
#include "sound/SoundCtrl.h"
#include "sound/Sound3DPlay.h"

extern "C" {
}

// 0x80123968
extern "C" void seCueClear(SeCue* cue)
{
	cue->mFrame = 0;
	cue->mId    = 0;
}

// 0x80123978
extern "C" void seCueSet(SeCue* cue, int frame, int id)
{
	cue->mFrame = frame;
	cue->mId    = id;
}

// 0x80123984
extern "C" void seCueUpdate(SeCue* cue, int frame)
{
	if (cue->mFrame <= frame) {
		bgmFadeTo(0.0f, cue->mId);
		cue->mFrame = 30000;
	}
}

// 0x801239CC
SeHolder::SeHolder()
{
	mPlaying = 0;
	mRange   = 400.0f;
}

// 0x801239EC
SeHolder::~SeHolder()
{
}

// 0x80123A28
void SeHolder::setRange(f32 range)
{
	mRange = range;
}

// 0x80123A30
extern "C" void seHolderStop(SeHolder* holder)
{
	if (holder->mPlaying != 0) {
		seLoopStop(&holder->mSound);
		holder->mPlaying = 0;
	}
}

// 0x80123A70
extern "C" void seHolderPlay(SeHolder* holder, int id, const Vec3f* pos)
{
	if (holder->mPlaying != 0) {
		seLoopMove(&holder->mSound, pos);
	} else {
		holder->mPlaying = 1;
		sePlay3DHeld(holder->mRange, (SoundHandle*)holder, id, 100, pos);
	}
}

// 0x80123ABC
extern "C" int seStart(int id)
{
	return sePlay(id);
}

// 0x80123ADC
extern "C" void seStartArg(int id, int arg)
{
	sePlayFor(id, arg);
}

// 0x80123AFC
extern "C" int seStartAt(int id, const Vec3f* pos)
{
	return sePlay3D(id, pos);
}

// 0x80123B1C
extern "C" int seStartWith(int id, int arg)
{
	int handle = seStart(id);
	seSetVolume(handle, arg);
	return handle;
}

// 0x80123B58
extern "C" int seStartAtWith(int id, const Vec3f* pos, int arg)
{
	int handle = seStartAt(id, pos);
	seSetPitch(handle, arg);
	return handle;
}

// 0x80123B94
extern "C" void seStartAtFar(int id, const void* pos)
{
	sePlay3DRange(700.0f, id, (const Vec3f*)pos, 0x78);
}

// 0x80123BBC
extern "C" void seStartAtRange(int id, const void* pos, int volume, f32 range)
{
	((int (*)(f32, int, const Vec3f*, int))sePlay3DRange)(range, id, (const Vec3f*)pos, volume);
}

// 0x80123BDC
extern "C" void seEnemyDie0(const Vec3f* pos)
{
	seStartAt(0xE3, pos);
	seStartAt(0xE5, pos);
}

// 0x80123C1C
extern "C" void seEnemyDie1(const Vec3f* pos)
{
	seStartAt(0xE1, pos);
	seStartAt(0xE5, pos);
}

// 0x80123C5C
extern "C" void seEnemyDie2(const Vec3f* pos)
{
	seStartAt(0xE2, pos);
	seStartAt(0xE5, pos);
}

// 0x80123C9C
extern "C" void seEnemyCrush(const Vec3f* pos)
{
	seStartAt(0xDF, pos);
}

// 0x80123CC4
extern "C" void seEnemyPuff(const Vec3f* pos)
{
	seStartAt(0x10D, pos);
}

// 0x80123CEC
extern "C" void seLoopStart(SoundHandle* sound, int id, const Vec3f* pos)
{
	sePlay3DHeld(400.0f, sound, id, 100, pos);
}

// 0x80123D18
extern "C" void seLoopMove(SoundHandle* sound, const Vec3f* pos)
{
	sound->mPos.x = pos->x;
	sound->mPos.y = pos->y;
	sound->mPos.z = pos->z;
}

// 0x80123D34
extern "C" void seLoopStop(SoundHandle* sound)
{
	seStop3D(sound);
}
