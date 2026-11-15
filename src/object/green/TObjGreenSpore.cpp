#include "stage/Stage.h"
#include "object/ObjAction.h"
#include "camera/TCameraCtrl.h"
#include "core/PlacementNew.h"
#include "core/MathUtil.h"
#include "engine/FxSystem.h"
#include "core/VecMath.h"
#include "object/green/TObjGreenSpore.h"
#include "core/TMainTask.h"
struct Fx;

typedef char TObjGreenSpore_size_check[(sizeof(TObjGreenSpore) == 0x58) ? 1 : -1];

// 0x801985D0
extern "C" int createTObjGreenSpore(s8* arg)
{
	new TObjGreenSpore(gTL_07, *arg);
	return 1;
}

// 0x80198620
TObjGreenSpore::TObjGreenSpore(TObject* parent, int)
    : TObject(parent)
{
	_00   = gTObjGreenSporeName;
	mSize = 0x58;
	mtxIdentity(mMatrix);
	mFx = fxSpawnFollow(1, 8, 0, mMatrix);
}

// 0x8019868C
TObjGreenSpore::~TObjGreenSpore()
{
	fxStop((Fx*)mFx);
}

// 0x801986F4
void TObjGreenSpore::draw()
{
	Vec3f pos;
	Vec3f dir;

	camGetDrawPos(&pos);
	camGetDrawFlat(&dir);
	vecScaleAdd(100.0f, &pos, &dir, &pos);
	mtxSetTrans((float*)mMatrix, pos.x, pos.y, pos.z);
}
