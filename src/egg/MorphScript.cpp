#include "egg/TMorphEggAnim.h"
#include "Ninja/ninja.h"

static u8* readSpan(u8* data, u16* out);
static u8* readSpanKey(u8* data, u16* out);
static u8* readFloat(const u8* data, f32* out);
static f32 hermite(f32 p0, f32 m0, f32 p1, f32 m1, f32 t);

// 0x8005676C
MorphScript::MorphScript()
{
	mDone   = 0;
	mWait   = 0;
	mActive = 0;
	syMemSet4(mSlot, 0, 16);
}

// 0x800567B8
void MorphScript::setScript(u8* script)
{
	mPc   = script;
	mBase = script;
	mWait = 1;
}

// 0x800567CC
void MorphScript::setSlot(int index, MorphKey* key)
{
	mSlot[index & 0xff] = key;
}

// 0x800567DC
void MorphScript::update()
{
	u32 bits;
	u32 bit;
	int i;
	u8* pc;
	u32 op;
	int hold;

	if (mActive != 0) {
		bits = mActive;
		bit = 1;
		for (i = 0; i < 16; i++) {
			if ((bits & bit) != 0 && mSlot[i]->update() != 0) {
				bits &= ~bit;
			}
			bit <<= 1;
		}
		mActive = bits;
	}
	if (mWait == 0) {
		return;
	}
	if (--mWait != 0) {
		return;
	}
	pc   = mPc;
	hold = 0;
	for (;;) {
		op   = *pc++;
		if (op == 0x7f) {
			mDone = (u8)(mDone | 1);
			return;
		}
		u8 c = op;
		if (c & 0x80) {
			hold = (u16)(c & 0x3f);
			if (c & 0x40) {
				hold = ((hold & 0xFF) << 8) | *pc;
				pc += 1;
			}
		} else if (c == 0x7b) {
			mLoops  = *pc;
			pc += 1;
			mLoopAt = pc;
		} else if (c == 0x7c) {
			if (mLoops != 0) {
				mLoops  = (u8)(mLoops - 1);
				pc      = mLoopAt;
			}
		} else if (c == 0x7d) {
			mMark = pc;
		} else if (c == 0x7e) {
			pc = mMark;
		} else {
			pc = runCmd(pc, op);
		}
		mPc   = pc;
		mWait = hold;
		if (mWait != 0) {
			break;
		}
	}
}

// 0x80056954
u8* MorphScript::runCmd(u8* data, int mode)
{
	u16 len;
	int slot;
	u32 bit;

	data = readSpan(data, &len);
	slot = *data;
	bit  = 1u << slot;
	data += 1;
	if (mSlot[slot] != 0) {
		mActive &= ~bit;
		if (mSlot[slot]->load(data, mode) != 0) {
			mActive |= bit;
		}
	}
	data += len - 1;
	return data;
}

// 0x80056A00
static u8* readSpan(u8* data, u16* out)
{
	u8 c = *data++;
	u8 n = c & 0x7f;
	u32 v = n;
	if (c & 0x80) {
		v = (n << 8) | *data++;
	}
	*out = v;
	return data;
}

// The same encoding emitted again for the key
// 0x80056A2C
static u8* readSpanKey(u8* data, u16* out)
{
	u8 c = *data++;
	u8 n = c & 0x7f;
	u32 v = n;
	if (c & 0x80) {
		v = (n << 8) | *data++;
	}
	*out = v;
	return data;
}

// Four bytes copied as they stand
// 0x80056A58
static u8* readFloat(const u8* data, f32* out)
{
	u8* dst = (u8*)out;
	dst[0] = *data++;
	dst[1] = *data++;
	dst[2] = *data++;
	dst[3] = *data++;
	return (u8*)data;
}

// Cubic hermite p0 and m0 are the start p1 and m1 the end
// 0x80056A84
static f32 hermite(f32 p0, f32 m0, f32 p1, f32 m1, f32 t)
{
	f32 t2 = t * t;
	f32 t3 = t2 * t;
	f32 h00 = 1.0f + (2.0f * t3 - 3.0f * t2);
	f32 h10 = t + (t3 - 2.0f * t2);
	f32 h01 = -2.0f * t3 + 3.0f * t2;
	f32 h11 = t3 - t2;
	return h11 * m1 + (h10 * m0 + (h00 * p0 + h01 * p1));
}

// 0x80056ACC
MorphKey::MorphKey()
{
	mValue = 0.0f;
}

// 0x80056AF8
int MorphKey::load(u8* data, int mode)
{
	mMode = mode;
	switch ((u8)mode) {
	case 0:
		readFloat(data, &mValue);
		mTime = 0;
		return 0;
	case 1:
		data = readSpanKey(data, &mTime);
		readFloat(data, &mTarget);
		return (bool)(mTime != 0);
	case 2:
		data = readSpanKey(data, &mTime);
		readFloat(data, &mTarget);
		mSpan = mTime;
		mFrom = mValue;
		return (bool)(mTime != 0);
	}
	return 0;
}

// 0x80056BCC
int MorphKey::update()
{
	f32 t;

	switch (mMode) {
	case 1:
		mValue += (mTarget - mValue) / (f32)mTime;
		break;
	case 2:
		t = (f32)mTime / (f32)mSpan;
		mValue = hermite(mFrom, 0.0f, mTarget, 0.0f, t);
		break;
	case 3:
		break;
	}
	mTime = (u16)(mTime - 1);
	return mTime == 0;
}
