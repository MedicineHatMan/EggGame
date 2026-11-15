#ifndef _TENELASTEXBOSSCORE_H
#define _TENELASTEXBOSSCORE_H

#include "stage/Stage.h"
struct DrawTask;
struct NJS_TEXLIST;
#include "stage/SetManager.h"
#include "engine/MotionPlayer.h"
#include "enemy/EnemyAnim.h"
#include "enemy/TEneAm01Parts.h"
#include "engine/FxHolder.h"
struct NJS_OBJECT;

enum LastExCoreState {
	CORE_APPEAR = 0, // fading in 4 a frame
	CORE_SHOWN  = 1, // glowing
	CORE_STILL  = 2, // shown without the glow
	CORE_HIDDEN = 3,
	CORE_BURST  = 4, // the death motion
};

class TEneLastExBossCore : public TObject {
public:
	TEneLastExBossCore(TObject* parent);
	virtual ~TEneLastExBossCore();
	virtual void draw();   // _0C
	virtual void vfn_10(); // _10 hidden while off camera or clear
	virtual void render(); // _1C

	void appear();                 // 0x8018D2E8
	void vanish();                 // 0x8018D31C
	void show();                   // 0x8018D33C
	void hide();                   // 0x8018D350
	void burst();                  // 0x8018D364
	void setPos(const Vec3f* pos); // 0x8018D3A0
	void flash();                  // 0x8018D3BC

	NJS_OBJECT* mModel;    // _20
	void* mMotions;        // _24
	NJS_TEXLIST* mTexList; // _28
	SetTransform mXform;   // _2C
	MotionPlayer mMotion;  // _70
	EnemyAnim mClip;       // _AC
	DrawTask* mDraw;       // _B8
	EnemyTint mTint;       // _BC the hit flash
	FxHolder mGlow;        // _C4
	int mState;            // _F8 a LastExCoreState
	int mAlpha;            // _FC 0 to 255
};

// Drops a TObjLastBossShot at pos
extern "C" void dropLastBossShot(const Vec3f* pos);

extern "C" {
extern u32 sLastExCoreName[2];
extern const char* gTObjLastBossShotName;
}

#endif
