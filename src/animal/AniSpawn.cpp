#include "animal/AniMount.h"
#include "animal/AniModelManager.h"
#include "egg/TEggCreatureTypeE.h"
#include "animal/TAniCamelOstrich.h"
#include "animal/TAniHorseshoeCrabShark.h"
#include "animal/TAniLionKangaroo.h"
#include "core/TMainTask.h"

struct AniSpawnEntry {
	int mKind;                         // _00 -1 ends the table
	void* (*mCreate)(TObject* parent); // _04
};

static const AniSpawnEntry sMounts[] = {
	{ 0x17, createCamelOstrich },
	{ 0x16, createHorseshoeCrabShark },
	{ 0x15, createLionKangaroo },
	{ -1, NULL },
};

// 0x801C09FC
extern "C" void aniPreload(int kind)
{
	aniModelRequest(kind);
}

// 0x801C0A1C
extern "C" TEggCreature* aniSpawn(int kind, int eggId, Vec3f* pos)
{
	for (int i = 0; sMounts[i].mKind != -1; i++) {
		if (kind == sMounts[i].mKind) {
			TEggCreature* animal = (TEggCreature*)sMounts[i].mCreate(gTL_05);
			if (animal != NULL) {
				animal->mEggId  = eggId;
				animal->mPlayer = findObjById(eggId)->mPlayer;
				animal->setPose(pos, 0);
			}
			return animal;
		}
	}
	TEggCreature* animal = aniCreate(eggId, kind, pos);
	if (animal != NULL) {
		animal->mEggId = eggId;
	}
	return animal;
}
