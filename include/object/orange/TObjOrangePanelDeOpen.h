#ifndef _TOBJORANGEPANELDEOPEN_H
#define _TOBJORANGEPANELDEOPEN_H
struct SetDesc;
struct TObject;
struct MeshColli;
struct MeshData;
struct CharaColli;

#include "stage/SetActor.h"
#include "object/ObjModel.h"
#include "engine/DrawTask.h"
#include "enemy/EnemyLight.h"
#include "Ninja/ninja.h"

extern "C" void objHeapFree(void*, void*);

/**
 * One tile of the panel. The position is taken from a child of MODEL_PANEL_FLOOR, and mSuit cycles
 * SPADE, HEART, DIAMOND, CLOVER. Ours.
 *
 * @size 0x10
 */
struct PanelTile {
	PanelTile();

	Vec3f mPos;  // _00
	u8 mSuit;    // _0C
	u8 mPressed; // _0D 1 while something is standing on it
	u8 _0E[2];
};

/**
 * A floor of 25 suit tiles. Set int 0 is a switch id, set int 1 is the answer suit, clamped to 0..3.
 * Stepping a tile advances its suit. When every tile shows the answer, the switch is set and the panel
 * stays open. A switch that is already set starts open.
 *
 * @size 0x2DC
 */
class TObjOrangePanelDeOpen : public TSetActor {
public:
	TObjOrangePanelDeOpen(TObject* parent, SetDesc* desc);
	virtual ~TObjOrangePanelDeOpen();

	inline void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();                      // _0C
	virtual void vfn_10();                    // _10
	virtual void render();                    // _1C
	virtual void initFromDesc(SetDesc* desc); // _30
	virtual void debugPrint(void*);           // _34
	virtual void vfn_74();                    // _74

	void readTiles();
	void makeShapes();
	void updateShapes();
	void freeShapes();
	static void setColliBox(void* colli, const Vec3f* center, f32 radius);
	void initColli();
	void updateColli();
	void freeColli();
	int suitsMatch();
	void statePlay();
	void stateDone();

	NJS_OBJECT* mModelData; // _64 MODEL_PANEL_FLOOR
	void* mTexFloor;        // _68 TLS_MODEL_PANEL_FLOOR
	void* mTexTiles;        // _6C TLS_MODEL_PANEL
	MeshData* mColliData;   // _70 COL_MODEL_PANEL_FLOOR
	DrawTask* mDrawTask;    // _74
	ObjModel mModel;        // _78
	f32 mMatrix[12];        // _98
	MeshColli* mMesh;       // _C8
	u16 mSwitch;            // _CC set int 0
	u8 mAnswer;             // _CE set int 1 clamped to 0..3
	u8 _CF;
	Vec3f mCenter;          // _D0
	f32 mRadius;            // _DC
	PanelTile mTiles[25];   // _E0
	CharaColli* mTileColli; // _270 25 boxes
	EnemyLight mLight;      // _274
	ObjDrawState mState;    // _2B4
	int mAct;               // _2D8 0 playing 1 open
};

extern "C" {
TObject* createTObjOrangePanelDeOpen(TObject*, SetDesc*);
}

extern "C" {
extern const char* sPanelAnswerNames[];
}

#endif
