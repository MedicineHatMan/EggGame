#include "core/TMainTask.h"
#include "object/TObject.h"
#include "core/Mem.h"
#include "stl/string.h"
#include "stl/string.h"
#include "object/ObjAction.h"
#include "Ninja/ninja.h"

const char* gTObjectName   = "TObject";
const char* gTMainTaskName = "TMainTask";

// The names of the TL_* parents createTopLevels makes
const char* gTL_SUName       = "TL_SU";
const char* gTL_00Name       = "TL_00";
const char* gTL_01Name       = "TL_01";
const char* gTL_02Name       = "TL_02";
const char* gTL_03Name       = "TL_03";
const char* gTL_04Name       = "TL_04";
const char* gTL_05Name       = "TL_05";
const char* gTL_06Name       = "TL_06";
const char* gTL_07Name       = "TL_07";
const char* gTL_08Name       = "TL_08";
const char* gTL_PARTICLEName = "TL_PARTICLE";
const char* gTL_TIMESTOPName = "TL_TIMESTOP";
const char* gTL_03BName      = "TL_03B";
const char* gTL_04BName      = "TL_04B";
const char* gTL_05BName      = "TL_05B";
const char* gTL_06BName      = "TL_06B";

// 0x80013B68
TObject::TObject(TObject* parent)
{
	mFlags         = 0;
	_1C            = 0;
	mSize          = 0;
	mParent        = (TObject*)parent;
	mChild         = NULL;
	_00            = (u32)gTObjectName;
	mSize          = 0x20;
	TObject* owner = (TObject*)parent;
	if (owner == NULL) {
		mPrev = this;
		mNext = NULL;
	} else if (owner->mChild != NULL) {
		TObject* first      = owner->mChild;
		mPrev               = first->mPrev;
		mNext               = NULL;
		first->mPrev->mNext = this;
		first->mPrev        = this;
	} else {
		mPrev         = this;
		owner->mChild = this;
		mNext         = NULL;
	}
	sObjAliveCount++;
}

// 0x80013BF4
TObject::~TObject()
{
	if (mFlags & 0x20) {
		while (true) { }
	}
	mFlags |= 0x20;
	while (mChild != NULL) {
		delete mChild;
	}
	if (mParent != NULL) {
		if (mPrev == this) {
			mParent->mChild = NULL;
		} else if (mParent->mChild == this) {
			mParent->mChild = mNext;
			mPrev->mNext    = NULL;
			if (mNext != NULL) {
				mNext->mPrev = mPrev;
			}
		} else {
			mPrev->mNext = mNext;
			if (mNext != NULL) {
				mNext->mPrev = mPrev;
			} else {
				mParent->mChild->mPrev = mPrev;
			}
		}
	}
	sObjAliveCount--;
}

// 0x80013D24
void objDeleteTree(TObject* obj)
{
	while (obj->mChild != NULL) {
		delete obj->mChild;
	}
}

// 0x80013D78
// Meant to mark everything below for deletion but each pass marks and recurses into the first child again so only the
// first child's line is marked
void objMarkChildrenDeleted(TObject* obj)
{
	TObject* child = obj->mChild;
	while (child != NULL) {
		obj->mChild->mFlags |= 1;
		objMarkChildrenDeleted(obj->mChild);
		child = child->mNext;
	}
}

// 0x80013DCC
// Acts on the flags without running any updates
void objApplyFlags(TObject* obj)
{
	TObject* next = obj->mChild;
	while (obj->mChild != NULL && next != NULL) {
		TObject* child = next;
		next           = next->mNext;
		if (child->mFlags & 3) {
			if (child->mFlags & 1) {
				child->mFlags &= 0xFFFE;
				if (!(child->mFlags & 0x20)) {
					delete child;
					continue;
				}
			}
			if (child->mFlags & 2) {
				objDeleteTree(child);
				child->draw();
				child->mFlags &= ~2;
			}
		} else {
			objApplyFlags(child);
		}
	}
}

