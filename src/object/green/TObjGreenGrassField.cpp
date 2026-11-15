#include "object/green/TObjGreenGrassField.h"
#include "object/ObjAction.h"
#include "Ninja/ninja.h"
#include "core/MathUtil.h"
#include "camera/TCamera.h"
#include "core/VecMath.h"

inline void TObjGreenGrassField::operator delete(void* mem)
{
	objHeapFree(gObjHeap, mem);
}

// 0x8017FD60
extern "C" TObjGreenGrassField* createTObjGreenGrassField(TObject* parent)
{
	return new TObjGreenGrassField(parent);
}

// 0x8017FDAC
TObjGreenGrassField::TObjGreenGrassField(TObject* parent)
    : TSetActor(parent)
{
	placeGroups();
}

// 0x8017FE0C
TObjGreenGrassField::~TObjGreenGrassField()
{
}

// 0x8017FE68
void TObjGreenGrassField::placeGroups()
{
	Vec3f center;
	center.x   = 0.0f;
	center.y   = 25.0f;
	center.z   = 0.0f;
	mGroups[0] = createTObjGreenGrassGroup(this, &center);
	for (int i = 0; i < 6; i++) {
		placeGroup(40.0f, 0.0f, i, 0);
	}
}

// 0x8017FED8
// Each clump places the next ones out the first three of a sector branch in two the next three step once along the
// sector's far edge
void TObjGreenGrassField::placeGroup(f32 a, f32 b, int sector, int k)
{
	if (k < 2) {
		placeGroup(a + 40.0f, b, sector, k * 2 + 1);
		placeGroup(a, b + 40.0f, sector, k * 2 + 2);
	} else if (k < 6) {
		placeGroup(a, b + 40.0f, sector, k + 3);
	}

	u32 next  = 3932100.0f * (sector + 1) / 360.0f;
	f32 side  = b * njSin(next);
	u32 angle = 3932100.0f * sector / 360.0f;
	f32 z     = a * njSin(angle) + side;
	b         = b * njCos(next);

	Vec3f pos;
	pos.x                      = a * njCos(angle) + b;
	pos.y                      = 25.0f;
	pos.z                      = z;
	TObjGreenGrassGroup* group = createTObjGreenGrassGroup(this, &pos);
	int index                  = sector * 9 + 1;
	index                      = k + index;
	mGroups[index]             = group;
}

// 0x8018005C
void TObjGreenGrassField::resetGroups()
{
	for (int i = 0; i < 55; i++) {
		mGroups[i]->resetBlades();
	}
}

// 0x801800A8
void TObjGreenGrassField::pushGroups(f32 radius, const Vec3f* pos)
{
	for (int i = 0; i < 55; i++) {
		mGroups[i]->pushBlades(radius, pos);
	}
}

// 0x8018010C
Vec3f* TObjGreenGrassField::getGroupPos(int index)
{
	return mGroups[index]->getPos();
}

// 0x80180144
TObjGreenGrassGroup* TObjGreenGrassField::getGroup(int index)
{
	return mGroups[index];
}

// 0x80180154
// Of the six outer corners the one farthest from pos that still has grass
int TObjGreenGrassField::findFarOuter(const Vec3f* pos, int exclude, int percent)
{
	int corners[6] = { 5, 14, 23, 32, 41, 50 };
	int i;
	int index;
	f32 dist;
	f32 best  = 0.0f;
	int found = 0;

	for (i = 0; i < 6; i++) {
		index = corners[i];
		if (index != exclude) {
			dist = vecDistanceSq(pos, getGroupPos(index));
			if (dist > best && hasGrass(index, percent) != 0) {
				best  = dist;
				found = index;
			}
		}
	}
	if (found == 0 && hasGrass(exclude, percent) != 0) {
		found = exclude;
	}
	return found;
}

// 0x80180270
// A neighbor of index the farthest from pos with grass
int TObjGreenGrassField::findFarNeighbor(const Vec3f* pos, int index, int exclude, int percent)
{
	int near[3]   = { 0, 9, 45 };
	int across[2] = { 18, 37 };
	int i;
	f32 dist;
	f32 best  = 0.0f;
	int found = index;
	int other;

	for (i = 0; i < 3; i++) {
		other = index + near[i];
		if (other == index) {
			other = 0;
		} else if (other > 55) {
			other = other % 55 + 1;
		}
		if (other != exclude) {
			dist = vecDistanceSq(pos, getGroupPos(other));
			if (dist > best && hasGrass(other, percent) != 0) {
				best  = dist;
				found = other;
			}
		}
	}
	if (found != index) {
		return found;
	}

	for (i = 0; i < 2; i++) {
		other = index + across[i];
		if (other > 55) {
			other = other % 55 + 1;
		}
		if (other != exclude) {
			dist = vecDistanceSq(pos, getGroupPos(other));
			if (dist > best && hasGrass(other, percent) != 0) {
				best  = dist;
				found = other;
			}
		}
	}
	if (found != index) {
		return found;
	}

	other = index + 27;
	if (other > 55) {
		other = other % 55 + 1;
	}
	if (hasGrass(other, percent) != 0) {
		return other;
	}
	if (hasGrass(exclude, percent) != 0) {
		return exclude;
	}
	return index;
}

// 0x801804A8
// Does the clump have at least percent of its 20 blades standing
int TObjGreenGrassField::hasGrass(int index, int percent)
{
	return getGroup(index)->countUncut() > percent * 20 / 100;
}

// 0x801804F8
// Is there grass in one of the nine clumps from start that are within 20 of pos
int TObjGreenGrassField::sectorHasGrassNear(int start, const Vec3f* pos, int percent)
{
	for (int i = 0; i < 9; i++) {
		int index = start + i;
		if (distXZ(getGroupPos(index), pos) <= 20.0f && hasGrass(index, percent) != 0) {
			return 1;
		}
	}
	return 0;
}

// 0x80180598
// Is there grass under pos
int TObjGreenGrassField::hasGrassNear(const Vec3f* pos, int percent)
{
	int start;
	int i;
	int result;
	Vec3f* center = getGroupPos(0);
	int starts[6] = { 1, 10, 19, 28, 37, 46 };
	result        = 0;

	if (distXZ(center, pos) <= 20.0f) {
		result = hasGrass(0, percent);
	} else {
		for (i = 0; i < 6; i++) {
			start         = starts[i];
			u16 angle     = angleToXZ(center, getGroupPos(start));
			u16 nextAngle = angleToXZ(center, getGroupPos(starts[(i + 1) % 6]));
			u16 posAngle  = angleToXZ(center, pos);
			if ((int)angle >= (int)nextAngle) {
				if ((int)posAngle >= (int)angle && (int)posAngle <= (int)nextAngle) {
					result = sectorHasGrassNear(start, pos, percent);
				}
			} else if ((int)posAngle <= (int)angle || (int)posAngle >= (int)nextAngle) {
				result = sectorHasGrassNear(start, pos, percent);
			}
		}
	}
	return result;
}

// 0x80180718
void TObjGreenGrassField::setNear()
{
	for (int i = 0; i < 55; i++) {
		mGroups[i]->mGrassFlags |= GRASS_NEAR;
	}
}

// 0x80180744
void TObjGreenGrassField::clearNear()
{
	for (int i = 0; i < 55; i++) {
		mGroups[i]->mGrassFlags &= ~GRASS_NEAR;
	}
}
