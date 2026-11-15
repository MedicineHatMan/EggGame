#ifndef _RESBANK_H
#define _RESBANK_H
#include "Ninja/ninja.h"
struct DrawTask;
#include "Ninja/njtypes.h"
#include "stage/Stage.h"
struct TexName;
struct TAniGreetMark;
struct TObject;
struct TexList;
struct TObjFxModel;

#include "core/types.h"
#include "io/ResFile.h"
#include "io/Resource.h"

// A resource bank an archive and its table of models motions and texture lists with texture files that can be added
// later

struct ResBankData {
	NJS_OBJECT** mModels;    // _00
	NJS_MOTION*** mMotions;  // _04 a motion list per entry
	NJS_TEXLIST** mTextures; // _08
	void* _0C;               // _0C
};

struct ResBankExtra {
	int mNo;             // _00
	ResBankExtra* mNext; // _04
	BinFile* mFile;      // _08
};

struct ResBank {
	TArc* mArc;           // _00
	ResBankData* mData;   // _04
	ResBankExtra* mExtra; // _08
};

struct FxBankFile {
	TArc* mArc; // _00
	int mBank;  // _04
};

struct Vec3f;

extern "C" {
void aniTexSwapEnd();
void aniTexSwapReset();
u32 findEggAhead(const Vec3f*, int, int, int, f32);
void fxBankLoad(FxBankFile*, const char*, int);
void fxBankLoadDefault(FxBankFile*, const char*);
void fxBankRelease(FxBankFile*);
}

extern "C" {
void modelPostTevAdd(u32);
void modelPostTexGenNormal(u32);
}

extern "C" {
void resBankLoad(ResBank*, const char*);
void resBankRelease(ResBank*);
void resBankAddTextures(ResBank*, int, const char*);
void resBankRemoveTextures(ResBank*, int);
NJS_OBJECT** resBankGetModels(ResBank*);
NJS_TEXLIST** resBankGetTextures(ResBank*);
void resBankCtor(ResBank*);
NJS_MOTION** resBankGetMotion(ResBank*, int);
void** resBankGetObjects(ResBank*);
}

extern "C" {
void aniTexSwapBegin(TexList*, int, int);
void fxModelMotion(f32, TObjFxModel*, void*);
TAniGreetMark* createTAniGreetMark(TObject*, const Vec3f*);
}

/** A model with its own draw task and a matrix */
class TObjFxModel : public TObject {
public:
	TObjFxModel(TObject* parent, void* model, void* textures, const f32* matrix);
	virtual ~TObjFxModel();

	virtual void draw();   // _0C fades out and then flags itself
	virtual void vfn_10(); // _10
	virtual void render(); // _1C

	NJS_OBJECT* mModel; // _20
	void* mActor;       // _24
	void* mTextures;    // _28
	u32 mState[8];      // _2C
	u32 _4C;            // _4C
	f32 mMatrix[12];    // _50
	DrawTask* mTask;    // _80
};

/** NJS_TEXNAME and NJS_TEXLIST ninja.h clashes with the GX prototypes above */
struct TexName {
	void* filename; // _00
	u32 attr;       // _04
	void* texaddr;  // _08
};

struct TexList {
	TexName* textures; // _00
	u32 nbTexture;     // _04
};

#endif
