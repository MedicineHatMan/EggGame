#ifndef _FLAGCLOTH_H
#define _FLAGCLOTH_H

#include "engine/SpringBody.h"

struct ClothMatrix {
	f32 m[12];
};

/**
 * A grid of points pulled together by springs, sideways, up and across. Vtable 0x80258B98.
 *
 * @size 0xB8
 */
class FlagCloth : public SpringBody {
public:
	FlagCloth(ClothMatrix base, u16 cols, u16 rows, f32 width, f32 height, f32 spring, f32 loss, f32 param); // 0x800A98C4

	void build(const ClothMatrix* base, u16 cols, u16 rows, f32 width, f32 height); // 0x800A9A0C
	void rebuild(const ClothMatrix* base);                                         // 0x800A9F54
	void pin(u16 x, u16 y);                                                        // 0x800AA0BC
	void setWind(const Vec3f* dir, s16 speed, f32 strength);                       // 0x800AA0F4
	SpringPoint* point(u16 x, u16 y);                                              // 0x800AA1B0

	void addLink(u32 from, u32 to, f32 rest)
	{
		mPoints[from].mLink[mPoints[from].mLinkCount] = &mPoints[to];
		mPoints[from].mRest[mPoints[from].mLinkCount] = rest;
		mPoints[from].mLinkCount++;
	}

	virtual void getForce(u16 index, Vec3f* out); // _08 0x800AA080 the gust the same for every point
	virtual void collide();                       // _0C 0x800AA09C
	virtual ~FlagCloth();                         // _10 0x800A99B4
	virtual void step();                          // _14 0x800A9FB0
	virtual void reset();                         // _18 0x800A9F30

	ClothMatrix mBase; // _50
	f32 mWidth;        // _80
	f32 mHeight;       // _84
	u16 mCols;         // _88 points across
	u16 mRows;         // _8A points down
	Vec3f mWindDir;    // _8C
	Vec3f mGust;       // _98
	Vec3f mSway;       // _A4
	s16 mSwaySpeed;    // _B0
	s16 mSwayStep;     // _B2
	s16 mSwayAngle;    // _B4
};

typedef char FlagCloth_size_check[(sizeof(FlagCloth) == 0xB8) ? 1 : -1];

#endif
