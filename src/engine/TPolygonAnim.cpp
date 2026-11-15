#include "engine/TPolygonAnim.h"
#include "engine/DrawTask.h"
#include "engine/TFogCtrl.h"
#include "engine/FxSystem.h"
#include "Ninja/ninja.h"
#include "core/TMainTask.h"

typedef char TPolygonAnim_size_check[(sizeof(TPolygonAnim) == 0x44) ? 1 : -1];

static void task0(void*);
static void task1(void*);
static void task2(void*);
static void task3(void*);
static void task4(void*);

const char* gTPolygonAnimName = "TPolygonAnim";

// 0x800DA0F0
TPolygonAnim::TPolygonAnim(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTPolygonAnimName;
	mSize = 0x44;
	mTask[0] = addDrawTask(this, (void (*)(void*))task0, 0x21, 0);
	mTask[1] = addDrawTask(this, (void (*)(void*))task1, 0x22, 0);
	mTask[1]->mSortKey = 0xffff;
	mTask[2] = addDrawTask(this, (void (*)(void*))task2, 0x2A, 0);
	mTask[3] = addDrawTask(this, (void (*)(void*))task3, 0x2B, 0);
	mTask[4] = addDrawTask(this, (void (*)(void*))task4, 0x3A, 0);
	_40 = 0;
}

// 0x800DA1E0
TPolygonAnim::~TPolygonAnim()
{
	int i;

	fxRemoveAll();
	for (i = 0; i < 5; i++) {
		removeDrawTask(mTask[i]);
	}
}

// 0x800DA268
void TPolygonAnim::draw()
{
	int i;

	for (i = 0; i < 5; i++) {
		fxUpdateGroup(i);
	}
	mAt.set(0.0f, 0.0f, 0.0f);
	gjScaleVector(0.016666668f, &mAt, &mAt);
}

// 0x800DA2C8
static void drawKind(int kind)
{
	f32 mtx[12];

	njGetMatrix(mtx);
	fxSetView(mtx);
	fxDrawGroup(kind);
	fogApply();
}

// 0x800DA30C
static void task0(void*) { drawKind(0); }

// 0x800DA330
static void task1(void*) { drawKind(1); }

// 0x800DA354
static void task2(void*) { drawKind(2); }

// 0x800DA378
static void task3(void*) { drawKind(3); }

// 0x800DA39C
static void task4(void*) { drawKind(4); }

// 0x800DA3C0
void TPolygonAnim::addAt(void* dst) const
{
	Vec3f* out = (Vec3f*)((u8*)dst + 0x3C);
	out->x += mAt.x;
	out->y += mAt.y;
	out->z += mAt.z;
}

// 0x800DA3F4
int TPolygonAnim::addAt0(void* dst)
{
	addAt(dst);
	return 0;
}

// 0x800DA418
int TPolygonAnim::addAt1(void* dst)
{
	addAt(dst);
	return 0;
}

// 0x800DA43C
int TPolygonAnim::addAt2(void* dst)
{
	addAt(dst);
	return 0;
}
