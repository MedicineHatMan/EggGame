#ifndef _ENEMYLIGHT_H
#define _ENEMYLIGHT_H

#include "core/types.h"
#include "egg/ObjEgg.h"

struct DrawTask;
struct DrawState;

/**
 * The light that follows an object and decides whether it is worth drawing. Enemies and many set objects hold one
 * as a member, so it is built and destroyed in member order. It keeps a state for each of the four views: the
 * object fades out as the camera comes within mFadeDist of it and as it nears the far distance. Its code is in
 * TCamPath.cpp. The name is ours.
 *
 * @size 0x40
 */
struct EnemyLight {
	EnemyLight();  // 0x80064208
	~EnemyLight(); // 0x80064258
	void reset();  // 0x80064294

	void setFadeDist(f32 dist);                                  // 0x800642EC
	int isFaded(int* alpha);                                      // 0x800642F4
	void noteView(int view, int nearFade, int farFade);          // 0x8006434C empty
	void sortTask(int view, int wasFaded, DrawTask* task);       // 0x80064350
	void setAlpha(f32 opacity, int view, int least);             // 0x800643DC
	int checkView(f32 far, f32 radius, Vec3f* pos, Vec3f* view);  // 0x80064494
	int isVisible(void* task, f32 far, Vec3f* pos, f32 radius, int view); // 0x80064608
	void place(int mode, DrawTask* task, Vec3f* pos, Vec3f* view); // 0x800646B8
	DrawState* drawState(DrawState* buf) const;                        // 0x800647D4
	void drawObject(void* model);                                // 0x8006487C
	void drawActor(void* actor);                                 // 0x800648E8
	void drawMotion(void* object, void* motion, f32 frame);      // 0x80064960

	s8 mFaded[4];    // _00 nonzero while that view draws it see-through
	u8 mAlpha[4];    // _04
	HitVec mView[4]; // _08 the object in each view's space
	f32 mFadeDist;   // _38
	int mTimer;      // _3C 0 = no fading up close
};

#endif
