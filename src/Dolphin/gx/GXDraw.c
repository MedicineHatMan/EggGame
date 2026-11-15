#include "Dolphin/gx.h"
#include "math.h"

static GXVtxDescList vcd[27];     // 0x802A1B70
static GXVtxAttrFmtList vat[27];  // 0x802A1C48

// 0x80271448
// The dodecahedron's faces five vertex indices each
static u32 dodecaFaces[12][5] = {
	{ 0, 12, 10, 11, 16 }, { 1, 17, 8, 9, 13 },   { 2, 14, 9, 8, 18 },  { 3, 19, 11, 10, 15 },
	{ 4, 14, 2, 3, 15 },   { 5, 12, 0, 1, 13 },   { 6, 17, 1, 0, 16 },  { 7, 19, 3, 2, 18 },
	{ 8, 17, 6, 7, 18 },   { 9, 14, 4, 5, 13 },   { 10, 12, 5, 4, 15 }, { 11, 19, 7, 6, 16 },
};

#define DA 0.809017f
#define DB 0.309017f
#define DC 0.5f

// 0x80271538
static f32 dodecaVerts[20][3] = {
	{ -DA, 0.0f, DB },  { -DA, 0.0f, -DB }, { DA, 0.0f, -DB },  { DA, 0.0f, DB },   { DB, -DA, 0.0f },
	{ -DB, -DA, 0.0f }, { -DB, DA, 0.0f },  { DB, DA, 0.0f },   { 0.0f, DB, -DA },  { 0.0f, -DB, -DA },
	{ 0.0f, -DB, DA },  { 0.0f, DB, DA },   { -DC, -DC, DC },   { -DC, -DC, -DC },  { DC, -DC, -DC },
	{ DC, -DC, DC },    { -DC, DC, DC },    { -DC, DC, -DC },   { DC, DC, -DC },    { DC, DC, DC },
};

// 0x80271628
static f32 octaVerts[6][3] = {
	{ 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f },
};

// 0x80271670
static u8 octaFaces[8][3] = {
	{ 0, 4, 2 }, { 1, 2, 4 }, { 0, 3, 4 }, { 1, 4, 3 }, { 0, 2, 5 }, { 1, 5, 2 }, { 0, 5, 3 }, { 1, 3, 5 },
};

#define IX 0.525731112119133606f
#define IZ 0.850650808352039932f

// 0x80271688
static f32 icosaVerts[12][3] = {
	{ -IX, 0.0f, IZ }, { IX, 0.0f, IZ },  { -IX, 0.0f, -IZ }, { IX, 0.0f, -IZ },
	{ 0.0f, IZ, IX },  { 0.0f, IZ, -IX }, { 0.0f, -IZ, IX },  { 0.0f, -IZ, -IX },
	{ IZ, IX, 0.0f },  { -IZ, IX, 0.0f }, { IZ, -IX, 0.0f },  { -IZ, -IX, 0.0f },
};

// 0x80271718
static u8 icosaFaces[20][3] = {
	{ 0, 4, 1 },  { 0, 9, 4 },  { 9, 5, 4 },  { 4, 5, 8 },  { 4, 8, 1 },  { 8, 10, 1 }, { 8, 3, 10 },
	{ 5, 3, 8 },  { 5, 2, 3 },  { 2, 7, 3 },  { 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 },
	{ 0, 1, 6 },  { 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 },  { 7, 2, 11 },
};

static inline void GetVertState(void)
{
	GXGetVtxDescv(vcd);
	GXGetVtxAttrFmtv(GX_VTXFMT3, vat);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
}

static inline void RestoreVertState(void)
{
	GXSetVtxDescv(vcd);
	GXSetVtxAttrFmtv(GX_VTXFMT3, vat);
}

static inline void normalize(f32 v[3])
{
	f32 d = dolsqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
}

static inline void DumpTriangle(f32 v0[3], f32 v1[3], f32 v2[3])
{
	GXBegin(GX_TRIANGLES, GX_VTXFMT3, 3);
	GXPosition3f32(v0[0], v0[1], v0[2]);
	GXNormal3f32(v0[0], v0[1], v0[2]);
	GXPosition3f32(v1[0], v1[1], v1[2]);
	GXNormal3f32(v1[0], v1[1], v1[2]);
	GXPosition3f32(v2[0], v2[1], v2[2]);
	GXNormal3f32(v2[0], v2[1], v2[2]);
	GXEnd();
}

/**
 * @note Address: 0x801F6714
 * @note Size: 0x3B4
 */
// 0x801F6714
static void Subdivide(u8 depth, f32 v0[3], f32 v1[3], f32 v2[3])
{
	f32 v01[3];
	f32 v12[3];
	f32 v20[3];
	u32 i;

	if (depth == 0) {
		DumpTriangle(v0, v1, v2);
		return;
	}

	for (i = 0; i < 3; i++) {
		v01[i] = v0[i] + v1[i];
		v12[i] = v1[i] + v2[i];
		v20[i] = v2[i] + v0[i];
	}

	normalize(v01);
	normalize(v12);
	normalize(v20);

	Subdivide(depth - 1, v0, v01, v20);
	Subdivide(depth - 1, v1, v12, v01);
	Subdivide(depth - 1, v2, v20, v12);
	Subdivide(depth - 1, v01, v12, v20);
}

/**
 * @note Address: 0x801F6AC8
 * @note Size: 0x6CC
 */
