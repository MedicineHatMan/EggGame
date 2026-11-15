#ifndef _ANIMAL_ANIMODEL_H
#define _ANIMAL_ANIMODEL_H
struct AniModelFile;
struct AniModelObject;
struct AniModelTextures;

#include "core/types.h"

// A Ninja model as AniModel reads it
struct AniModelObject {
	u32 mFlags; // _00 0x2000 = skinned
};

// The textures of the export
struct AniModelTextures {
	void* mList; // _00
};

// The AniModel export
struct AniModelFile {
	struct AniTexList {
		struct Entry {
			u8 _00[8];
			void* mTexture; // _08 set once the texture is in
		}* mEntries; // _00
		int mCount;  // _04
	};

	AniModelObject** mObject;  // _00
	void* mMotions;            // _04
	void** mEggObject;         // _08
	void** mEggTexList;        // _0C
	f32** mEggScale;           // _10
	union {
		AniModelTextures* mTextures; // _14
		AniTexList** mTexList;       // _14 as the egg creature reads it
	};
};

#endif