// 0x80013EA4
// The update walk Flag bit 4 stops an object from updating and bit 8 is checked with it
void objUpdateTree(TObject* obj)
{
	sObjWalkDepth++;
	TObject* next = obj->mChild;
	while (obj->mChild != NULL && next != NULL) {
		TObject* child = next;
		next           = next->mNext;
		if (child->mFlags & 0xF) {
			if (child->mFlags & 1) {
				child->mFlags &= 0xFFFE;
				if (!(child->mFlags & 0x20)) {
					delete child;
					continue;
				}
			}
			if (child->mFlags & 2) {
				objDeleteTree(child);
				child->draw();
				child->mFlags &= ~2;
			}
			if ((child->mFlags & 4) || (child->mFlags & 8)) {
				continue;
			}
			continue;
		}
		child->draw();
		objUpdateTree(child);
	}
	sObjWalkDepth--;
}

// 0x80013FB4
void objWalkChildren(TObject* obj)
{
	sObjWalkDepth++;
	TObject* child = obj->mChild;
	while (child != NULL) {
		child->vfn_10();
		objWalkChildren(child);
		child = child->mNext;
	}
	sObjWalkDepth--;
}

// 0x80014020
// Prints the tree
void objPrintTree(TObject* obj)
{
	objDumpPrintf("%5d:%5d:%s%s", obj->_1C, obj->mSize, sObjDumpSpaces + (strlen(sObjDumpSpaces) - sObjWalkDepth * 2),
	              (const char*)obj->_00);
	sObjWalkDepth++;
	TObject* child = obj->mChild;
	while (child != NULL) {
		objPrintTree(child);
		child = child->mNext;
	}
	sObjWalkDepth--;
}

// 0x800140BC
// Moves an object to a new parent
void objSetParent(TObject* obj, TObject* parent)
{
	if (obj->mParent != NULL) {
		if (obj->mPrev == obj) {
			obj->mParent->mChild = NULL;
		} else if (obj->mParent->mChild == obj) {
			obj->mParent->mChild = obj->mNext;
			obj->mPrev->mNext    = NULL;
			if (obj->mNext != NULL) {
				obj->mNext->mPrev = obj->mPrev;
			}
		} else {
			obj->mPrev->mNext = obj->mNext;
			if (obj->mNext != NULL) {
				obj->mNext->mPrev = obj->mPrev;
			} else {
				obj->mParent->mChild->mPrev = obj->mPrev;
			}
		}
	}
	obj->mParent = parent;
	if (parent == NULL) {
		obj->mPrev = obj;
		obj->mNext = NULL;
	} else if (parent->mChild != NULL) {
		TObject* first      = parent->mChild;
		obj->mPrev          = first->mPrev;
		obj->mNext          = NULL;
		first->mPrev->mNext = obj;
		first->mPrev        = obj;
	} else {
		obj->mPrev     = obj;
		parent->mChild = obj;
		obj->mNext     = NULL;
	}
}

// 0x800141A4
void TObject::vfn_18(TObject* obj)
{
	memDebugObject(obj);
}

// 0x800141C8
// Counts the children
int objCountChildren(TObject* obj)
{
	int count = 0;
	for (TObject* child = obj->mChild; child != NULL; child = child->mNext) {
		count++;
	}
	return count;
}

// 0x800141E8
void drawObjectTask(TObject* obj)
{
	obj->render();
}

typedef char TMainTask_size_check[(sizeof(TMainTask) == 0x20) ? 1 : -1];

// Node walked from gMainTask.mChild The next link is at +0x0C and flags at +0x04
struct TaskLink {
	u8 _00[4];
	u16 mFlags; // _04
	u8 _06[6];
	TaskLink* mNext; // _0C
};

// 0x80014214
// Parent passed to TObject is 0
TMainTask::TMainTask()
    : TObject(0)
{
	_00   = (u32)gTMainTaskName;
	mSize = 0x20;
}

