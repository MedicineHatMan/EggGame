#ifndef _TOBJGREENGRASSFIELD_H
#define _TOBJGREENGRASSFIELD_H

#include "object/green/TObjGreenGrassGroup.h"
#include "egg/ObjEgg.h"

class TObjGreenGrassField : public TSetActor {
public:
	TObjGreenGrassField(TObject* parent);
	virtual ~TObjGreenGrassField();

	inline void operator delete(void* mem);

	void placeGroups();
	void placeGroup(f32 a, f32 b, int sector, int k);
	void resetGroups();
	void pushGroups(f32 radius, const Vec3f* pos);
	Vec3f* getGroupPos(int index);
	TObjGreenGrassGroup* getGroup(int index);
	int findFarOuter(const Vec3f* pos, int exclude, int percent);
	int findFarNeighbor(const Vec3f* pos, int index, int exclude, int percent);
	int hasGrass(int index, int percent);
	int sectorHasGrassNear(int start, const Vec3f* pos, int percent);
	int hasGrassNear(const Vec3f* pos, int percent);
	void setNear();
	void clearNear();

	TObjGreenGrassGroup* mGroups[55]; // _64
	HitVec _140[55];                  // _140 never used
};

typedef char TObjGreenGrassField_size_check[(sizeof(TObjGreenGrassField) == 0x3D4) ? 1 : -1];

extern "C" TObjGreenGrassField* createTObjGreenGrassField(TObject* parent);

struct Vec3f;

#endif
