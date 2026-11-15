#include "ui/TAeObject.h"
#include "ui/TAeLayer.h"

typedef char TAeObject_size_check[(sizeof(TAeObject) == 0x34) ? 1 : -1];

// Comes before every other float of this file in .sdata2
const f32 gAeOne[1] = { 1.0f };

const char* gTAeObjectName = "TAeObject";

f32 gAeFrameTime = 1.0f / 60.0f; // one frame at 60 fps

// 0x800A6FAC
TAeObject::TAeObject(TObject* parent, void* data, void* assets)
    : TObject(parent)
{
	_00     = (u32)gTAeObjectName;
	mSize   = 0x34;
	_2C     = 0;
	mData   = data;
	mAssets = assets;
}

// 0x800A7010
// Groups of 0x38-byte records separated by a zero kind
void relocateAe(void* file)
{
	u32 base       = (u32)file;
	u32* header    = (u32*)file;
	header[0]     += base;
	header[1]     += base;
	AeRecord* rec  = (AeRecord*)header[1];
	while (rec->mKind != 0) {
		while (rec->mKind != 0) {
			rec->mOff20 += base;
			rec->mOff24 += base;
			rec->mOff28 += base;
			rec->mOff2C += base;
			rec->mOff30 += base;
			if (rec->mOff34 != 0) {
				rec->mOff34 += base;
			}
			rec++;
		}
		rec++;
	}
}

// 0x800A70A8
TAeObject::~TAeObject() { }

// 0x800A7104
void* TAeObject::getAssets() { return mAssets; }

// 0x800A710C
TAeLayer* TAeObject::createLayer(int id, u32 flags) { return new TAeLayer(this, this, id, flags); }

#include "ui/TAeLayer.h"
#include "egg/EggCreature.h"
#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "core/Mem.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"

// 0x800A80CC
// subtracts 0x20 and jumps to ~TAeLayer
// 0x800A80D4
// clears the AE pool and is the next unit
// The AE draw helpers it calls are in AeDraw.cpp

typedef char TAeClip_size_check[(sizeof(TAeClip) == 0x88) ? 1 : -1];
typedef char TAeLayer_size_check[(sizeof(TAeLayer) == 0xB0) ? 1 : -1];
typedef char AeSlot_size_check[(sizeof(AeSlot) == 0x18) ? 1 : -1];

// 0x800A716C
TAeClip::TAeClip(TAeObject* owner, u32 id, u32 flags)
{
	mItems = 0;
	setup(owner, id, flags);
}

// 0x800A71B0
TAeClip::~TAeClip()
{
	if (mItems != 0) {
		memFree(mItems);
		mItems = 0;
	}
}

// 0x800A7218
void TAeClip::setup(TAeObject* owner, u32 id, u32 flags)
{
	u32 count = flags & 0x7f;
	mOwner    = owner;
	u32 bits  = flags & 0x380;
	if (count == 0) {
		count = 1;
	}
	mPlayFlags = count | bits;
	mId        = id;
	mTime      = 0.0f;
	aeGetRange(((AeFile*)mOwner->mData)->mList, id & 0xffff, &mStart, &mEnd);
	mtxIdentity(mMtx);
	_50 = 0.0f;
	_54 = 0.0f;
	_58 = 0.98f;
	_5C = 0.0f;
	_60 = gAeOne[0];
	_64 = gAeOne[0];
	_68 = gAeOne[0];
	_6C = gAeOne[0];
	_70 = gAeOne[0];
	_74 = gAeOne[0];
	_78 = gAeOne[0];
	if (mItems != 0) {
		memFree(mItems);
		mItems = 0;
	}
	mCount = 0;
	mItems = 0;
	s16 ids[0x96];
	aeCollectSlots(((AeFile*)mOwner->mData)->mList, (s16)id, ids, 0x96, &mCount);
	if (mCount > 0) {
		mItems      = (AeSlot*)memAlloc(mCount * sizeof(AeSlot));
		int i;
		AeSlot* dst = mItems;
		for (i = 0; i < mCount; i++) {
			dst->mIndex = ids[i];
			dst->_02    = 0;
			dst++;
		}
	}
	mBanks = NULL;
}

// 0x800A7368
void TAeClip::reset(u32 id, u32 flags) { setup(mOwner, id, flags); }

