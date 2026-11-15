#include "ui/TAeLayer.h"
#include "Ninja/ninja.h"
#include "ui/AeDraw.h"
#include "ui/AeArc.h"
#include "core/MathUtil.h"

struct AeXform;

/**
 * Where one record puts its children at the frame's time. Names are ours.
 *
 * @size 0x20
 */
struct AeXform {
	AeRecord* mRecord; // _00
	f32 mPosX;         // _04
	f32 mPosY;         // _08
	f32 mScaleX;       // _0C
	f32 mScaleY;       // _10
	f32 mAnchorX;      // _14
	f32 mAnchorY;      // _18
	f32 mSpin;         // _1C degrees
};

// 0x800A659C
// Widens [*start *end] by the times of every sprite and slot under record id with each group's window clipped to its
// parent's
void aeScanRange(void* list, u32 id, f32* start, f32* end, f32 base, f32 lo, f32 hi)
{
	AeRecord* rec = &((AeRecord*)list)[(u16)id];
	for (; rec->mKind != 0; rec++) {
		f32 at   = base + rec->mDelay / rec->mRate;
		f32 from = at + rec->mStart / rec->mRate;
		f32 to   = at + rec->mEnd / rec->mRate;
		if (from < lo) {
			from = lo;
		}
		if (to > hi) {
			to = hi;
		}
		switch (rec->mKind) {
		case 1:
		case 3:
			if (from < *start) {
				*start = from;
			}
			if (to > *end) {
				*end = to;
			}
			break;
		case 2:
			aeScanRange(list, rec->mChild, start, end, at, from, to);
			break;
		}
	}
}

// 0x800A66C0
void aeGetRange(void* list, u32 id, f32* start, f32* end)
{
	*start = 65536.0f;
	*end   = -65536.0f;
	aeScanRange(list, id, start, end, 0.0f, 0.0f, 65536.0f);
}

// 0x800A66F8
// Moves to the position spins and scales about the anchor and multiplies the result onto m NULL for the current matrix
void aeMultXform(f32* m, AeXform* xform)
{
	f32 mtx[12];
	AeRecord* rec = xform->mRecord;
	f32 cx        = rec->mSizeX / 2.0f;
	f32 cy        = rec->mSizeY / 2.0f;
	mtxSetTrans((float*)mtx, xform->mPosX, xform->mPosY, 0.0f);
	mtxTranslate((float*)mtx, (const float*)mtx, cx, cy, 0.0f);
	mtxRotateZ(mtx, mtx, (int)(182.04445f * xform->mSpin));
	mtxScale(xform->mScaleX, xform->mScaleY, 0.0f, mtx, mtx);
	mtxTranslate((float*)mtx, (const float*)mtx, -xform->mAnchorX, -xform->mAnchorY, 0.0f);
	mtxTranslate((float*)mtx, (const float*)mtx, -cx, -cy, 0.0f);
	njMultiMatrix(m, mtx);
}

// 0x800A6808
// Reads a list of {time x y z} at time blending between the two keys around it
void aeKey3(f32 time, f32* x, f32* y, f32* z, f32* keys)
{
	if (keys == NULL) {
		return;
	}
	f32* prev = NULL;
	while (true) {
		if (keys[0] < -0.5f) {
			if (prev == NULL) {
				return;
			}
			*x = prev[1];
			*y = prev[2];
			*z = prev[3];
			return;
		}
		if (time <= keys[0]) {
			if (prev == NULL) {
				*x = keys[1];
				*y = keys[2];
				*z = keys[3];
				return;
			}
			f32 t = (time - prev[0]) / (keys[0] - prev[0]);
			*x    = t * (keys[1] - prev[1]) + prev[1];
			*y    = t * (keys[2] - prev[2]) + prev[2];
			*z    = t * (keys[3] - prev[3]) + prev[3];
			return;
		}
		prev = keys;
		keys += 4;
	}
}

// 0x800A68D8
// aeKey3 for {time x y}
void aeKey2(f32 time, f32* x, f32* y, f32* keys)
{
	if (keys == NULL) {
		return;
	}
	f32* prev = NULL;
	while (true) {
		if (keys[0] < -0.5f) {
			if (prev == NULL) {
				return;
			}
			*x = prev[1];
			*y = prev[2];
			return;
		}
		if (time <= keys[0]) {
			if (prev == NULL) {
				*x = keys[1];
				*y = keys[2];
				return;
			}
			f32 t = (time - prev[0]) / (keys[0] - prev[0]);
			*x    = t * (keys[1] - prev[1]) + prev[1];
			*y    = t * (keys[2] - prev[2]) + prev[2];
			return;
		}
		prev = keys;
		keys += 3;
	}
}

// 0x800A6984
// aeKey3 for {time x}
void aeKey1(f32 time, f32* x, f32* keys)
{
	if (keys == NULL) {
		return;
	}
	f32* prev = NULL;
	while (true) {
		if (keys[0] < -0.5f) {
			if (prev == NULL) {
				return;
			}
			*x = prev[1];
			return;
		}
		if (time <= keys[0]) {
			if (prev == NULL) {
				*x = keys[1];
				return;
			}
			f32 t = (time - prev[0]) / (keys[0] - prev[0]);
			*x    = t * (keys[1] - prev[1]) + prev[1];
			return;
		}
		prev = keys;
		keys += 2;
	}
}

