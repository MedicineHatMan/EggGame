#ifndef _TOBJLAUNCHER_H
#define _TOBJLAUNCHER_H
struct GjModel;

#include "object/ObjAction.h"
struct NJS_OBJECT;
#include "stage/SetManager.h"
#include "engine/DrawTask.h"

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;
}

/**
 * A piece thrown off by something that breaks: a model that flies, spins and falls until it reaches mFloor.
 * Spawned rather than placed. The names are ours.
 *
 * @size 0xE4
 */
class TObjLauncher : public TObjAction {
public:
	enum {
		BIT_STOPPED = 1,    // reached the floor
		BIT_DRAG    = 2,    // slow the flight by mDrag
		BIT_KEEP    = 4,    // stay once on the floor instead of being deleted
		BIT_OBJ_ID  = 8,    // holds an object id
		BIT_DELAYED = 0x10, // waiting mDelay frames before it starts
	};

	TObjLauncher(TObject* parent, f32 floorY, f32 scale, const Vec3f* pos, const Vec3f* vel, int bank, int model,
	             int texture); // 0x800552E8
	TObjLauncher(TObject* parent, f32 floorY, f32 scale, const Vec3f* pos, const Vec3f* vel, void* model,
	             void* texture); // 0x80055480
	// The scale is passed by value
	TObjLauncher(TObject* parent, f32 floorY, const Vec3f* pos, const SetRot* rot, const Vec3f* vel, Vec3f scale, void* motion,
	             void* texture); // 0x800555E8
	virtual ~TObjLauncher();     // 0x800556E0

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();     // _0C 0x80055784
	virtual void vfn_10();   // _10 0x80055808 blinks once 30 frames have passed
	virtual void render();   // _1C 0x80055858
	virtual Vec3f* getPos(); // _20 0x80054910 defined at the end of TObjArrowPoint.cpp
	virtual int getRotY();   // _28 0x80055C94
	virtual int getKind();   // _2C 0x80055C8C

	void init(const Vec3f* pos, const Vec3f* vel); // 0x800551E0
	void calcMatrix();                             // 0x80055910
	void step();                                   // 0x80055964
	void setGravity(f32 g);                        // 0x80055B30
	void noDrag();                                 // 0x80055B48
	void keepAtFloor();                            // 0x80055B60
	void setRandomSpin();                          // 0x80055B70
	void faceVelocity();                           // 0x80055C40
	void takeObjId();                              // 0x80055C4C

	void spin();

	u32 mBits;          // _30
	int mSpinMode;      // _34 0 = spin by mSpin 1 = point along the velocity
	Vec3f mScale;       // _38
	f32 mFloor;         // _44 the height where it stops
	int mSpin[3];       // _48 added to the rotation each frame
	NJS_OBJECT* mModel; // _54 an NJS_OBJECT
	GjModel* mMotion;   // _58 a model drawn with the default draw state
	void* mTexture;     // _5C
	SetTransform mX;    // _60
	f32 mDrag;          // _A4
	f32 mSpinMax;       // _A8
	f32 mMtx[12];       // _AC
	DrawTask* mDraw;    // _DC
	u16 mAge;           // _E0
	u16 mDelay;         // _E2
};

extern "C" {
struct DrawStateSlot;
struct DrawState;
DrawStateSlot* getDrawStateSlot();                           // 0x80055CFC
DrawState* getDefaultDrawState();                            // 0x80055D04
f32* gjGetMatrix();                                          // 0x80055D40
void createDrawStateSlot();                                  // 0x80055C9C
void deleteDrawStateSlot();                                  // 0x80055CD4
void enableSSMode();                                         // 0x80055D10
void endFrameSwap();                                         // 0x80055D1C
void debugDrawBegin(const f32* mtx, const u32* color);       // 0x80055D48
void debugDrawSphere(const f32* mtx, const u32* color);      // 0x80055E88
void debugDrawCylinder(const f32* mtx, const u32* color);    // 0x80055EB0
void debugDrawBeginOpaque(const f32* mtx, const u32* color); // 0x80055ED8
void drawWireSphere(u8 rings, u8 segs);                      // 0x80055FFC
void debugWireSphere(const f32* mtx, const u32* color);      // 0x800561E4
void drawWireCylinder(u8 segs);                              // 0x80056238
void debugWireCylinder(const f32* mtx, const u32* color);    // 0x80056440
void debugWireBox(const f32* mtx, const u32* color);         // 0x80056490
}

extern "C" {
extern const char* gTObjLauncherName;
}

#endif