// 0x800A7398
void TAeClip::step()
{
	apply();
	u32 flags = mPlayFlags;
	if ((flags & 0x100) != 0 || (flags & 0x80) != 0) {
		return;
	}

	f32 delta = gAeFrameTime * _68;
	if ((flags & 0x60) != 0) {
		mPrevTime = mTime;
		mTime     = mTime - delta;
	} else {
		mPrevTime = mTime;
		mTime     = mTime + delta;
	}

	f32 now = 0.0001f + mTime;
	if (now < 0.0f || now >= mEnd) {
		flags = mPlayFlags;
		if ((flags & 1) != 0) {
			mTime = 0.0f;
			mPlayFlags |= 0x400;
		} else if ((flags & 2) != 0) {
			if (mTime < 0.0f) {
				mPlayFlags |= 0x400;
			}
			_68 *= -1.0f;
			mTime = -2.0f * delta + mTime;
		} else if ((flags & 4) != 0) {
			mPlayFlags = flags | 0x400;
			hide();
		} else if ((flags & 8) != 0 || (flags & 0x10) != 0) {
			mTime = mEnd - delta;
			mPlayFlags |= 0x400;
			pause();
		} else if ((flags & 0x20) != 0) {
			mTime = 0.0f;
			mPlayFlags |= 0x400;
		} else if ((flags & 0x40) != 0) {
			mTime = 0.0f;
			mPlayFlags |= 0x400;
			hide();
		}
	}
}

// 0x800A753C
void TAeClip::paint()
{
	if ((mPlayFlags & 0x80) != 0 || (mPlayFlags & 0x200) != 0) {
		return;
	}
	f32 now = mTime + 0.0001f;
	if (now < mStart || now >= mEnd) {
		return;
	}
	AeFrame frame;
	fillFrame(&frame);
	njZBufferMode(0);
	pushCullMode(0);
	njPushMatrixEx();
	njSetMatrix(0, mMtx);
	njSetTexture(mOwner->mAssets);
	njTextureClampMode(0x18000);
	njQuadTextureStart(1);
	aeDrawGroup(&frame, (s16)mId, mBanks);
	njQuadTextureEnd();
	njZBufferMode(1);
	popCullMode();
	njPopMatrixEx();
}

// 0x800A7618
void TAeClip::fillFrame(AeFrame* frame)
{
	frame->mHead  = ((AeFile*)mOwner->mData)->_00;
	frame->mList  = ((AeFile*)mOwner->mData)->mList;
	frame->mTime  = mTime;
	frame->mRate  = _58;
	frame->mMark  = 0;
	frame->mColor.a    = _6C;
	frame->mColor.r    = _70;
	frame->mColor.g    = _74;
	frame->mColor.b    = _78;
	frame->mCount = mCount;
	frame->mItems = mItems;
}

// 0x800A7684
// Bit 10 set by step when playback finishes
int TAeClip::isDone() { return (mPlayFlags >> 10) & 1; }

// 0x800A7690
// The window is the step just taken
int TAeClip::containsTime(f32 time)
{
	if ((mPlayFlags & 0x20) != 0) {
		return (u8)(mPrevTime >= time && time > mTime);
	}
	return (u8)(mPrevTime <= time && time < mTime);
}

// 0x800A76F4
void TAeClip::apply()
{
	mtxSetTrans((float*)mMtx, _50, _54, 0.0f);
	mtxRotateZ(mMtx, mMtx, (int)(182.04445f * _5C));
	mtxScale(_60, _64, gAeOne[0], mMtx, mMtx);
}

// 0x800A776C
void TAeClip::setTrans(f32 x, f32 y)
{
	_50 = x;
	_54 = y;
}

// 0x800A7778
void TAeClip::setScale(f32 x, f32 y)
{
	_60 = x;
	_64 = y;
}

// 0x800A7784
void TAeClip::setSpin(f32 angle) { _5C = angle; }

// 0x800A778C
void TAeClip::setColor(f32* color)
{
	_6C = color[0];
	_70 = color[1];
	_74 = color[2];
	_78 = color[3];
}

// 0x800A77B0
void TAeClip::setRate(f32 rate) { _68 = rate; }

// 0x800A77B8
// Replaces the low 7 flag bits
void TAeClip::setMode(u32 mode)
{
	if (mode == 0) {
		mode = 1;
	}
	mPlayFlags &= ~0x7fu;
	mPlayFlags |= mode & 0x7f;
}

