#include "engine/MotionPlayer.h"
struct NJS_MOTION;

MotionPlayer::MotionPlayer()
{
	mTable   = 0;
	mMotions = 0;
	mShapes  = 0;
	mFlags   = 0;
	mState   = 0;
	mPrev    = -1;
	mCurrent = -1;
	mSpeed   = 1.0f;
}

void MotionPlayer::init(const MotionEntry* table, void* motions)
{
	mTable   = table;
	mMotions = (NJS_MOTION**)motions;
	mShapes  = 0;
	mState   = 2;
	setMotion(mCurrent);
}

void MotionPlayer::init(const MotionEntry* table, void* motions, void* shapes)
{
	mTable   = table;
	mMotions = (NJS_MOTION**)motions;
	mShapes  = (void**)shapes;
	mState   = 2;
	setMotion(mCurrent);
}

void MotionPlayer::play(s16 id)
{
	if (mTable != 0) {
		mFlags &= ~0xE;
		mFlags |= 2;
		mFlags &= ~0x10;
		mPrev    = mCurrent;
		mCurrent = id;
		mFrame   = 0.0f;
	}
}

void MotionPlayer::playBlend(s16 id, f32 rate)
{
	if (mTable != 0) {
		mFlags &= ~0xE;
		mFlags |= 6;
		mFlags &= ~0x10;
		mPrev      = mCurrent;
		mCurrent   = id;
		mBlendRate = rate;
	}
}

void MotionPlayer::playSynced(s16 id, f32 rate)
{
	if (mTable != 0) {
		getType(id);
		mFlags &= ~0xE;
		mFlags |= 10;
		mFlags &= ~0x10;
		mCurrent   = id;
		mBlendRate = rate;
	}
}

void MotionPlayer::set(s16 id)
{
	if (mTable != 0) {
		mFlags &= ~0xE;
		mFlags &= ~0x10;
		setMotion(id);
		mState = 2;
	}
}

int MotionPlayer::isBlending()
{
	return mFlags & 1;
}

int MotionPlayer::isDone()
{
	return mFlags & 0x10;
}

MotionBlend* MotionPlayer::getBlend()
{
	return &mBlend;
}

void MotionPlayer::update()
{
	if (mFlags & 2) {
		if (mFlags & 4) {
			mState            = 3;
			mBlend.mFrom      = getMotion(mPrev);
			mBlend.mTo        = getMotion(mCurrent);
			mBlend.mFromFrame = mFrame;
			mBlend.mToFrame   = getStart(mCurrent);
			mFrame            = getStart(mCurrent);
			startBlend();
		} else if (mFlags & 8) {
			mState       = 4;
			mBlend.mFrom = getMotion(mPrev);
			mBlend.mTo   = getMotion(mCurrent);
			startBlend();
		} else {
			mState = 1;
			setMotion(mCurrent);
		}
		mPrev = mCurrent;
		mFlags &= ~0xE;
	}

	switch (mState) {
	case 1:
		mState = 2;
		break;
	case 2:
		switch (getType(mCurrent)) {
		case MOTION_LOOP:
			if (advance()) {
				mFrame = getStart(mCurrent);
			}
			break;
		case MOTION_ONCE:
			if (advance()) {
				mFlags |= 0x10;
				mFrame = getEnd(mCurrent);
			}
			break;
		case MOTION_CHAIN:
			if (advance()) {
				mPrev = mCurrent;
				setMotion(getNext(mCurrent));
			}
			break;
		case MOTION_BLEND:
			if (isBlending() == 0) {
				if (advance()) {
					mBlend.mFrom      = getMotion(mCurrent);
					mBlend.mTo        = getMotion(getNext(mCurrent));
					mBlend.mFromFrame = getEnd(mCurrent);
					mBlend.mToFrame   = getStart(getNext(mCurrent));
					startBlend();
				}
			} else {
				mBlendTime += getBlendRate(mCurrent);
				if (mBlendTime >= 1.0f) {
					setMotion(getNext(mCurrent));
					mPrev = mCurrent;
				}
			}
			break;
		}
		break;
	case 3:
		mBlendTime += mBlendRate;
		if (mBlendTime >= 1.0f) {
			mState = 2;
			mFrame = getStart(mCurrent);
			endBlend();
		}
		break;
	case 4:
		if (advance()) {
			mFrame = getStart(mCurrent);
		}
		mBlend.mFromFrame = mBlend.mToFrame = mFrame;
		mBlendTime += mBlendRate;
		if (mBlendTime >= 1.0f) {
			mState = 2;
			endBlend();
		}
		break;
	}
}

void MotionPlayer::setMotion(s16 id)
{
	mCurrent = id;
	mFrame   = getStart(id);
	endBlend();
}

// Returns 1 when the frame reaches the end in whichever direction the motion runs
int MotionPlayer::advance()
{
	mPrevFrame = mFrame;
	mFrame += mSpeed * getRate(mCurrent);
	if (getRate(mCurrent) >= 0.0f) {
		if (mFrame >= getEnd(mCurrent)) {
			mFlags |= 0x100;
			return 1;
		}
	} else {
		if (mFrame <= getEnd(mCurrent)) {
			mFlags |= 0x100;
			return 1;
		}
	}
	return 0;
}

void MotionPlayer::startBlend()
{
	mFlags |= 1;
	mBlendTime = 0.0f;
}

void MotionPlayer::endBlend()
{
	mFlags &= ~1;
}

// Whether the last update stepped over this frame
int MotionPlayer::passedFrame(f32 frame)
{
	if (isBlending()) {
		return 0;
	}
	if (getRate(mCurrent) >= 0.0f) {
		return mPrevFrame <= frame && frame < mFrame;
	}
	return mPrevFrame >= frame && frame > mFrame;
}

u32 MotionPlayer::getUser(s16 id)
{
	return mTable[id].mUser;
}

s16 MotionPlayer::getType(s16 id)
{
	if (id == -1) {
		return 4;
	}
	return mTable[id].mType;
}

s16 MotionPlayer::getNext(s16 id)
{
	if (id == -1) {
		return -1;
	}
	return mTable[id].mNext;
}

NJS_MOTION* MotionPlayer::getMotion(s16 id)
{
	if (id == -1) {
		return 0;
	}
	NJS_MOTION** motions = mMotions;
	if (motions == 0) {
		return 0;
	}
	int index = mTable[id].mMotion;
	if (index != -1) {
		return motions[index];
	}
	return 0;
}

// This ignores the row it is given and reads the current one
void* MotionPlayer::getShape(s16 id)
{
	if (id == -1) {
		return 0;
	}
	void** shapes = mShapes;
	if (shapes == 0) {
		return 0;
	}
	int index = mTable[mCurrent].mShape;
	if (index != -1) {
		return shapes[index];
	}
	return 0;
}

f32 MotionPlayer::getStart(s16 id)
{
	if (id == -1) {
		return 0.0f;
	}
	return mTable[id].mStart;
}

f32 MotionPlayer::getEnd(s16 id)
{
	if (id == -1) {
		return 0.0f;
	}
	return mTable[id].mEnd;
}

f32 MotionPlayer::getRate(s16 id)
{
	if (id == -1) {
		return 0.0f;
	}
	return mTable[id].mRate;
}

f32 MotionPlayer::getBlendRate(s16 id)
{
	if (id == -1) {
		return 0.0f;
	}
	return mTable[id].mBlendRate;
}

void MotionPlayer::getCurrent(void** motion, void** shape)
{
	if (motion != 0) {
		*motion = getMotion(mCurrent);
	}
	if (shape != 0) {
		*shape = getShape(mCurrent);
	}
}
