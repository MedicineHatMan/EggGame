#ifndef _ITEMPARTS_H
#define _ITEMPARTS_H

#include "core/types.h"
#include "stage/Stage.h"
#include "engine/MotionPlayer.h"
#include "object/ObjCommon.h"

struct ItemHolder;
struct GjActor;

/** One effect an item model carries on a node: the node index and the particle id. @size 0x4 */
struct ItemFxNode {
	s16 mNode; // _00
	s16 mId;   // _02
};

/**
 * Record at +0x4C of an item archive: the model, its motions and the effects it carries. The first word of root,
 * and mTexList, go to the owner's slot 0xA4. Names are ours.
 *
 * @size 0x20
 */
struct ItemModelRec {
	void* root;                // _00 points at the NJS_OBJECT
	void* mMotions;            // _04
	void* arg;                 // _08 the texture list
	u32 _0C;                   // _0C
	int mMotionNum;            // _10
	const MotionEntry* mTable; // _14
	int mFxNum;                // _18
	ItemFxNode* mFx;           // _1C
};

/**
 * Per-node matrices for a model worn or carried, and the effects that ride on its nodes. Its code is at 0x801CDD1C.
 * init maps each node that carries an effect to a slot, set stores a node's matrix, and calc moves every slot by
 * the base matrix and puts the effects on their slots. Vtable 0x8026A7D0. Name is ours.
 *
 * @size 0x1C
 */
struct ItemNodes {
	ItemNodes();
	virtual ~ItemNodes();

	void init(const ItemModelRec* rec);
	int has(int node);
	void set(int node, f32* mtx);
	void calc(f32* base);

	// _00 = vtable
	TObject* mFx; // _04 the effects' parent
	f32* mOut;    // _08 mNum matrices base times mNode
	f32* mNode;   // _0C mNum matrices from set
	int mNum;     // _10
	s16* mSlot;   // _14 a slot per node -1 = none
	int mSlotNum; // _18
};

/** What makes an item's model pulse. Its code is at 0x801CD704. @size 0x60 */
struct ItemGlow {
	ItemGlow();
	~ItemGlow();

	void stop(ItemHolder* holder);
	void update(ItemHolder* holder);
	void calc(ItemHolder* holder, f32* mtx);
	void draw(ItemHolder* holder, const f32* mtx);
	void setup(ItemHolder* holder);
	void tick(const ItemHolder* holder);
	void play(const ItemHolder* holder, int id);

	s8 mState;            // _00 0 until the archive is in then 1
	GjActor* mActor;      // _04 for a skinned model
	MotionPlayer mMotion; // _08
	ItemNodes mNodes;     // _44
};

class MeshColli;

/** The start of an AniRider AniRider.h the links and the mesh it stands on */
struct BombRider {
	void* mNext;      // _00
	void* mPrev;      // _04
	void* mSelf;      // _08
	void* mList;      // _0C
	MeshColli* mMesh; // _10 the mesh it stands on or null
};

/**
 * The falling body of a bomb, run as a small state machine: each state takes an enter, exit, update and check
 * event. Its code is at 0x8018AAF0, after TEneLastExBossFeather. The names are ours.
 *
 * @size 0x4C
 */
struct BombBody {
	enum {
		BOMB_RIDE    = 0, // on a mesh carried when it moves
		BOMB_FALL    = 1, // gravity and drag
		BOMB_FLY     = 2, // thrown no gravity
		BOMB_STOPPED = 3,
		BOMB_NONE    = 4,
	};
	enum {
		BOMB_ENTER  = 0,
		BOMB_EXIT   = 1,
		BOMB_UPDATE = 2,
		BOMB_CHECK  = 3,
	};

	BombBody();  // 0x8018AAF0
	~BombBody(); // 0x8018AB54

	int getGround(Vec3f* out) const; // 0x8018ABB4 the floor below while it is near
	void launch(const Vec3f* vel);   // 0x8018ABE8
	void start(const Vec3f* vel);    // 0x8018AC24
	void stop();                     // 0x8018AC60
	void update();                   // 0x8018ACB8
	void run(int state, int event);  // 0x8018AD04
	void setState(int state);        // 0x8018AD64
	void stateRide(int event);       // 0x8018ADC4
	void stateFall(int event);       // 0x8018AE7C
	void stateFly(int event);        // 0x8018AF18
	void probe();                    // 0x8018AF94

	s8 mState;        // _00
	FlagByte mFlags;  // _01 1 = a floor is near
	u16 mTri;         // _02 the triangle it stands on
	Vec3f mPos;       // _04
	Vec3f mVel;       // _10
	f32 mRadius;      // _1C
	BombRider mRider; // _20
	Vec3f mLocal;     // _34 where it stands in the mesh's space
	Vec3f mGround;    // _40
};

#endif
