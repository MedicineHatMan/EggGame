#include "enemy/boss/EneBoss.h"
struct DrawTask;
#include "engine/DrawTask.h"
#include "object/ObjArchive.h"
#include "egg/EggCreature.h"
#include "stl/math.h"
#include "animal/AniMount.h"
#include "object/ObjAction.h"
#include "engine/TFootPrinter.h"
#include "core/MathUtil.h"
#include "ui/AeArc.h"
#include "Ninja/ninja.h"
#include "core/TMainTask.h"

typedef char FootPrint_size_check[(sizeof(FootPrint) == 0x38) ? 1 : -1];

typedef char TFootPrinter_size_check[(sizeof(TFootPrinter) == 0x382C) ? 1 : -1];

// 0x80085FCC
extern "C" void* footPrinterCreate()
{
	if (sFootPrinterObj == 0) {
		TFootPrinter* obj = new TFootPrinter(gTL_05);
		sFootPrinterObj = obj;
	}
	return sFootPrinterObj;
}

// 0x8008601C
extern "C" void* footPrinterGet() { return sFootPrinterObj; }

const char* gTFootPrinterName = "TFootPrinter";

// 0x80086024
TFootPrinter::TFootPrinter(TObject* parent)
    : TObject(parent)
{
	_00   = (u32)gTFootPrinterName;
	mSize = 0x382C;
	mNext = 0;
	mTask = addDrawTask(this, (void (*)(void*))drawObjectTask, 0x22, 0);
	initRing();
	mTexList = getStgObjTextures()[35];
	if (mTexList == 0) {
		mTask->mFlags |= 1;
	}
}

// 0x800860C4
TFootPrinter::~TFootPrinter()
{
	removeDrawTask(mTask);
	if (sFootPrinterObj == this) {
		sFootPrinterObj = 0;
	}
}

// 0x80086140
void TFootPrinter::add(const Vec3f* pos, const Vec3f* dir, int rotY, int kind)
{
	FootPrint* print = mNext;
	print->mLife     = 0xB4;
	print->mKind     = kind;

	Vec3f up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	njOuterProduct(&up, dir, &up);

	f32 matrix[12];
	mtxSetTransV(matrix, pos);
	mtxRotateZ(matrix, matrix, (int)(10430.38043493439 * (f32)asin(up.z)));
	mtxRotateY(matrix, matrix, (int)(10430.38043493439 * (f32)asin(up.y)));
	mtxRotateX(matrix, matrix, (int)(10430.38043493439 * (f32)asin(up.x)));
	mtxRotateY(matrix, matrix, rotY);

	for (int i = 0; i < 4; i++) {
		njCalcPoint((const f32*)matrix, (const Vec3f*)&sFootCorners[i], &print->mCorner[i]);
	}
	mNext = mNext->mNext;
}

// 0x8008628C
extern "C" void footPrintAdd(const Vec3f* pos, const Vec3f* dir, int kind, f32 rotY)
{
	TFootPrinter* obj = (TFootPrinter*)footPrinterGet();
	if (obj != 0) {
		obj->add(pos, dir, (int)rotY, kind);
	}
}

// 0x800862F0
void TFootPrinter::initRing()
{
	for (s16 i = 0; i < 0x100; i++) {
		mPrints[i].mLife = 0;
		mPrints[i].mNext = &mPrints[(i + 1) % 0x100];
	}
	mNext = mPrints;
}

// 0x80086350
void TFootPrinter::draw()
{
	FootPrint* print = mNext;
	for (int i = 0; i < 0x100; i++) {
		if (print->mLife != 0) {
			print->mLife--;
		}
		print = print->mNext;
	}
}

// 0x8008637C
void TFootPrinter::vfn_10() { }

// 0x80086380
void TFootPrinter::render()
{
	FootPrint* print = mNext;
	njSetTexture(mTexList);
	njSetTextureNum(0);
	njColorBlendingMode(0, 8);
	njColorBlendingMode(1, 6);
	pushCullMode(0x10000000);
	njDrawTexture3DExStart(1);
	for (int i = 0x100; i-- != 0;) {
		if (print->mLife == 0) {
			print = print->mNext;
			continue;
		}
		FootCorner verts[4];
		for (int j = 0; j < 4; j++) {
			verts[j].mPos = print->mCorner[j];
			verts[j].mU   = sFootCorners[j].mU;
			verts[j].mV   = sFootCorners[j].mV;
		}
		u32 alpha  = (u32)(128.0f * ((f32)print->mLife / 180.0f));
		u32 colour = (alpha << 24) | 0xFFFFFF;
		verts[0].mColour = verts[1].mColour = verts[2].mColour = verts[3].mColour = colour;
		njDrawTexture3DExSetData((const NJS_TEXTURE_VTX*)verts, 4);
		print = print->mNext;
	}
	njDrawTexture3DExEnd();
	popCullMode();
}
