#include "enemy/EnemyAnim.h"
struct NJS_MOTION;
#include "enemy/boss/BossParts.h"
#include "core/MslMath.h"

// 0x80072A18
EnemyAnim::EnemyAnim()
{
	mPlayer = 0;
	mNext   = -1;
	mBlend  = 0.0f;
}

// 0x80072A34
extern "C" void motionCtrlSetPlayer(void* anim, void* player)
{
	((EnemyAnim*)anim)->mPlayer = (MotionPlayer*)player;
}

// 0x80072A3C
extern "C" void* motionCtrlGetPlayer(void* anim)
{
	return ((EnemyAnim*)anim)->mPlayer;
}

// 0x80072A44
extern "C" void motionCtrlInit(void* anim, void* table, void* motions)
{
	((EnemyAnim*)anim)->mPlayer->init((const MotionEntry*)table, motions);
}

// 0x80072A68
// Starts the queued play then steps the player
extern "C" void motionCtrlUpdate(void* anim)
{
	EnemyAnim* a = (EnemyAnim*)anim;
	if (a->mNext >= 0) {
		if (a->mBlend <= 0.0f) {
			a->mPlayer->play(a->mNext);
		} else {
			a->mPlayer->playBlend(a->mNext, a->mBlend);
		}
		a->mNext  = -1;
		a->mBlend = 0.0f;
	}
	a->mPlayer->update();
}

// 0x80072AE4
extern "C" void motionCtrlPlay(void* anim, int entry)
{
	motionCtrlPlayBlend(0.0f, anim, entry);
}

// 0x80072B08
extern "C" void motionCtrlPlayBlend(f32 rate, void* anim, int entry)
{
	((EnemyAnim*)anim)->mNext  = (s16)entry;
	((EnemyAnim*)anim)->mBlend = rate;
}

// 0x80072B18
extern "C" int motionCtrlIsDone(const void* anim)
{
	return ((EnemyAnim*)anim)->mPlayer->isDone();
}

// 0x80072B3C
extern "C" int motionCtrlCurrent(void* anim)
{
	return ((EnemyAnim*)anim)->mPlayer->mCurrent;
}

// 0x80072B48
extern "C" void motionCtrlSetSpeed(void* anim, f32 speed)
{
	((EnemyAnim*)anim)->mPlayer->mSpeed = speed;
}

// 0x80072B54
extern "C" int motionCtrlIsBlending(const void* anim)
{
	return ((EnemyAnim*)anim)->mPlayer->isBlending();
}

// 0x80072B78
extern "C" NJS_MOTION* motionCtrlGetMotion(const void* anim)
{
	MotionPlayer* player = ((EnemyAnim*)anim)->mPlayer;
	return player->getMotion(player->mCurrent);
}

// 0x80072BA0
extern "C" f32 motionCtrlFrame(void* anim)
{
	return ((EnemyAnim*)anim)->mPlayer->mFrame;
}

// 0x80072BAC
extern "C" void motionCtrlSetFrame(f32 frame, void* anim)
{
	((EnemyAnim*)anim)->mPlayer->mFrame = frame;
}

// 0x80072BB8
extern "C" void* motionCtrlGetBlend(const void* anim)
{
	return ((EnemyAnim*)anim)->mPlayer->getBlend();
}

// 0x80072BDC
extern "C" f32 motionCtrlBlendTime(void* anim)
{
	return ((EnemyAnim*)anim)->mPlayer->mBlendTime;
}

// 0x80072BE8
extern "C" int motionCtrlPassed(f32 frame, void* anim)
{
	return ((EnemyAnim*)anim)->mPlayer->passedFrame(frame);
}

// 0x80072C0C
EnemyMotion::EnemyMotion()
{
}

// 0x80072C3C
EnemyMotion::~EnemyMotion()
{
}

// 0x80072C78
// Holds the player on one frame of an entry
extern "C" void enemyMotionSet(EnemyMotion* motion, int entry, f32 frame)
{
	motion->mPlayer.mFlags &= ~0x1F;
	motion->mPlayer.mPrev    = motion->mPlayer.mCurrent;
	motion->mPlayer.mFrame   = frame;
	motion->mPlayer.mCurrent = entry;
}

// 0x80072C98
// Holds the player partway through a blend from one entry's frame into another's
extern "C" void enemyMotionBlend(EnemyMotion* motion, int fromEntry, f32 from, int toEntry, f32 to, f32 blend)
{
	MotionPlayer* player = &motion->mPlayer;
	player->mFlags &= ~0x1E;
	player->mFlags |= 1;
	player->mBlend.mFrom      = player->getMotion(fromEntry);
	player->mBlend.mTo        = player->getMotion(toEntry);
	player->mBlend.mFromFrame = from;
	player->mBlend.mToFrame   = to;
	player->mBlendTime        = blend;
	player->mCurrent          = fromEntry;
	player->mFrame            = from;
}

// 0x80072D2C
// Blends from the end of one entry into the start of the next
extern "C" void enemyMotionBlendEnds(EnemyMotion* motion, int fromEntry, int toEntry, f32 blend)
{
	f32 from = motion->mPlayer.getEnd(fromEntry);
	f32 to   = motion->mPlayer.getStart(toEntry);
	enemyMotionBlend(motion, fromEntry, from, toEntry, to, blend);
}

