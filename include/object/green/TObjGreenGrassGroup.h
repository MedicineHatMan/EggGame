#ifndef _TOBJGREENGRASSGROUP_H
#define _TOBJGREENGRASSGROUP_H

#include "stage/SetActor.h"
struct DrawTask;
#include "engine/GrassBlade.h"

enum GrassFlags {
	GRASS_CUT    = 0x1,
	GRASS_NEAR   = 0x2,
	GRASS_SPARSE = 0x4,
};

/** A collision contact on the list at +0x80 of the collision */
struct GrassContact {
	GrassContact* mNext; // _00
	void* mColli;        // _04
	int mShape;          // _08
};

/** A line as njDistanceP2L takes it */
struct GrassLine {
	Vec3f mStart; // _00
	Vec3f mDir;   // _0C
};

class TObjGreenGrassGroup : public TSetActor {
public:
	TObjGreenGrassGroup(TObject* parent, const Vec3f* pos);
	virtual ~TObjGreenGrassGroup();

	inline void operator delete(void* mem);

	virtual void draw();
	virtual void vfn_10();
	virtual void render();

	void makeShape(const Vec3f* pos);
	GrassContact* findContact();
	void createBlades();
	void destroyBlades();
	int isAllCut();
	void resetBlades();
	void pushBlades(f32 radius, const Vec3f* pos);
	void cutBlades(TObjAction* other);
	void bendBlades(TObjAction* other);
	int countUncut();
	void updateBlades();
	void drawBlades();
	void readCamera();
	int hasCameraMoved();
	void sortBlades();

	DrawTask* mDrawTask; // _64
	u8 _68[4];
	GrassBlade* mBlades[20]; // _6C
	u8 mGrassFlags;          // _BC GrassFlags
	Vec3f mHitFrom;          // _C0
	int mOrder[20];          // _CC blade indices in draw order
	Vec3f mCameraPos;        // _11C
	u8 _128[4];
};

typedef char TObjGreenGrassGroup_size_check[(sizeof(TObjGreenGrassGroup) == 0x12C) ? 1 : -1];

extern "C" TObjGreenGrassGroup* createTObjGreenGrassGroup(TObject* parent, Vec3f* pos);

extern "C" {
extern const char* gTObjGreenGrassGroupName;
}

#endif