// 0x80014264
// Drops every child then destroys the object heap
TMainTask::~TMainTask()
{
	objDeleteTree(this);
	objHeapDestroy((ObjHeap*)gObjHeap, 1);
}

extern "C" {

TObject* gTL_SU;
TObject* gTL_00;
TObject* gTL_01;
TObject* gTL_02;
TObject* gTL_03;
TObject* gTL_04;
TObject* gTL_05;
TObject* gTL_06;
TObject* gTL_07;
TObject* gTL_08;
TObject* gTL_PARTICLE;
TObject* gTL_TIMESTOP;
TObject* gTL_03B;
TObject* gTL_04B;
TObject* gTL_05B;
TObject* gTL_06B;
TObject* gTL_03On;
TObject* gTL_04On;
TObject* gTL_05On;
TObject* gTL_06On;
}

// 0x800142D4
extern "C" void initObjects(TObject* parent)
{
	void* mem = objAlloc(0xC);
	if (mem != 0) {
		mem = objHeapInit((ObjHeap*)mem, 0x70000, 0x10);
	}
	gObjHeap = mem;

	gTL_SU            = new TObject(parent);
	gTL_SU->_00       = (u32)gTL_SUName;
	gTL_00            = new TObject(parent);
	gTL_00->_00       = (u32)gTL_00Name;
	gTL_01            = new TObject(parent);
	gTL_01->_00       = (u32)gTL_01Name;
	gTL_02            = new TObject(parent);
	gTL_02->_00       = (u32)gTL_02Name;
	gTL_06            = new TObject(parent);
	gTL_06->_00       = (u32)gTL_06Name;
	gTL_06B           = new TObject(parent);
	gTL_06B->_00      = (u32)gTL_06BName;
	gTL_05            = new TObject(parent);
	gTL_05->_00       = (u32)gTL_05Name;
	gTL_05B           = new TObject(parent);
	gTL_05B->_00      = (u32)gTL_05BName;
	gTL_04            = new TObject(parent);
	gTL_04->_00       = (u32)gTL_04Name;
	gTL_04B           = new TObject(parent);
	gTL_04B->_00      = (u32)gTL_04BName;
	gTL_03            = new TObject(parent);
	gTL_03->_00       = (u32)gTL_03Name;
	gTL_03B           = new TObject(parent);
	gTL_03B->_00      = (u32)gTL_03BName;
	gTL_07            = new TObject(parent);
	gTL_07->_00       = (u32)gTL_07Name;
	gTL_08            = new TObject(parent);
	gTL_08->_00       = (u32)gTL_08Name;
	gTL_PARTICLE      = new TObject(parent);
	gTL_PARTICLE->_00 = (u32)gTL_PARTICLEName;
	gTL_TIMESTOP      = new TObject(parent);
	gTL_TIMESTOP->_00 = (u32)gTL_TIMESTOPName;

	gTL_03On = gTL_03;
	gTL_04On = gTL_04;
	gTL_05On = gTL_05;
	gTL_06On = gTL_06;
}

// 0x80014600
// Bit 0 of the mask is the first node
void TMainTask::draw()
{
	u32 start = syTmrGetCount();
	if (sMainTaskProcMask != sMainTaskProcMaskPrev) {
		u32 bit        = 1;
		TaskLink* node = (TaskLink*)gMainTask.mChild;
		while (node != 0) {
			if ((sMainTaskProcMask & bit) != 0) {
				node->mFlags |= 8;
			} else {
				node->mFlags &= ~8;
			}
			node = node->mNext;
			bit <<= 1;
		}
		sMainTaskProcMaskPrev = sMainTaskProcMask;
	}
	objUpdateTree(this);
	u32 end = syTmrGetCount();
	_1C     = (u16)syTmrCountToMicro(syTmrDiffCount(start, end));
}

