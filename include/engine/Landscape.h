#ifndef _LANDSCAPE_H
#define _LANDSCAPE_H

#include "core/types.h"
struct DrawTask;
struct NJS_TEXLIST;
#include "stage/Stage.h"

/**
 * @size 0x40
 */
struct LandNode {
	u32 mFlags;    // _00 1 no translate 2 no rotate 4 no scale 0x20 skip the children 0x40 rotate in ZXY order
	s16 mMatrix;   // _04 the slot in the output matrix array
	s16 _06;       // _06
	s16 mChild;    // _08 -1 = none
	s16 mSibling;  // _0A -1 = none
	f32 mPosX;     // _0C
	f32 mPosY;     // _10
	f32 mPosZ;     // _14
	u32 mAngX;     // _18
	u32 mAngY;     // _1C
	u32 mAngZ;     // _20
	f32 mSclX;     // _24
	f32 mSclY;     // _28
	f32 mSclZ;     // _2C
	Vec3f mCenter; // _30
	f32 mRadius;   // _3C
};

/**
 * Render state of a mesh. LandModel.cpp sets it up with setLandMaterial.
 * @size 0x30
 */
struct LandMaterial {
	u32 mFlags;     // _00 1 vertex colours 2 unlit 4 two sided 0x10 no z test 0x20 no z write 0x40 alpha test
	u8 mColor[4];   // _04 the material colour when mFlags & 1 is clear
	u32 _08;        // _08
	int mBlend;     // _0C GXSetBlendMode's arguments
	int mSrc;       // _10
	int mDst;       // _14
	int mZFunc;     // _18
	u32 _1C;        // _1C
	u32 mTexFlags;  // _20 the low byte is the GXSetTevOp mode then wrap and texgen source bits
	int mTexNum;    // _24
	u32 mTexFlags2; // _28 the second texture
	int mTexNum2;   // _2C
};

/**
 * @size 0x8
 */
struct LandMaterialEntry {
	int mTexMode;            // _00 0 untextured 1 one texture 3 two textures
	LandMaterial* mMaterial; // _04
};

/**
 * The vertex arrays of some meshes. Each type is 0 for none, then picks a GX format.
 * @size 0x30
 */
struct LandVtxSet {
	u16 mPosType;  // _00
	u16 _02;       // _02
	void* mPos;    // _04
	u16 mNrmType;  // _08
	u16 _0A;       // _0A
	void* mNrm;    // _0C
	u16 mClrType;  // _10
	u16 mClrCount; // _12
	u32* mClr;     // _14 RGBA8
	u16 mClr2Type; // _18
	u16 _1A;       // _1A
	void* mClr2;   // _1C
	u16 mTexType;  // _20
	u16 _22;       // _22
	void* mTex;    // _24
	u16 mTex2Type; // _28
	u16 _2A;       // _2A
	void* mTex2;   // _2C
};

/**
 * @size 0x8
 */
struct LandVtxEntry {
	u32 _00;          // _00
	LandVtxSet* mSet; // _04
};

/**
 * Two display lists, drawn with front and back faces culled in turn.
 * @size 0x14
 */
struct LandDisplay {
	u32 mVtxFlags;  // _00 which of the six vertex arrays are indexed
	void* mFront;   // _04
	u32 mFrontSize; // _08
	void* mBack;    // _0C
	u32 mBackSize;  // _10
};

/**
 * @size 0x8
 */
struct LandDisplayEntry {
	u32 mType;             // _00 only 0 is drawn
	LandDisplay* mDisplay; // _04
};

/**
 * @size 0x14
 */
struct LandMesh {
	int mNode;     // _00 the index into the visibility list
	int mMatrix;   // _04
	int mMaterial; // _08
	int mVtxSet;   // _0C
	int mDisplay;  // _10
};

/**
 * @size 0xC
 */
struct LandMeshGroup {
	u32 mPass;         // _00 drawn when the pass mask has one of these bits 1 and 4 opaque 2 translucent
	int mMeshCount;    // _04
	LandMesh* mMeshes; // _08
};

/**
 * @size 0x2C
 */
struct LandModel {
	int mMaterialCount;            // _00
	LandMaterialEntry* mMaterials; // _04
	int mVtxSetCount;              // _08
	LandVtxEntry* mVtxSets;        // _0C
	int mDisplayCount;             // _10
	LandDisplayEntry* mDisplays;   // _14
	u32 mNodeCount;                // _18
	LandNode* mNodes;              // _1C
	int mMatrixCount;              // _20 1 or up to 9 matrices picked per mesh
	int mGroupCount;               // _24
	LandMeshGroup* mGroups;        // _28
};

/**
 * A second set of vertex colours for a model, the night ones. LandModel.cpp blends the model's colours toward these
 * into a copy of the model.
 * @size 0x8
 */
struct LandMotion {
	int mVtxSetCount;       // _00
	LandVtxEntry* mVtxSets; // _04 only mClr and mClrCount are read
};

/**
 * What is being drawn, for the draw hook. One static instance.
 * @size 0x30
 */
struct LandDrawState {
	int mMaterial;             // _00
	int mLastMaterial;         // _04 setLandMaterial does nothing when it equals mMaterial unless mForce is set
	u32 _08;                   // _08
	int mMesh;                 // _0C
	int mGroup;                // _10
	LandMaterialEntry* mEntry; // _14
	LandModel* mModel;         // _18
	f32* mMtx;                 // _1C
	u32 mPass;                 // _20
	u32 mFlags;                // _24
	int mForce;                // _28 a hook sets it to have the next mesh's material set again
	u32 _2C;                   // _2C
};

