#include "engine/MirrorRipple.h"
#include "stl/math.h"

typedef char MirrorRipple_size_check[(sizeof(MirrorRipple) == 0x2858) ? 1 : -1];

static inline f32 rippleSqrt(f32 x)
{
	return __fres((f32)__frsqrte(x));
}

// 0x80165030
void RippleVert::init(f32 x, f32 y)
{
	mX      = x;
	mY      = y;
	mU      = x;
	mV      = y;
	mDirX   = 0.0f;
	mDirY   = 0.0f;
	mVelX   = 0.0f;
	mVelY   = 0.0f;
	mDist   = 0.0f;
	mMoving = 0;
}

// 0x80165064
void RippleSource::clear()
{
	mPos.x      = 0.0f;
	mPos.y      = 0.0f;
	mRadius     = 0.0f;
	mPrevRadius = 0.0f;
	mMaxRadius  = 0.0f;
	mSpeed      = 0.0f;
	mActive     = 0;
}

// 0x8016508C
void RippleSource::start(const NJS_POINT2* pos, f32 speed, f32 maxRadius)
{
	mPos        = *pos;
	mRadius     = 5.0f;
	mPrevRadius = 0.0f;
	mMaxRadius  = maxRadius;
	mSpeed      = speed;
	mActive     = 1;
}

// 0x801650C0
int RippleSource::isIdle()
{
	return mActive == 0;
}

// 0x801650D0
void RippleSource::step()
{
	if (mActive == 0) {
		return;
	}
	mPrevRadius = mRadius;
	mRadius += mSpeed;
	if (mRadius >= mMaxRadius) {
		mActive = 0;
	}
}

// 0x80165114
MirrorRipple::MirrorRipple()
{
	buildGrid();
	reset();
}

// 0x8016514C
// x from -50 to 50 y from 100 down to 0
void MirrorRipple::buildGrid()
{
	RippleVert* v = mVerts[0];

	for (int row = 0; row < 16; row++) {
		f32 y = 100.0f * (f32)(15 - row) / 15.0f;

		for (int col = 0; col < 16; col++) {
			f32 x = -50.0f;
			x += 100.0f * (f32)col / 15.0f;
			v->init(x, y);
			v++;
		}
	}
}

// 0x80165238
void MirrorRipple::reset()
{
	for (int i = 0; i < 3; i++) {
		mSources[i].clear();
	}
	mBusy = 0;
}

// 0x80165288
// The larger of best and the squared distance from x y to the point
f32 MirrorRipple::farthestSq(f32 x, f32 y, int vert, f32 best)
{
	f32 dx = mVerts[0][vert].mX - x;
	f32 dy = mVerts[0][vert].mY - y;
	f32 d  = dx * dx + dy * dy;

	if (d > best) {
		best = d;
	}
	return best;
}

// 0x801652BC
// Starts a ring at x y that runs out to the farthest corner if one is free
void MirrorRipple::touch(f32 x, f32 y)
{
	RippleSource* src;
	int i;

	for (i = 0; i < 3; i++) {
		src = &mSources[i];

		if (src->isIdle()) {
			NJS_POINT2 at;
			f32 reach;

			reach = farthestSq(x, y, 0, 0.0f);
			reach = farthestSq(x, y, 0xF, reach);
			reach = farthestSq(x, y, 0xF0, reach);
			reach = farthestSq(x, y, 0xFF, reach);
			reach = rippleSqrt(reach);
			if (reach > 200.0f) {
				reach = 200.0f;
			}
			at.x = x;
			at.y = y;
			src->start(&at, 2.0f, reach);
			mBusy = 1;
			return;
		}
	}
}

/** Pushes a point straight away from x y */
static inline void pushVert(RippleVert* v, f32 x, f32 y, f32 strength)
{
	f32 dy = v->mY - y;
	f32 dx = v->mX - x;
	f32 k  = strength * (f32)__frsqrte(dx * dx + dy * dy);

	v->mVelX += dx * k;
	v->mVelY += dy * k;
	v->mMoving = 1;
}

