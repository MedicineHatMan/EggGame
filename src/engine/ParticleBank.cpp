#include "engine/ParticleBank.h"
#include "io/ResFile.h"
#include "io/Resource.h"
#include "stage/Stage.h"
#include "Ninja/ninja.h"
#include "camera/TCamEditor.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/rand.h"
#include "io/ResBank.h"

extern "C" {

const char* sParticleCommonName[2] = { "ptcl2_common.bin", 0 }; // 0x802C3C90

ParticleBank sParticleBanks[2];  // 0x8027FAD0
BinFile* sParticleFiles[2];      // 0x802C4D98
ParticleDesc* sParticleDescs[2]; // 0x802C4DA0
int sParticleDescCounts[2];      // 0x802C4DA8
int sParticleTexBanks[2];        // 0x802C4DB0
}

/**
 * The start of a ptcl2 .bin file. The records follow it.
 * @size 0x20
 */
struct ParticleFileHeader {
	u32 _00;
	u32 mSize; // _04 of the records
	u8 _08[0x18];
};

static void bindParticleTextures(int slot);

// 0x8003FA98
static void particleBankLoad(ParticleBank* bank, const char* arc, const char* gvm)
{
	resBankLoad(&bank->mBank, arc);
	resBankAddTextures(&bank->mBank, 0, gvm);
	int count;
	NJS_TEXLIST* list;
	list          = *resBankGetTextures(&bank->mBank);
	count         = list->nbTexture;
	bank->mTexIds = new u32[count];
	for (int i = 0; i < count; i++) {
		bank->mTexIds[i] = (u32)list->textures[i].texaddr->hdr->texaddr;
	}
	bank->mTexCount = count;
}

// 0x8003FB34
static void particleBankRelease(ParticleBank* bank)
{
	if (bank->mTexIds != 0) {
		delete[] bank->mTexIds;
		bank->mTexIds = 0;
	}
	resBankRemoveTextures(&bank->mBank, 0);
	resBankRelease(&bank->mBank);
	bank->mTexCount = 0;
}

// 0x8003FB90
static NJS_TEXLIST* particleBankGetTexList(ParticleBank* bank)
{
	return *resBankGetTextures(&bank->mBank);
}

// 0x8003FBB4
// Copies name up to its '.' then '.' and up to three characters of ext into out max bytes with the terminator
static void swapExtension(const char* name, const char* ext, char* out, int max)
{
	const char* p = name;
	int len       = 0;
	int last      = max - 1;
	for (; *p != '\0'; p++) {
		if (*p == '.') {
			break;
		}
		len++;
	}
	if (len > last) {
		len = last;
	}

	char* o = out;
	int n   = 0;
	for (int k = 0; k < len; k++) {
		*o++ = *name++;
		n++;
	}
	if (n < last) {
		out[n++] = '.';
	}

	int i = 0;
	while (n < last) {
		if (*ext == '\0') {
			break;
		}
		i++;
		out[n] = *ext;
		n++;
		ext++;
		if (i >= 3) {
			break;
		}
	}
	out[n] = '\0';
}

static void loadParticleTextures(const char* name, int slot, int texBank);
static void loadParticleDescs(const char* name, int slot);

// 0x8003FC80
extern "C" void loadParticles(const char* name)
{
	loadParticleTextures(sParticleCommonName[0], 0, 0);
	loadParticleTextures(name, 1, 1);
	loadParticleDescs(sParticleCommonName[0], 0);
	loadParticleDescs(name, 1);
}

// 0x8003FCE0
static void loadParticleTextures(const char* name, int slot, int texBank)
{
	char arc[0x88];
	char gvm[0x88];

	swapExtension(name, "arc", arc, 0x80);
	swapExtension(name, "gvm", gvm, 0x80);
	particleBankLoad(&sParticleBanks[slot], arc, gvm);
	spriteSetTexList(texBank, particleBankGetTexList(&sParticleBanks[slot]));
	sParticleTexBanks[slot] = slot;
}

