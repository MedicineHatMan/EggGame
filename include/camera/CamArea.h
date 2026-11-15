#ifndef _CAMAREA_H
#define _CAMAREA_H

#include "core/types.h"
#include "io/ResFile.h"
#include "stage/Stage.h"

class TCamera;

/** A point look's or a watch's parameters */
struct CamLookParam {
	f32 mNear;     // _00
	Vec3f mAt;     // _04
	f32 mDistance; // _10
	f32 _14[3];
};

/** A path's or a start's parameters */
struct CamPathParam {
	Vec3f mPos;   // _00
	Vec3f mDir;   // _0C
	f32 _18[2];
};

/**
 * One zone. A shape around mPos; the outer bounds are the shape plus 50 on every side and are tested first, a few
 * zones a frame.
 *
 * @size 0x80
 */
struct CamArea {
	void init();                             // 0x800A8D00
	int isEnd();                             // 0x800A8D94 an empty kind ends the table
	void prepare();                          // 0x800A8DA4
	void getPoint(Vec3f* out) const;               // 0x800A8E4C
	void getLine(Vec3f* pos, Vec3f* line) const;   // 0x800A8E68
	void getWatch(Vec3f* out) const;               // 0x800A8E98
	void getPath(Vec3f* pos, Vec3f* dir) const;    // 0x800A8EB4

	s16 mType;        // _00 0 = end 1 = settings 2 = point look 3 = vertical line 4 = watch 5 = path 6 = start
	s16 mPriority;    // _02 the highest wins for a start the player
	f32 mFar;         // _04 the far clip given to the camera on entering
	f32 mFarBack;     // _08 the far clip given back on leaving
	union {
		f32 mParam[8];    // _0C read differently by each kind
		CamLookParam mLook; // _0C point looks and watches
		CamPathParam mPath; // _0C paths and starts
	};
	u32 mFlags;       // _2C 1 = drop when the player rides 2/4/8/0x10 = which settings 0x20 = keep the far clip
	Vec3f mPos;       // _30
	int mAngle;       // _3C
	int mShape;       // _40 0 = sphere 1 = cylinder 2 = box
	f32 mRadius;      // _44 for a box the half width
	f32 mHeight;      // _48 half
	f32 mDepth;       // _4C the box's half depth
	u32 mSide;        // _50 bit 0 only entered from behind its facing
	f32 mOuterRadius; // _54
	f32 mOuterHeight; // _58
	f32 mSin;         // _5C of mAngle
	f32 mCos;         // _60
	int _64[7];       // _64
};

/**
 * Which zone each player is in and the camera it made. TCameraCtrl holds one.
 *
 * @size 0x128
 */
class CamAreaCtrl {
public:
	CamAreaCtrl();  // 0x800A8308
	~CamAreaCtrl(); // 0x800A8374

	void testOuter(CamArea* area, Vec3f* pos, u8* state); // 0x800A83B0
	void testInner(CamArea* area, Vec3f* pos, u8* state); // 0x800A8454
	void leave(int player);                               // 0x800A85FC
	void enter(int player, int area);                     // 0x800A86AC
	void applySettings(int player, CamArea* area);        // 0x800A8884
	void checkRide(int player);                           // 0x800A89B4
	void update();                                        // 0x800A8A74

	int mScan;            // _00 the first zone of the next outer test
	u8 mState[4][64];     // _04 per player and zone 1 = near 2 = inside 4 = may be picked
	int mArea[4];         // _104 the zone each player is in -1 = none
	TCamera* mCamera[4];  // _114 the camera the zone made
	int mFirst;           // _124 test every zone on the next update
};

typedef char CamAreaCtrl_size_check[(sizeof(CamAreaCtrl) == 0x128) ? 1 : -1];

extern "C" {
extern BinFile* gCamAreaFile; // 0x802C5148
extern CamArea* gCamAreas;        // 0x802C514C
extern int gCamAreaCount;         // 0x802C5150

void clearCamAreas();                                  // 0x800A80D4
void loadCamAreas(const char* name);                   // 0x800A80E8 0 makes 64 empty zones
void freeCamAreas();                                   // 0x800A8274
CamAreaCtrl* createCamAreaCtrl();                      // 0x800A82D0
int countCamAreas();                                   // 0x800A8CA4
int findCamStart(int player, Vec3f* pos, Vec3f* dir);  // 0x800A8EE8 0 when there is none
}

#endif
