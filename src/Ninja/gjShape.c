#include "Ninja/ninja.h"
#include "Ninja/njInternal.h"
#include "Ninja/gjPrim.h"

GjShapeState gjShapeStates[2];
GjShapeState* gjCurShapeState;

// 0x80218164
void gjShapeSetListCount(s32 count)
{
	s32 size = count << 2;
	u8* base;
	gjCurShapeState->size  = size;
	gjCurShapeState->total = size + size;
	base                   = gjCurShapeState->base;
	gjCurShapeState->list0 = (void**)base;
	gjCurShapeState->list1 = (void**)(base + gjCurShapeState->size);
	gjCurShapeState->index = 0;
}

// 0x802181A8
void gjShapeNextList(void)
{
	u8* base = gjCurShapeState->base;

	base += gjCurShapeState->total;
	gjCurShapeState->base  = base;
	gjCurShapeState->list0 = (void**)base;
	gjCurShapeState->list1 = (void**)(base + gjCurShapeState->size);
	gjCurShapeState->index = 0;
}

// 0x802181E4
void gjShapeBegin(GjShape* shape, f32 frame)
{
	gjCurShapeState->base  = shape->base;
	gjCurShapeState->_18   = shape->_04;
	gjCurShapeState->frame = frame;
	gjCurShapeState->index = 0;
	gjCurShapeState->type  = shape->type;
	gjShapeSetListCount(shape->nodes & ~0xC0);
}

// 0x80218244
void gjShapeSelectState(s32 index)
{
	gjCurShapeState = &gjShapeStates[index];
}

// 0x8021825C
void gjShapeActor(GjActor* actor, void* motion, GjShape* shape, f32 frame)
{
	njStartMotionObj(motion, frame);
	gjBeginCalcActor(actor);
	if (shape == NULL) {
		gjDrawMotionTree(actor->object);
	} else {
		gjShapeSelectState(0);
		gjShapeBegin(shape, frame);
		gjDrawShapeTree(actor->object);
	}
	gjEndCalcActor(actor);
}

// 0x802182E4
void gjDrawShapeTree(NJS_OBJECT* obj)
{
	do {
		njPushMatrixEx();
		njMotionTransformEx(obj);
		if (gjObjectCallback != NULL) {
			gjObjectCallback(obj);
		}
		if ((obj->evalflags & 0x80) == 0) {
			if (obj->model != NULL && (obj->evalflags & 8) == 0) {
				gjShapeApplyModel(obj->model);
			} else {
				gjShapeNextList();
			}
		} else if (obj->model != NULL && (obj->evalflags & 8) == 0) {
			gjCalcActor(obj->model);
		}
		if (obj->child != NULL) {
			gjDrawShapeTree(obj->child);
		}
		njPopMatrixEx();
		obj = obj->sibling;
	} while (obj != NULL);
}

// 0x802183A4
void gjShapeApplyModel(void* model)
{
	GjShapeState* s = gjCurShapeState;
	void* a;
	void* b;

	if (((u32)s->type & 0x10) != 0) {
		a = s->list0[s->index];
		if (a != NULL) {
			b = s->list1[s->index];
		}
		s->index++;
	} else {
		a = NULL;
	}
	if (a != NULL) {
		gjShapeBlendKeys(a, b, model, gjCurShapeState->frame);
	} else {
		gjCalcActorModel(model);
	}
	gjShapeNextList();
}

// 0x80218430
void gjShapeBlendKeys(void* a, void* b, void* model, f32 frame)
{
	f32 t;
	GjKey* key0;
	GjKey* key1;
	f32 rest;
	void* data0;
	void* data1;

	if (a != NULL) {
		gjKeyBracket(a, 8, (u32)b, frame, (void**)&key0, (void**)&key1, &t);
		rest  = 1.0f - t;
		data0 = key0->data;
		data1 = key1->data;
	}
	if (a != NULL) {
		gjShapeBlendVerts(model, data0, t, data1, rest);
	} else {
		gjCalcActorModel(model);
	}
}