void GXDrawCylinder(u8 numEdges)
{
	s32 i;
	f32 top;
	f32 bottom;
	f32 x[100];
	f32 y[100];
	f32 angle;

	top    = 1.0f;
	bottom = -top;

	GetVertState();

	for (i = 0; i <= numEdges; i++) {
		angle = 3.1415927f * (2.0f * i) / numEdges;
		x[i]  = cosf(angle);
		y[i]  = sinf(angle);
	}

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT3, (u16)((numEdges + 1) * 2));
	for (i = 0; i <= numEdges; i++) {
		GXPosition3f32(x[i], y[i], bottom);
		GXNormal3f32(x[i], y[i], 0.0f);
		GXPosition3f32(x[i], y[i], top);
		GXNormal3f32(x[i], y[i], 0.0f);
	}
	GXEnd();

	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT3, (u16)(numEdges + 2));
	GXPosition3f32(0.0f, 0.0f, top);
	GXNormal3f32(0.0f, 0.0f, 1.0f);
	for (i = 0; i <= numEdges; i++) {
		GXPosition3f32(x[i], -y[i], top);
		GXNormal3f32(0.0f, 0.0f, 1.0f);
	}
	GXEnd();

	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT3, (u16)(numEdges + 2));
	GXPosition3f32(0.0f, 0.0f, bottom);
	GXNormal3f32(0.0f, 0.0f, -1.0f);
	for (i = 0; i <= numEdges; i++) {
		GXPosition3f32(x[i], y[i], bottom);
		GXNormal3f32(0.0f, 0.0f, -1.0f);
	}
	GXEnd();

	RestoreVertState();
}

/**
 * @note Address: 0x801F7194
 * @note Size: 0x218
 */
static void GXDrawCubeFace(f32 nx, f32 ny, f32 nz, f32 tx, f32 ty, f32 tz, f32 bx, f32 by, f32 bz,
                           GXAttrType binormal, GXAttrType texture)
{
	GXPosition3f32(0.57735026f * (nx + tx + bx), 0.57735026f * (ny + ty + by), 0.57735026f * (nz + tz + bz));
	GXNormal3f32(nx, ny, nz);
	if (binormal != GX_NONE) {
		GXNormal3f32(tx, ty, tz);
		GXNormal3f32(bx, by, bz);
	}
	if (texture != GX_NONE) {
		GXTexCoord2s8(1, 1);
	}

	GXPosition3f32(0.57735026f * (nx - tx + bx), 0.57735026f * (ny - ty + by), 0.57735026f * (nz - tz + bz));
	GXNormal3f32(nx, ny, nz);
	if (binormal != GX_NONE) {
		GXNormal3f32(tx, ty, tz);
		GXNormal3f32(bx, by, bz);
	}
	if (texture != GX_NONE) {
		GXTexCoord2s8(0, 1);
	}

	GXPosition3f32(0.57735026f * (nx - tx - bx), 0.57735026f * (ny - ty - by), 0.57735026f * (nz - tz - bz));
	GXNormal3f32(nx, ny, nz);
	if (binormal != GX_NONE) {
		GXNormal3f32(tx, ty, tz);
		GXNormal3f32(bx, by, bz);
	}
	if (texture != GX_NONE) {
		GXTexCoord2s8(0, 0);
	}

	GXPosition3f32(0.57735026f * (nx + tx - bx), 0.57735026f * (ny + ty - by), 0.57735026f * (nz + tz - bz));
	GXNormal3f32(nx, ny, nz);
	if (binormal != GX_NONE) {
		GXNormal3f32(tx, ty, tz);
		GXNormal3f32(bx, by, bz);
	}
	if (texture != GX_NONE) {
		GXTexCoord2s8(1, 0);
	}
}

/**
 * @note Address: 0x801F73AC
 * @note Size: 0x250
 */
void GXDrawCube(void)
{
	GXAttrType binormal;
	GXAttrType texture;

	GXGetVtxDesc(GX_VA_NBT, &binormal);
	GXGetVtxDesc(GX_VA_TEX0, &texture);

	GetVertState();

	if (binormal != GX_NONE) {
		GXSetVtxDesc(GX_VA_NBT, GX_DIRECT);
		GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_NBT, GX_NRM_NBT, GX_F32, 0);
	}
	if (texture != GX_NONE) {
		GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
		GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_TEX0, GX_TEX_ST, GX_S8, 0);
	}

	GXBegin(GX_QUADS, GX_VTXFMT3, 24);
	GXDrawCubeFace(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, binormal, texture);
	GXDrawCubeFace(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, binormal, texture);
	GXDrawCubeFace(0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, binormal, texture);
	GXDrawCubeFace(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, binormal, texture);
	GXDrawCubeFace(0.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, binormal, texture);
	GXDrawCubeFace(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, binormal, texture);
	GXEnd();

	RestoreVertState();
}

/**
 * @note Address: 0x801F75FC
 * @note Size: 0x114
 */
void GXDrawSphere1(u8 depth)
{
	s32 i;

	GetVertState();

	for (i = 19; i >= 0; i--) {
		Subdivide(depth, icosaVerts[icosaFaces[(u8)i][0]], icosaVerts[icosaFaces[(u8)i][1]],
		          icosaVerts[icosaFaces[(u8)i][2]]);
	}

	RestoreVertState();
}