// 0x800146B8
// Same walk flag 0x10 mask sMainTaskDrawMask
void TMainTask::vfn_10()
{
	u32 start = syTmrGetCount();
	if (sMainTaskDrawMask != sMainTaskDrawMaskPrev) {
		u32 bit        = 1;
		TaskLink* node = (TaskLink*)gMainTask.mChild;
		while (node != 0) {
			if ((sMainTaskDrawMask & bit) != 0) {
				node->mFlags |= 0x10;
			} else {
				node->mFlags &= ~0x10;
			}
			node = node->mNext;
			bit <<= 1;
		}
		sMainTaskDrawMaskPrev = sMainTaskDrawMask;
	}
	objWalkChildren(this);
	u32 end = syTmrGetCount();
	mSize   = (u16)syTmrCountToMicro(syTmrDiffCount(start, end));
}

// 0x80014770
void TMainTask::vfn_14()
{
	memShowStats();
	memDebugDrawEnd();
}

// 0x80014794
// Pops the event layers without moving anything back then empties every layer but gTL_00
extern "C" void objClearLayers(TMainTask* task)
{
	if (task->mFlags & 0x100) {
		task->mFlags &= ~0x100;
		gTL_03 = gTL_03On;
		gTL_04 = gTL_04On;
		gTL_05 = gTL_05On;
		gTL_06 = gTL_06On;
		gTL_03->mFlags &= ~4;
		gTL_04->mFlags &= ~4;
		gTL_05->mFlags &= ~4;
		gTL_06->mFlags &= ~4;
	}
	objDeleteTree(gTL_TIMESTOP);
	objDeleteTree(gTL_PARTICLE);
	objDeleteTree(gTL_08);
	objDeleteTree(gTL_07);
	objDeleteTree(gTL_06B);
	objDeleteTree(gTL_06);
	objDeleteTree(gTL_05B);
	objDeleteTree(gTL_05);
	objDeleteTree(gTL_04B);
	objDeleteTree(gTL_04);
	objDeleteTree(gTL_03B);
	objDeleteTree(gTL_03);
	objDeleteTree(gTL_02);
	objDeleteTree(gTL_01);
}

// 0x80014890
extern "C" void objClearAllLayers(TMainTask* task)
{
	objClearLayers(task);
	objDeleteTree(gTL_00);
}

// 0x800148B8
// Pauses the On layers and sends new objects to the B layers
extern "C" void objPushLayers(TMainTask* task)
{
	task->mFlags ^= 0x100;
	gTL_03On->mFlags |= 4;
	objApplyFlags(gTL_03On);
	gTL_04On->mFlags |= 4;
	objApplyFlags(gTL_04On);
	gTL_05On->mFlags |= 4;
	objApplyFlags(gTL_05On);
	gTL_06On->mFlags |= 4;
	objApplyFlags(gTL_06On);
	gTL_03 = gTL_03B;
	gTL_04 = gTL_04B;
	gTL_05 = gTL_05B;
	gTL_06 = gTL_06B;
}

// 0x80014960
extern "C" void objMoveChildren(TObject* to, TObject* from)
{
	TObject* next;
	for (TObject* child = from->mChild; child != 0; child = next) {
		next = child->mNext;
		objSetParent(child, to);
	}
}

// 0x800149AC
// Undoes objPushLayers and moves what the B layers gathered back to the On layers
extern "C" void objPopLayers(TMainTask* task)
{
	if (task->mFlags & 0x100) {
		task->mFlags ^= 0x100;
		gTL_03On->mFlags &= ~4;
		gTL_04On->mFlags &= ~4;
		gTL_05On->mFlags &= ~4;
		gTL_06On->mFlags &= ~4;
		gTL_03 = gTL_03On;
		gTL_04 = gTL_04On;
		gTL_05 = gTL_05On;
		gTL_06 = gTL_06On;
		objMoveChildren(gTL_03, gTL_03B);
		objMoveChildren(gTL_04, gTL_04B);
		objMoveChildren(gTL_05, gTL_05B);
		objMoveChildren(gTL_06, gTL_06B);
	}
}

// 0x80014A68
// is the static initializer for this
TMainTask gMainTask;
