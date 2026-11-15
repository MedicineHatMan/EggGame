#include "ui/TAeLayer.h"
#include "Ninja/ninja.h"
#include "sdk/Dolphin/gx/GXEnum.h"
#include "ui/AeSprite.h"

/**
 * @size 0x10
 */
struct AeSprite {
	u16 mTex;    // _00 index into the current texture list and the palette banks
	u16 mU;      // _02
	u16 mV;      // _04
	u16 mWidthU; // _06
	u16 mWidthV; // _08
	u16 mSizeX;  // _0A
	u16 mSizeY;  // _0C
	u16 _0E;     // _0E
};

// 0x800A623C
// banks gives the palette bank for each texture or NULL for direct colour
void aeDrawSprite(AeFrame* frame, const u32* banks)
{
	GjTexHeader* hdr;
	AeSprite* sprite = (AeSprite*)frame->mHead;
	int tlut         = -1;
	hdr              = njGetCurrentTexList()->textures[sprite->mTex].texaddr->hdr;
	if (banks != NULL) {
		tlut = banks[sprite->mTex];
	}

	NJS_POINT3 src[3];
	NJS_POINT3 dst[3];
	src[0].x = 0.0f;
	src[0].y = 0.0f;
	src[0].z = 0.0f;
	src[1].x = 0.0f;
	src[1].y = sprite->mSizeY;
	src[1].z = 0.0f;
	src[2].x = sprite->mSizeX;
	src[2].y = 0.0f;
	src[2].z = 0.0f;
	njCalcPoints((f32*)NULL, (const Vec3f*)src, (Vec3f*)dst, 3);
	for (int i = 0; i < 3; i++) {
		dst[i].z = frame->mRate;
	}

	f32 u0 = sprite->mU / 256.0f;
	f32 v0 = sprite->mV / 256.0f;
	f32 u1 = (int)(sprite->mU + sprite->mWidthU) / 256.0f;
	f32 v1 = (int)(sprite->mV + sprite->mWidthV) / 256.0f;

	u32 color = ((u32)(255.0f * frame->mColor.a) << 24) | (((u32)(255.0f * frame->mColor.r) & 0xff) << 16)
	          | (((u32)(255.0f * frame->mColor.g) & 0xff) << 8) | ((u32)(255.0f * frame->mColor.b) & 0xff);

	if (frame->mMark & 4) {
		GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
	}

	NJS_QUAD_TEXTURE_EX quad = sAeZeroQuad;
	quad.x                   = dst[0].x;
	quad.y                   = dst[0].y;
	quad.z                   = dst[0].z;
	quad.vx1                 = dst[1].x - dst[0].x;
	quad.vy1                 = dst[1].y - dst[0].y;
	quad.vx2                 = dst[2].x - dst[0].x;
	quad.vy2                 = dst[2].y - dst[0].y;
	quad.u                   = u0;
	quad.v                   = v0;
	quad.vu1                 = u0 - u0;
	quad.vv1                 = v1 - v0;
	quad.vu2                 = u1 - u0;
	quad.vv2                 = v0 - v0;

	GXTexObj obj;
	if (tlut != -1) {
		GXInitTexObjCI(&obj, hdr->info.image, hdr->info.width, hdr->info.height, hdr->info.format, GX_CLAMP, GX_CLAMP, GX_FALSE, tlut);
	} else {
		GXInitTexObj(&obj, hdr->info.image, hdr->info.width, hdr->info.height, hdr->info.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	}
	GXLoadTexObj(&obj, GX_TEXMAP0);
	njSetQuadTextureColor(color);
	njDrawQuadTextureEx(&quad);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
}
