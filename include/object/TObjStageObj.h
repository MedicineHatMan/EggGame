#ifndef _OBJECT_TOBJSTAGEOBJ_H_
#define _OBJECT_TOBJSTAGEOBJ_H_
#include "enemy/EnemyLight.h"
#include "engine/ModelDraw.h"
#include "object/ObjModel.h"
#include "stage/SetActor.h"
#include "stage/Stage.h"
struct DrawTask;
struct MeshObject;
struct ShatterBank;
struct StageHitInfo;
struct StageModelInfo;
struct StageShapeInfo;
struct TObjAction;
struct TObjStageObj;
struct SetDesc;
struct TObject;

#include "core/types.h"

extern "C" {
TObject* createTObjStageObj(TObject*, SetDesc*);
}

/** One collision shape in StageModelInfo */
struct StageShapeInfo {
	int mType;  // _00 0 sphere 1 box 2 cylinder
	Vec3f mPos; // _04
	f32 mSizeX; // _10
	f32 mSizeY; // _14
	f32 mSizeZ; // _18
};

/** How one object reacts when it is hit */
struct StageHitInfo {
	int mDamage; // _00
	u32 mKinds;  // _04 1 = kind 0 hits it 2 = kind 1 and 4 = kind 2
};

/** One row of a stage object list in the common and stage object files */
struct StageModelInfo {
	f32 mDrawDist; // _00
	s16 mModel;    // _04 in the pack's models
	u8 _06[2];
	s16 mMotion;              // _08 in gGeObjCommon's motions -1 = none
	s16 mColli;               // _0A in the pack's collision -1 = none
	int mSound;               // _0C in sStageObjBreakSounds
	u32 mFlags;               // _10 StageModelFlags
	int mShapeCount;          // _14
	StageShapeInfo** mShapes; // _18
	ShatterBank* mPieces;     // _1C
	u32* mFx;                 // _20 two arguments for particleGenById
	const char* mName;        // _24
	u8 _28[8];
	StageHitInfo* mHit; // _30
};

// Slot _74 builds the drawn object once a model was found
class TObjStageObj : public TSetActor {
public:
	TObjStageObj(TObject* parent, SetDesc* desc);
	virtual ~TObjStageObj();

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual void initFromDesc(SetDesc* desc);
	virtual void debugPrint(void*);
	virtual int onHit(const TObjAction* other, int arg, int* result);
	virtual void vfn_74();

	inline void operator delete(void* mem);

	void computeBounds();
	void updateMatrix();
	void updateColliBox();
	int checkContacts();
	void updateIdle();
	void updateWobble();
	void updateBroken();
	void fall();
	int checkGround();
	void createShapes();
	void updateShapes();

	StageModelInfo* mInfo;   // _64
	MeshObject* mColli;      // _68 the model's collision
	void* mFx;               // _6C
	int mBreakBank;          // _70 the effect bank 7 = common and 8 = stage
	int mBreakId;            // _74
	ModelDraw mDraw;         // _78
	MotionTrack mTrack;      // _94
	ObjDrawState mDrawState; // _B0
	f32 mMatrix[12];         // _D4
	f32 mScale;              // _104
	DrawTask* mDrawTask;     // _108
	EnemyLight mLight;       // _10C
	int mIndex;              // _14C in the list
	int mBank;               // _150 0 = common and 1 = stage
	s16 mModelId;            // _154
	s16 mMotionId;           // _156
	s16 mColliId;            // _158
	u32 mModelFlags;         // _15C StageModelFlags
	u8 mDropItem;            // _160 for dropItem
	int mAct;                // _164 StageObjAct
	Vec3f mPieceSize;        // _168 the first shape's size for the pieces
	Vec3f mGroundPos;        // _174 where the last look down found the ground
	int mOnGround;           // _180
	int mGroundTimer;        // _184 frames since the last look down
	int mFrames;             // _188 frames falling or frames since last touched
	int mLightSet;           // _18C
	f32 mDrawDist;           // _190 at least 850
	Vec3f mBoxLow;           // _194
	Vec3f mBoxHigh;          // _1A0
	Vec3f mCentre;           // _1AC
	f32 mRadius;             // _1B8
	Vec3f mWorldCentre;      // _1BC
	f32 mWorldRadius;        // _1C8
};

#endif