// 0x8003FD74
static void loadParticleDescs(const char* name, int slot)
{
	loadSizedResource(name, &sParticleFiles[slot]);
	ParticleFileHeader* file  = (ParticleFileHeader*)sParticleFiles[slot]->mData;
	u32 size                  = file->mSize;
	sParticleDescs[slot]      = (ParticleDesc*)(file + 1);
	sParticleDescCounts[slot] = size / sizeof(ParticleDesc);
	bindParticleTextures(slot);
}

// 0x8003FDE0
extern "C" void freeParticles()
{
	for (int i = 1; i >= 0; i--) {
		releaseResource(sParticleFiles[i]);
		sParticleFiles[i]      = 0;
		sParticleDescs[i]      = 0;
		sParticleDescCounts[i] = 0;
	}
	for (int i = 1; i >= 0; i--) {
		particleBankRelease(&sParticleBanks[i]);
	}
}

// 0x8003FE60
// Finds each record's texture in the slot's list by its global index
static void bindParticleTextures(int slot)
{
	ParticleDesc* descs = getParticleDescs(slot);
	ParticleDesc* desc;
	int texCount;
	u32* ids;
	int i;
	if (descs != 0) {
		int count = sParticleDescCounts[slot];
		particleBankGetTexList(&sParticleBanks[slot]);
		texCount = sParticleBanks[slot].mTexCount;
		ids      = sParticleBanks[slot].mTexIds;
		for (i = 0; i < count; i++) {
			desc              = &descs[i];
			desc->mTex.mBank  = slot;
			desc->mTex.mIndex = 0;
			u8 index          = 0;
			for (int j = 0; j < texCount; j++) {
				if (desc->mTexId == ids[j]) {
					desc->mTex.mBank  = slot;
					desc->mTex.mIndex = index;
					break;
				}
				index++;
			}
		}
	}
}

// 0x8003FF30
static int particleSlotLoaded(int slot)
{
	if (slot >= 0 && slot < 2) {
		return (bool)(sParticleDescs[slot] != 0);
	}
	return false;
}

// 0x8003FF64
static int particleDescValid(int slot, int id)
{
	if (!particleSlotLoaded(slot)) {
		return 0;
	}
	bool valid = false;
	if (id >= 0 && id < sParticleDescCounts[slot]) {
		valid = true;
	}
	return valid;
}

// 0x8003FFCC
extern "C" ParticleDesc* getParticleDesc(int slot, int id)
{
	if (!particleDescValid(slot, id)) {
		return 0;
	}
	return &sParticleDescs[slot][id];
}

// 0x80040020
extern "C" ParticleDesc* getParticleDescs(int slot)
{
	return getParticleDesc(slot, 0);
}

// 0x80040044
extern "C" int getParticleSlotCount()
{
	return 2;
}

// 0x8004004C
extern "C" int getParticleDescCount(int slot)
{
	if (!particleSlotLoaded(slot)) {
		return 0;
	}
	return sParticleDescCounts[slot];
}

// 0x80040094
// A random point in the box of half-size mRadius mRadiusY mRadiusZ
extern "C" void particleRandomOffset(const ParticleDesc* desc, Vec3f* out)
{
	out->x = desc->mRadius * (2.0f * (rand() * (1.0f / 32768.0f) - 0.5f));
	out->y = desc->mRadiusY * (2.0f * (rand() * (1.0f / 32768.0f) - 0.5f));
	out->z = desc->mRadiusZ * (2.0f * (rand() * (1.0f / 32768.0f) - 0.5f));
}

// 0x80040180
extern "C" void setSpriteMode(u16* mode, int blend, int anim, int flag)
{
	*mode = (blend & 3) | ((anim << 2) & 0xC) | ((flag << 4) & 0x10);
}

// 0x80040194
extern "C" int getSpriteAnim(const u16* mode)
{
	return (*mode >> 2) & 3;
}