// 0x800A77E4
void TAeClip::show() { mPlayFlags &= ~0x180u; }

// 0x800A77F4
// Bit 7 makes step and paint skip this clip
void TAeClip::hide() { mPlayFlags |= 0x80; }

// 0x800A7804
// Bit 9 makes paint skip this clip
void TAeClip::hold() { mPlayFlags |= 0x200; }

// 0x800A7814
void TAeClip::release() { mPlayFlags &= ~0x200u; }

// 0x800A7824
// 1 parks the clip on its last frame
void TAeClip::cue(int mode)
{
	switch (mode) {
	case 0:
		mTime = 0.0f;
		mPlayFlags &= ~0x400u;
		break;
	case 1:
		mTime = mEnd - 0.0002f;
		mPlayFlags |= 0x100;
		break;
	}
}

// 0x800A7870
// Clears hide pause and the done bit
void TAeClip::wake() { mPlayFlags &= ~0x580u; }

// 0x800A7884
// Bit 8 makes step return before it advances the time
void TAeClip::pause() { mPlayFlags |= 0x100; }

// 0x800A7894
void TAeClip::flipPause()
{
	if ((mPlayFlags & 0x100) != 0) {
		wake();
	} else {
		pause();
	}
}

// 0x800A78C8
void TAeClip::home()
{
	cue(0);
	setTrans(gAeOne[0], gAeOne[0]);
	setScale(gAeOne[0], gAeOne[0]);
	setSpin(0.0f);
	setRate(gAeOne[0]);
}

// 0x800A7930
AeSlot* TAeClip::findSlot(int id)
{
	if (mItems == 0) {
		return 0;
	}
	int count = mCount;
	AeSlot* slot = mItems;
	int i;
	for (i = 0; i < count; i++) {
		if (slot->mIndex == id) {
			break;
		}
		slot++;
	}
	if (i == count) {
		return 0;
	}
	return slot;
}

// 0x800A7984
void TAeClip::setSprite(u32 id, AeUv* src, void* tex)
{
	AeSlot* slot = findSlot(id);
	if (slot == 0) {
		return;
	}
	if (tex == 0) {
		tex = mOwner->mAssets;
	}
	slot->_02        = 1;
	((AeUv*)slot->_04)->mVals[0] = src->mVals[0];
	((AeUv*)slot->_04)->mVals[1] = src->mVals[1];
	((AeUv*)slot->_04)->mVals[2] = src->mVals[2];
	((AeUv*)slot->_04)->mVals[3] = src->mVals[3];
	((AeUv*)slot->_04)->mVals[4] = src->mVals[4];
	((AeUv*)slot->_04)->mVals[5] = src->mVals[5];
	((AeUv*)slot->_04)->mVals[6] = src->mVals[6];
	((AeUv*)slot->_04)->mVals[7] = src->mVals[7];
	slot->_14        = tex;
}

// 0x800A7A1C
// A negative index clears the slot
void TAeClip::setFrame(u32 id, int index)
{
	if (index < 0) {
		clearSlot(id);
		return;
	}
	AeFile* file = (AeFile*)mOwner->mData;
	AeUv* src    = (AeUv*)(file->_00 + index * 0x10);
	setSprite(id, src, 0);
}

// 0x800A7A64
// A negative index clears the slot
void TAeClip::setCell(u32 id, int index)
{
	if (index < 0) {
		clearSlot(id);
		return;
	}
	AeSlot* slot = findSlot(id);
	if (slot != 0) {
		slot->_02            = 2;
		*(s16*)slot->_04     = (s16)index;
	}
}

// 0x800A7AB0
void TAeClip::attach(u32 id, TAeClip* other, int flags)
{
	AeSlot* slot = findSlot(id);
	if (slot == 0) {
		return;
	}
	slot->_02              = 4;
	*(u32*)slot->_04       = (u32)flags;
	*(u32*)(slot->_04 + 4) = (u32)other;
}

// 0x800A7AF8
void TAeClip::setHook(u32 id, u32 fn, u32 arg)
{
	AeSlot* slot = findSlot(id);
	if (slot == 0) {
		return;
	}
	slot->_02              = 5;
	*(u32*)slot->_04       = fn;
	*(u32*)(slot->_04 + 4) = arg;
}

