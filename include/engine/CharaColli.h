#ifndef _CHARACOLLI_H
#define _CHARACOLLI_H

#include "core/types.h"

enum ColliShapeType {
	COLLISHAPE_SPHERE   = 0,
	COLLISHAPE_BOX      = 1,
	COLLISHAPE_CYLINDER = 2,
};

/**
 * @size 0x30
 */
struct ColliShape {
	Vec3f mPos; // _00
	u32 _0C;    // _0C
	int mRotY;  // _10 for a box
	u32 _14;    // _14
	f32 mSizeX; // _18 the radius for a sphere or cylinder
	f32 mSizeY; // _1C the height for a cylinder
	f32 mSizeZ; // _20
	u32 mFlags; // _24 2 touched this frame 8 and 0x10 read by the contact tests
	u32 mType;  // _28 ColliShapeType
	u32 mTag;   // _2C set by colliShapeSetTag
};

// ColliShape as its copies see it the words at 0x0C-0x14 are one nested struct so a whole-shape copy moves them as a
// block colliCopyShapes the TAniFlushFlowerBomb ctor
struct ColliShapeWords {
	u32 w[3];
};
struct ColliShapeCopy {
	Vec3f mPos;
	ColliShapeWords mMid;
	f32 mSizeX;
	f32 mSizeY;
	f32 mSizeZ;
	u32 mFlags;
	u32 mType;
	u32 mTag;
};

struct CharaColli;

// A touch between two colliders in the list at CharaColli's mContacts
struct ColliContact {
	ColliContact* mNext; // _00
	CharaColli* mOther;  // _04 the other collider
	int mOtherShape;     // _08 which of its shapes touched
	Vec3f mPos;          // _0C
	int mShape;          // _18 which of this collider's shapes touched
	Vec3f mPush;         // _1C how far the other collider pushes this one out

	ColliContact() { } // 0x8000A6C8
};

/**
 * @size 0x88
 */
struct CharaColli {
	CharaColli* mPrev;   // _00
	CharaColli* mNext;   // _04
	int mShapeCount;     // _08
	ColliShape mInline;  // _0C used when mShapeCount is 1
	ColliShape* mShapes; // _3C
	u32 mOwner;          // _40 an object id
	ColliShape mBounds;  // _44
	u32 mState;          // _74 1 disabled 2 test mBounds first
	u8 mGroup;           // _78 the bit for what this is
	u8 mFirmMask;        // _79 groups that decide who gets moved in a contact
	u8 mSensorMask;      // _7A groups that are tested in contact mode 0
	u8 mIgnoreMask;      // _7B groups that are never tested
	int mContactCount;   // _7C
	ColliContact* mContacts; // _80
	ColliContact* mLastContact; // _84

	CharaColli();  // 0x800084C4
	~CharaColli(); // 0x80008500
};

/** The colour a collision shape's debug outline is drawn in. @size 0x4 */
struct ColliColor {
	u8 r, g, b, a;
};

#endif
