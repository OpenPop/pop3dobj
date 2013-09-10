/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

made by ALACN
alacn@bol.com.br
uhahaa@msn.com
http://www.strategyplanet.com/populous

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
*/


#define M3DMAGIC				0x4D4D
#define MDATA					0x3D3D
#define NAMED_OBJECT			0x4000
#define N_TRI_OBJECT			0x4100
#define POINT_ARRAY				0x4110
#define FACE_ARRAY				0x4120
#define MSH_MAT_GROUP			0x4130
#define TEX_VERTS				0x4140
#define MAT_ENTRY				0xAFFF
#define MAT_NAME				0xA000
#define MAT_DIFFUSE				0xA020
#define MAT_SPECULAR			0xA030
#define MAT_TEXMAP				0xA200
#define MAT_MAPNAME				0xA300
#define COLOR_24				0x0011



#define LINK(list, add) { if(list) { add->Prev = list->Prev; add->Next = list; list->Prev->Next = add; list->Prev = add; } else { list = add; add->Next = add->Prev = add; } }
#define UNLINK(list, del) { del->Next->Prev = del->Prev; del->Prev->Next = del->Next; if(list == del) { list = del->Next; if(list == del) list = 0; } del->Next = 0; del->Prev = 0; }



#pragma pack(push, 1)

struct _3DS_UV
{
	float						u,
								v;
};


struct _3DS_MATERIAL_BYTE
{
	BYTE						r,
								g,
								b;
};


struct _3DS_FACE
{
	WORD						a,
								b,
								c,
								flag;
};


struct _3DS_POINT
{
	float						x,
								y,
								z;
};


struct CHUNK_HDR
{
	WORD						id;
	DWORD						length;
};

#pragma pack(pop)


struct CHUNK
{
	CHUNK						*Next,
								*Prev,
								*Child;
	CHUNK_HDR					hdr;
	DWORD						offset,
								data_size;
	void						*data;
};



DWORD _3dsCalcChunkSize(CHUNK *root);
void _3dsDestroyChunk(CHUNK *root);
bool _3dsWriteChunk(HANDLE h, CHUNK *root);
bool _3dsWriteFile(char *name, CHUNK *root);