typedef void (*LandDrawHook)(LandDrawState* state);

// LandModel.cpp
u32 getLandColorBufferSize(LandModel* model, LandMotion* motion, int unused);
void initLandColorBuffer(LandModel* buffer, LandModel* model, LandMotion* motion, int unused);
void blendLandColors(LandModel* buffer, LandModel* model, LandMotion* motion, int unused, f32 t);
int setLandMaterial(const LandDrawState* state);
void drawLandModel(LandModel* model, f32* mtx, u32* hidden, u32 pass, u32 flags);
void calcLandMatrices(f32* out, LandModel* model);
void calcLandMotionMatrices(f32* out, LandModel* model, void* motion, f32 frame);
void mulLandMatrix(f32* out, f32* a, f32* b, u32 count);
void setLandDrawHook(LandDrawHook hook);

/**
 * One animated part of a block, like a turning windmill.
 * @size 0xC
 */
struct LandPartDef {
	LandModel* mModel;   // _00
	LandMotion* mMotion; // _04
	int mId;             // _08
};

/**
 * @size 0x10
 */
struct LandBlock {
	LandModel* mModel;   // _00
	LandMotion* mMotion; // _04
	int mPartCount;      // _08
	LandPartDef* mParts; // _0C
};

/**
 * @size 0x8
 */
struct LandAnimNode {
	u16 mPartId; // _00
	s16 mAnim;   // _02 -1 = none
	u32 _04;     // _04
};

/**
 * Ninja's NJS_ACTION layout.
 * @size 0x8
 */
struct LandAction {
	void* mObject; // _00
	u32* mMotion;  // _04 a Ninja NJS_MOTION
};

/**
 * @size 0x8
 */
struct LandAnimEntry {
	u32 _00;             // _00
	LandAction* mAction; // _04
};

struct LandAnimSet {
	u8 _00[0x8];
	u32 mNodeCount;       // _08
	LandAnimNode* mNodes; // _0C
	u8 _10[0x4];
	LandAnimEntry* mAnims; // _14
};

/**
 * The root of a .lnd file.
 * @size 0x18
 */
struct LandFile {
	LandBlock* mMain;      // _00
	int mVariantCount;     // _04 2 on world stages 1 on the battle stage checked 0 on the boss stage checked
	LandBlock** mVariants; // _08
	LandAnimSet* mAnims;   // _0C
	NJS_TEXLIST* mTexList; // _10 a Ninja NJS_TEXLIST
	void* mGvm;            // _14 the texture bank
};

/**
 * Runtime state of one LandMotion.
 * @size 0x14
 */
struct LandMotionState {
	LandModel* mModel;   // _00
	LandModel* mBuffer;  // _04 the model or its copy with blended colours when there is a motion
	LandMotion* mMotion; // _08
	f32* mMatrices;      // _0C
	int mAnimIndex;      // _10 -1 = none
};

/**
 * Runtime state of one LandBlock.
 * @size 0x30
 */
struct LandBlockState {
	LandBlock* mBlock;       // _00
	LandMotionState mRoot;   // _04
	LandMotionState* mParts; // _18
	f32* mPartMatrices;      // _1C one matrix per part
	u32* mVisible;           // _20 one int per node
	u32* mVisibleFar;        // _24
	void* mMotionBuffers;    // _28
	NJS_TEXLIST* mTexList;   // _2C
};

/**
 * Runtime state of one animation in the LandAnimSet.
 * @size 0x14
 */
struct LandAnimState {
	f32 mFrame;        // _00
	f32 mSpeed;        // _04
	LandModel* mModel; // _08
	u32* mHeader;      // _0C a Ninja NJS_MOTION The frame count is at _04
	f32* mMatrices;    // _10
};

/**
 * @size 0x40
 */
struct TLandscape {
	TLandscape();
	~TLandscape();

	void init(LandFile* file);
	void freeBlocks();
	void updateAnims();
	LandBlockState* getVariant(int index);

	LandFile* mFile;            // _00
	LandBlockState mMain;       // _04
	LandBlockState* mVariants;  // _34
	LandAnimState* mAnimStates; // _38
	f32* mAnimMatrices;         // _3C
};

/**
 * The object that updates and draws the landscape.
 * @size 0x34
 */
class TLandNightTest : public TObject {
public:
	TLandNightTest(TObject* parent);
	virtual ~TLandNightTest();

	virtual void draw();   // _0C runs the fade and the animation
	virtual void vfn_10(); // _10 sorts nodes by distance from the camera

	inline void* operator new(unsigned long size, void* pool);
	inline void operator delete(void* mem);

	void setBlend(f32 blend, int frames);
	void applyBlend(f32 blend);

	DrawTask* mDraw;  // _20 opaque pass
	void* mDrawAlpha; // _24 translucent pass
	f32 mBlend;       // _28 0 = first variant 1 = second variant
	f32 mBlendTarget; // _2C
	int mBlendTimer;  // _30
};

void initLandscape();
void loadLandscape(const char* name);
void freeLandscape();
void createLandscapeObject();
void setLandscapeBlend(f32 blend, int frames);
f32 getLandscapeBlend();

extern "C" {
void setLandscapeFlag(int);
}

extern "C" {
int setLandscapeMode(char*);
}

extern "C" {
extern void* gTLandNightTestName;
}

#endif