// 0x80072DA8
// A step that plays a row of the motion table looped rows loop the rest play once
extern "C" void motionStepFromEntry(f32 time, MotionStep* step, int entry, const MotionEntry* table)
{
	if (table[entry].mType == MOTION_LOOP) {
		motionStepLoop(time, table[entry].mStart, table[entry].mEnd, table[entry].mRate, step, entry);
	} else {
		motionStepOnce(time, table[entry].mStart, table[entry].mEnd, table[entry].mRate, step, entry);
	}
}

// 0x80072E00
extern "C" void motionStepOnce(f32 time, f32 start, f32 end, f32 rate, MotionStep* step, int entry)
{
	step->mType  = MotionStep::STEP_ONCE;
	step->mEntry = entry;
	step->mTime  = time;
	step->mRate  = rate;
	step->mStart = start;
	step->mEnd   = end;
}

// 0x80072E20
extern "C" void motionStepLoop(f32 time, f32 start, f32 end, f32 rate, MotionStep* step, int entry)
{
	step->mType  = MotionStep::STEP_LOOP;
	step->mEntry = entry;
	step->mTime  = time;
	step->mRate  = rate;
	step->mStart = start;
	step->mEnd   = end;
}

// 0x80072E40
// Blends from where the motion is now
extern "C" void motionStepBlend(f32 time, f32 rate, MotionStep* step, EnemyMotion* from)
{
	step->mType  = MotionStep::STEP_BLEND;
	step->mEntry = from->mPlayer.mCurrent;
	step->mTime  = time;
	step->mRate  = rate;
	step->mStart = from->mPlayer.mFrame;
	step->mEnd   = 0.0f;
}

// 0x80072E6C
// Blends from the frame the step before has reached at this time
extern "C" void motionStepBlendOut(f32 time, f32 rate, MotionStep* step, const MotionStep* from)
{
	int entry = from->mEntry;
	f32 t     = time - from->mTime;
	f32 frame;
	if (from->mType == MotionStep::STEP_ONCE) {
		frame = motionStepOnceFrame(t, from->mStart, from->mEnd, from->mRate);
	} else if (from->mType == MotionStep::STEP_LOOP) {
		frame = motionStepLoopFrame(t, from->mStart, from->mEnd, from->mRate);
	} else {
		entry = 0;
		frame = 0.0f;
	}
	step->mType  = MotionStep::STEP_BLEND;
	step->mEntry = entry;
	step->mTime  = time;
	step->mRate  = rate;
	step->mStart = frame;
	step->mEnd   = 0.0f;
}

// 0x80072F2C
extern "C" void motionStepEnd(MotionStep* step)
{
	step->mType  = MotionStep::STEP_END;
	step->mEntry = 0;
	step->mTime  = 0.0f;
	step->mRate  = 0.0f;
	step->mStart = 0.0f;
	step->mEnd   = 0.0f;
}

// 0x80072F54
extern "C" f32 motionStepOnceFrame(f32 time, f32 start, f32 end, f32 rate)
{
	f32 frame = time * rate;
	frame += start;
	if (frame < start) {
		return start;
	}
	if (frame > end) {
		return end;
	}
	return frame;
}

// 0x80072F7C
extern "C" f32 motionStepLoopFrame(f32 time, f32 start, f32 end, f32 rate)
{
	f32 length = end - start;
	f32 at     = njFraction(time * rate / length);
	if (at < 0.0f) {
		at += 1.0f;
	}
	at *= length;
	return at + start;
}

// 0x80072FE8
// From 0 to 1
extern "C" f32 motionStepBlendRate(f32 time, f32 rate)
{
	f32 blend = time * rate;
	if (blend < 0.0f) {
		blend = 0.0f;
	} else if (blend > 1.0f) {
		blend = 1.0f;
	}
	return blend;
}

// 0x80073014
// Poses the motion at this time on the timeline
extern "C" int motionStepsPlay(f32 time, const MotionStep* steps, EnemyMotion* motion)
{
	const MotionStep* step;
	const MotionStep* next;
	int done;
	done = 0;
	next = steps;
	for (;;) {
		step = next++;
		if (next->mType == MotionStep::STEP_END) {
			done = 1;
			break;
		}
		if (time < next->mTime) {
			break;
		}
	}
	time -= step->mTime;
	switch (step->mType) {
	case MotionStep::STEP_ONCE: {
		f32 frame = motionStepOnceFrame(time, step->mStart, step->mEnd, step->mRate);
		enemyMotionSet(motion, step->mEntry, frame);
		done &= (frame >= step->mEnd);
		break;
	}
	case MotionStep::STEP_LOOP:
		enemyMotionSet(motion, step->mEntry, motionStepLoopFrame(time, step->mStart, step->mEnd, step->mRate));
		done = 0;
		break;
	case MotionStep::STEP_BLEND:
		enemyMotionBlend(motion, step->mEntry, step->mStart, next->mEntry, next->mStart, motionStepBlendRate(time, step->mRate));
		done = 0;
		break;
	}
	return done;
}
