#include "stage/Stage.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "Ninja/ninja.h"
#include "core/PlacementNew.h"
#include "core/MathUtil.h"
#include "object/TObjEggRing.h"
#include "engine/FxSystem.h"
#include "stl/math.h"
#include "core/VecMath.h"
#include "object/green/TObjGreenHoverLeaf.h"
#include "core/TMainTask.h"
struct Fx;

typedef char TObjGreenHoverLeaf_size_check[(sizeof(TObjGreenHoverLeaf) == 0x58) ? 1 : -1];

// 0x8016FB8C
extern "C" int createTObjGreenHoverLeaf(s8* arg)
{
	new TObjGreenHoverLeaf(gTL_07, *arg);
	return 1;
}

// 0x8016FBDC
TObjGreenHoverLeaf::TObjGreenHoverLeaf(TObject* parent, int)
    : TObject(parent)
{
	_00   = gTObjGreenHoverLeafName;
	mSize = 0x58;
	mtxIdentity(mMatrix);
	mFx = fxSpawnFollow(1, 8, 1, mMatrix);
}

// 0x8016FC48
TObjGreenHoverLeaf::~TObjGreenHoverLeaf()
{
	fxStop((Fx*)mFx);
}

// 0x8016FCB0
// The two angles and the matrix are the rotation that takes the Z axis to dir though dir is always that axis
void TObjGreenHoverLeaf::draw()
{
	f32 m[12];
	Vec3f pos;
	Vec3f at;
	Vec3f dir;
	f32 sinA;
	f32 cosA;
	f32 sinB;
	f32 cosB;

	mtxIdentity(m);
	camGetDrawPos(&pos);
	camGetDrawFlat(&dir);
	vecScaleAdd(100.0f, &pos, &dir, &pos);
	if (mFx == 0) {
		return;
	}

	dir.x = 0.0f;
	dir.y = 0.0f;
	dir.z = 1.0f;
	if (0.0f == vecNormalize(&dir, &dir)) {
		dir.x = 0.0f;
		dir.y = 0.0f;
		dir.z = 1.0f;
	}

	njSinCos((int)(10430.38043493439 * (f32)atan2(dir.y, dir.z)), &sinA, &cosA);
	njSinCos((int)(10430.38043493439 * (f32)atan2(dir.x, dir.y * sinA + dir.z * cosA)), &sinB, &cosB);

	m[0]  = cosB;
	m[1]  = 0.0f;
	m[2]  = sinB;
	m[4]  = -sinA * sinB;
	m[5]  = cosA;
	m[6]  = sinA * cosB;
	m[8]  = -cosA * sinB;
	m[9]  = -sinA;
	m[10] = cosA * cosB;

	vecScaleAdd(-100.0f, &at, &dir, &pos);
	m[3]  = at.x;
	m[7]  = 50.0f + at.y;
	m[11] = at.z;
	mtxCopy(mMatrix, m);
}