// 0x800A7B40
void TAeClip::clearSlot(u32 id)
{
	AeSlot* slot = findSlot(id);
	if (slot != 0) {
		slot->_02 = 0;
	}
}

// 0x800A7B70
void TAeClip::clearSlots()
{
	if (mItems == 0) {
		return;
	}
	int i;
	AeSlot* slot = mItems;
	for (i = 0; i < mCount; i++) {
		slot->_02 = 0;
		slot++;
	}
}

// 0x800A7BA8
void* TAeClip::getAssets() { return mOwner->getAssets(); }

// 0x800A7BCC
u32 TAeClip::getId() { return mId; }

// 0x800A7BD4
// The callers pass a trailing zero that this does not read
void TAeClip::bind(u32 id, TAeClip* other, u32) { setHook(id, (u32)&drawLinked, (u32)other); }

// 0x800A7C00
// Stored in a slot by bind and called with the AE file a record index and the clip
void drawLinked(AeFile* file, int index, void* arg)
{
	TAeClip* clip = (TAeClip*)arg;
	AeFrame frame;
	u8* rec;
	f32 x;
	f32 y;

	clip->fillFrame(&frame);
	njPushMatrixEx();
	njTranslate(0, clip->_50, clip->_54, 0.0f);
	njRotateZ(0, (int)(182.04445f * clip->_5C));
	rec = (u8*)file->mList + index * 0x38;
	x   = *(f32*)(rec + 8);
	y   = *(f32*)(rec + 0xC);
	njTranslate(0, 0.5f * x, 0.5f * y, 0.0f);
	njScale(0, clip->_60, clip->_64, gAeOne[0]);
	rec = (u8*)file->mList + index * 0x38;
	x   = *(f32*)(rec + 8);
	y   = *(f32*)(rec + 0xC);
	njTranslate(0, 0.5f * -x, 0.5f * -y, 0.0f);
	aeDrawGroup(&frame, (s16)clip->mId, clip->mBanks);
	njPopMatrixEx();
}

const char* gTAeLayerName = "TAeLayer";

// 0x800A7CFC
TAeLayer::TAeLayer(TObject* parent, TAeObject* owner, int id, u32 flags)
    : TObject(parent)
    , TAeClip(owner, id, flags)
{
	_00   = (u32)gTAeLayerName;
	mSize = 0xB0;
	_A8   = 0.0f;
	_AC   = 0;
}

// 0x800A7D80
TAeLayer::~TAeLayer() { }

// 0x800A7DF4
// Bit 3 of the clip flags asks for the layer to be deleted once the clip is done
void TAeLayer::update()
{
	step();
	if (isDone() != 0 && (mPlayFlags & 8) != 0) {
		mFlags |= 1;
	}
}

// 0x800A7E58
void TAeLayer::present() { paint(); }

// 0x800A7E84
// Reached from the clip vtable through the thunk at 0x800A80C4
void TAeLayer::onEnd() { mFlags |= 1; } // TObject::mFlags bit 0 = delete me

// 0x800A7E94
int TAeLayer::play()
{
	int done = 0;

	switch (_AC) {
	case 0:
		if (containsTime(_A8) != 0) {
			done = 1;
		} else {
			if (mTime < _A8) {
				setMode(0x10);
				_AC = 1;
			} else if (mTime > _A8) {
				setMode(0x20);
				mPlayFlags &= ~0x500u;
				_AC = 2;
			}
			apply();
		}
		break;
	case 1:
		if (containsTime(_A8) != 0) {
			_AC  = 0;
			done = 1;
			apply();
		} else if (isDone() != 0) {
			done = 1;
			mPlayFlags &= ~0x500u;
			_AC = 0;
			apply();
		} else if (mTime > _A8) {
			setMode(0x20);
			mPlayFlags &= ~0x500u;
			_AC = 2;
			apply();
		} else {
			update();
		}
		break;
	case 2:
		if (containsTime(_A8) != 0) {
			_AC  = 0;
			done = 1;
			apply();
		} else if (isDone() != 0) {
			done = 1;
			mPlayFlags &= ~0x500u;
			_AC = 0;
			apply();
		} else if (mTime < _A8) {
			setMode(0x10);
			mPlayFlags &= ~0x500u;
			_AC = 1;
			apply();
		} else {
			update();
		}
		break;
	}
	return done;
}

// 0x800A80C0
void TAeClip::onEnd() { }