// 0x801653DC
// Pushes every point between the two circles about the source
void MirrorRipple::pushRing(const RippleSource* src, f32 outer, f32 inner, f32 strength)
{
	f32 y;
	f32 x;
	f32 width;
	f32 down;
	f32 left;
	x     = src->mPos.x;
	left  = x - mVerts[0][0].mX;
	y     = src->mPos.y;
	width = mVerts[0][15].mX - mVerts[0][0].mX;
	down  = -(y - mVerts[0][0].mY);
	f32 colScale;
	f32 height;
	f32 rowScale;
	f32 rowStep;
	int row;
	int rowEnd;

	if (left - outer > width || left + outer < 0.0f) {
		return;
	}
	colScale = 15.0f / width;
	height   = mVerts[0][0].mY - mVerts[15][0].mY;
	if (down - outer > height || down + outer < 0.0f) {
		return;
	}
	rowScale = 15.0f / height;
	rowStep  = __fres(rowScale);
	row      = (int)(f32)ceil((down - outer) * rowScale);
	if (row < 0) {
		row = 0;
	}
	rowEnd = (int)(f32)floor((down + outer) * rowScale);
	if (rowEnd >= 16) {
		rowEnd = 15;
	}
	outer *= outer;
	inner *= inner;
	for (; row <= rowEnd; row++) {
		f32 dy = (f32)row * rowStep - down;
		f32 half;
		int colStart;
		int colEnd;
		int innerStart;
		int innerEnd;
		int col;

		dy *= dy;
		half     = rippleSqrt(outer - dy);
		colStart = (int)(f32)ceil(colScale * (left - half));
		if (colStart < 0) {
			colStart = 0;
		}
		colEnd = (int)(f32)floor(colScale * (left + half));
		if (colEnd >= 16) {
			colEnd = 15;
		}
		if (dy > inner) {
			innerStart = colEnd + 1;
			innerEnd   = innerStart;
		} else {
			half       = rippleSqrt(inner - dy);
			innerStart = (int)(f32)ceil(colScale * (left - half));
			innerEnd   = (int)(f32)floor(colScale * (left + half));
		}
		for (col = colStart; col < innerStart; col++) {
			pushVert(&mVerts[row][col], x, y, strength);
		}
		for (col = innerEnd + 1; col <= colEnd; col++) {
			pushVert(&mVerts[row][col], x, y, strength);
		}
	}
}

// 0x80165780
int MirrorRipple::isCalm()
{
	return mBusy == 0;
}

// 0x80165790
// Grows the rings 1 while any is still going
int MirrorRipple::spread()
{
	RippleSource* src;
	int going;
	int i;

	going = 0;

	for (i = 0; i < 3; i++) {
		src = &mSources[i];

		if (!src->isIdle()) {
			going = 1;
			src->step();
			pushRing(src, src->mRadius, src->mPrevRadius, 2.5f);
		}
	}
	return going;
}

// 0x80165810
// Springs each moving point back
int MirrorRipple::settle()
{
	RippleVert* v = mVerts[0];
	int moving    = 0;

	for (int i = 0; i < 256; i++) {
		if ((u32)v->mMoving != 0) {
			f32 dx;
			f32 dy;
			f32 dist;
			f32 newU;
			f32 newV;
			f32 velX;
			f32 velY;
			f32 homeX;
			f32 homeY;
			f32 pull;
			f32 inv;

			moving = 1;
			pull   = v->mDist;
			pull *= -0.2f;
			velY = v->mVelY;
			velX = v->mVelX;
			velY += v->mDirY * pull;
			velX += v->mDirX * pull;
			velY *= 0.945f;
			velX *= 0.945f;
			newV  = v->mV;
			homeY = v->mY;
			newU  = v->mU;
			homeX = v->mX;
			newV += velY;
			newU += velX;
			dy   = newV - homeY;
			dx   = newU - homeX;
			dist = __fres((f32)__frsqrte(dx * dx + dy * dy));
			inv  = __fres(dist);
			dx *= inv;
			dy *= inv;
			if (dist <= 0.05f && velX * velX + velY * velY <= 0.0025000002f) {
				v->mMoving = 0;
				newU       = homeX;
				newV       = homeY;
				dist = velY = velX = dy = dx = 0.0f;
			}
			v->mU    = newU;
			v->mV    = newV;
			v->mDirX = dx;
			v->mDirY = dy;
			v->mVelX = velX;
			v->mVelY = velY;
			v->mDist = dist;
		}
		v++;
	}
	return moving;
}

// 0x80165914
void MirrorRipple::update()
{
	if (mBusy != 0) {
		int spreading = spread();
		int settling  = settle();

		if ((spreading | settling) == 0) {
			mBusy = 0;
		}
	}
}
