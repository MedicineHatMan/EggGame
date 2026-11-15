#ifndef _TENEANTSBOMB_H
#define _TENEANTSBOMB_H
struct CharaColli;

#include "object/ObjAction.h"
struct DrawTask;
#include "stage/SetManager.h"

// The bomb TEneAntsQueen lobs and the blast it leaves

extern "C" {
void objHeapFree(void*, void*);
extern void* gObjHeap;
}

/**
 * @size 0x48
 */
class TEneAntsBomb_Blast : public TObjAction {
public:
	TEneAntsBomb_Blast(TObject* parent, Vec3f* pos);
	virtual ~TEneAntsBomb_Blast();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);

	Vec3f mPos;   // _30
	f32 mRadius;  // _3C
	CharaColli* mColli; // _40
	int mTime;    // _44 from -8 It only hurts from 0
};

typedef char TEneAntsBomb_Blast_size_check[(sizeof(TEneAntsBomb_Blast) == 0x48) ? 1 : -1];

/**
 * @size 0x80
 */
class TEneAntsBomb : public TObjAction {
public:
	TEneAntsBomb(TObject* parent, Vec3f* pos, Vec3f* target, Vec3f* vel);
	virtual ~TEneAntsBomb();

	void operator delete(void* mem) { objHeapFree(gObjHeap, mem); }

	virtual void draw();
	virtual void vfn_10();
	virtual void render();
	virtual Vec3f* getPos();
	virtual int getKind();
	virtual int onHit(const TObjAction* other, int arg, int* result);

	SetTransform mBody; // _30
	DrawTask* mDrawTask;    // _74
	CharaColli* mColli;       // _78
	int mLife;          // _7C
};

typedef char TEneAntsBomb_size_check[(sizeof(TEneAntsBomb) == 0x80) ? 1 : -1];

extern "C" {
extern const char* gTEneAntsBombName;
extern const char* gTEneAntsBomb_BlastName;
}

#endif