// 0x800A6A0C
// Applies record index's keys at the frame's time the transform onto the current matrix alpha and colour onto the frame
void aeApplyRecord(AeFrame* frame, int index)
{
	f32 time      = frame->mTime;
	AeRecord* rec = &((AeRecord*)frame->mList)[index];
	AeXform xform;
	xform.mRecord  = rec;
	xform.mPosX    = 0.0f;
	xform.mPosY    = 0.0f;
	xform.mScaleX  = 1.0f;
	xform.mScaleY  = 1.0f;
	xform.mAnchorX = 128.0f;
	xform.mAnchorY = 128.0f;
	xform.mSpin    = 0.0f;
	aeKey2(time, &xform.mAnchorX, &xform.mAnchorY, (f32*)rec->mOff20);
	aeKey2(time, &xform.mPosX, &xform.mPosY, (f32*)rec->mOff24);
	aeKey2(time, &xform.mScaleX, &xform.mScaleY, (f32*)rec->mOff28);
	aeKey1(time, &xform.mSpin, (f32*)rec->mOff2C);
	aeMultXform(NULL, &xform);

	f32 alpha = 1.0f;
	aeKey1(time, &alpha, (f32*)rec->mOff30);
	frame->mColor.a *= alpha;

	f32 r = 1.0f;
	f32 g = r;
	f32 b = r;
	aeKey3(time, &r, &g, &b, (f32*)rec->mOff34);
	frame->mColor.r *= r;
	frame->mColor.g *= g;
	frame->mColor.b *= b;
}

// 0x800A6B6C
// The frame's slot for record id or NULL when it is missing or empty
AeSlot* aeFindSlot(AeFrame* frame, u32 id)
{
	int i;
	AeSlot* slot = frame->mItems;
	if (slot == NULL) {
		return NULL;
	}
	for (i = 0; i < frame->mCount; i++) {
		if ((u16)slot->mIndex == id) {
			break;
		}
		slot++;
	}
	if (i == frame->mCount) {
		return NULL;
	}
	if ((u16)slot->_02 == 0) {
		return NULL;
	}
	return slot;
}

// 0x800A6BD8
// Draws every record of the group at index that is live at the frame's time
void aeDrawGroup(AeFrame* frame, int index, const u32* banks)
{
	AeRecord* rec = &((AeRecord*)frame->mList)[(s16)index];
	while (rec->mKind != 0) {
		f32 time = (frame->mTime - rec->mDelay / rec->mRate) * rec->mRate;
		if (time + 0.0001f < rec->mStart || time + 0.0001f >= rec->mEnd) {
			index++;
			rec = &((AeRecord*)frame->mList)[(s16)index];
			continue;
		}
		AeFrame sub;
		sub.mHead   = frame->mHead;
		sub.mList   = frame->mList;
		sub.mTime   = frame->mTime;
		sub.mRate   = frame->mRate;
		sub.mMark   = frame->mMark;
		sub.mColor  = frame->mColor;
		sub.mCount  = frame->mCount;
		sub.mItems  = frame->mItems;
		sub.mTime   = time;
		sub.mMark   = frame->mMark | rec->mFlags;
		njPushMatrixEx();
		aeApplyRecord(&sub, (s16)index);
		switch (rec->mKind) {
		case 1:
			sub.mHead += rec->mChild * sizeof(u32[4]);
			aeDrawSprite(&sub, banks);
			break;
		case 2:
			aeDrawGroup(&sub, (s16)rec->mChild, banks);
			break;
		case 3: {
			AeSlot* slot = aeFindSlot(&sub, (s16)index);
			if (slot == NULL) {
				break;
			}
			switch (slot->_02) {
			case 1: {
				NJS_TEXLIST* tex = njGetCurrentTexList();
				njSetTexture(slot->_14);
				sub.mHead = (u32)slot->_04;
				aeDrawSprite(&sub, banks);
				njSetTexture(tex);
				break;
			}
			case 2:
				aeDrawGroup(&sub, *(s16*)slot->_04, banks);
				break;
			case 4: {
				TAeClip* clip    = *(TAeClip**)(slot->_04 + 4);
				NJS_TEXLIST* tex = njGetCurrentTexList();
				AeFrame inner;
				clip->fillFrame(&inner);
				sub.mHead  = inner.mHead;
				sub.mList  = inner.mList;
				sub.mColor.r    = inner.mColor.r;
				sub.mColor.g    = inner.mColor.g;
				sub.mColor.b    = inner.mColor.b;
				sub.mCount = inner.mCount;
				sub.mItems = inner.mItems;
				if (*(u32*)slot->_04 == 0) {
					sub.mTime = inner.mTime;
				}
				njSetTexture(clip->getAssets());
				aeDrawGroup(&sub, (s16)clip->getId(), banks);
				njSetTexture(tex);
				break;
			}
			case 5: {
				NJS_TEXLIST* tex = njGetCurrentTexList();
				(*(void (**)(AeFrame*, int, u32))slot->_04)(&sub, (s16)index, *(u32*)(slot->_04 + 4));
				njSetTexture(tex);
				break;
			}
			}
			break;
		}
		}
		index++;
		rec = &((AeRecord*)frame->mList)[(s16)index];
		njPopMatrixEx();
	}
}

// 0x800A6EC8
// Adds the index of every slot record under index to ids once each
void aeCollectSlots(void* list, s16 index, s16* ids, int max, int* count)
{
	AeRecord* rec = &((AeRecord*)list)[index];
	for (; rec->mKind != 0; rec++, index++) {
		switch (rec->mKind) {
		case 2:
			aeCollectSlots(list, rec->mChild, ids, max, count);
			break;
		case 3: {
			int n = *count;
			int i;
			for (i = 0; i < n; i++) {
				if (index == ids[i]) {
					break;
				}
			}
			if (i == n) {
				ids[n] = index;
				(*count)++;
			}
			break;
		}
		}
	}
}
