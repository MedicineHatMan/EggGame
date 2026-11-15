#include "core/types.h"
#include "stage/Stage.h"
#include "sound/SoundCtrl.h"
#include "sound/SoundHandle.h"

#include "sound/TSoundEffect3DCtrl.h"

// 0x800EE354
extern "C" int sePlay3D(int id, const Vec3f* pos)
{
	if (gSound3D != 0) {
		return gSound3D->play(0, id, 0, 100, pos, 0x7F, 0, 0x78, 600.0f, 0.0f);
	}
	return -1;
}

// 0x800EE3B8
extern "C" int sePlay3DGroup(int id, const Vec3f* pos, int group)
{
	if (gSound3D != 0) {
		return gSound3D->play(0, id, group, 200, pos, 0x7F, 0, 0x78, 600.0f, 0.0f);
	}
	return -1;
}

// 0x800EE420
extern "C" int sePlay3DExtra(int id, const Vec3f* pos, u16 extra, int group)
{
	if (gSound3D != 0) {
		return gSound3D->play(0, id, group, 200, pos, 0x7F, 0, extra, 600.0f, 0.0f);
	}
	return -1;
}

// 0x800EE480
extern "C" int sePlay3DRange(f32 range, int id, const Vec3f* pos, u16 extra)
{
	if (gSound3D != 0) {
		return gSound3D->play(0, id, 0, 100, pos, 0x7F, 0, extra, range, 0.0f);
	}
	return -1;
}

// 0x800EE4E0
extern "C" int sePlay3DHeld(f32 range, SoundHandle* handle, int id, int mode, const Vec3f* pos)
{
	if (gSEMap == 0) {
		return -1;
	}
	int index = id - gSEMap->mBase;
	if (index < 0 || (u32)index >= gSEMap->mCount) {
		return -1;
	}
	switch (gSEMap->mTable[index].mKind) {
	case 0:
		if (gSound3D != 0) {
			return gSound3D->play(handle, index, 0, mode, pos, 0x7F, 0, 0, range, 0.0f);
		}
		break;
	}
	return -1;
}

// 0x800EE588
extern "C" void seStop3D(SoundHandle* handle)
{
	if (gSound3D != 0) {
		gSound3D->stop(handle);
	}
}
