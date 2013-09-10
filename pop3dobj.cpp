/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

made by ALACN
alacn@bol.com.br
uhahaa@msn.com
http://www.strategyplanet.com/populous

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
*/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "pop.h"
#include "3ds.h"

#ifdef _DEBUG
//#include <assert.h>
#define assert(exp) {if(!(exp)){__asm{ int 3 }}}
#endif



#define STR_SIZE	256



struct PART_UV
{
	float			u,
					v;
};


struct PART_FACE
{
	PART_FACE		*Next,
					*Prev;
	BYTE			PointsCount;
	_3DS_POINT		*Points;
	PART_UV			*UV;
};


struct PART
{
	PART			*Next,
					*Prev;
	PART_FACE		*Child;
	signed short	TextureIdx;
};


void ErrorMsg(char *msg)
{
	MessageBox(0, msg, "by ALACN", MB_ICONHAND);
}


bool ExtractTo3ds(char *objects, char *faces, char *points, char *destpath, char *prefix)
{
	bool rs;
	char str[STR_SIZE];

	ObjEditObjectsInfo		*Objs = 0;
	ObjEditFacesInfo		*Facs = 0;
	Points3DWord			*Pnts = 0;
	DWORD					ObjsCount,
							FacsCount,
							PntsCount;

	HANDLE h;
	DWORD dwA, dwRW;

	// objs

	h = CreateFile(objects, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(h == INVALID_HANDLE_VALUE)
	{
		sprintf(str, "cannot open objs: %s", objects);
		ErrorMsg(str);
		goto failed;
	}

	dwA = SetFilePointer(h, 0, 0, 2);
	Objs = (ObjEditObjectsInfo*)malloc(dwA);
	ObjsCount = dwA / sizeof(ObjEditObjectsInfo);

	SetFilePointer(h, 0, 0, 0);
	dwRW = 0;
	ReadFile(h, Objs, dwA, &dwRW, 0);
	if(dwRW != dwA)
	{
		CloseHandle(h);
		sprintf(str, "objs read error: %s" , objects);
		ErrorMsg(str);
		goto failed;
	}

	CloseHandle(h);

	// facs

	h = CreateFile(faces, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(h == INVALID_HANDLE_VALUE)
	{
		sprintf(str, "cannot open facs: %s", faces);
		ErrorMsg(str);
		goto failed;
	}

	dwA = SetFilePointer(h, 0, 0, 2);
	Facs = (ObjEditFacesInfo*)malloc(dwA);
	FacsCount = dwA / sizeof(ObjEditFacesInfo);

	SetFilePointer(h, 0, 0, 0);
	dwRW = 0;
	ReadFile(h, Facs, dwA, &dwRW, 0);
	if(dwRW != dwA)
	{
		CloseHandle(h);
		sprintf(str, "facs read error", faces); 
		ErrorMsg(str);
		goto failed;
	}

	CloseHandle(h);

	// pnts

	h = CreateFile(points, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(h == INVALID_HANDLE_VALUE)
	{
		sprintf(str, "cannot open pnts: %s", points);
		ErrorMsg(str);
		goto failed;
	}

	dwA = SetFilePointer(h, 0, 0, 2);
	Pnts = (Points3DWord*)malloc(dwA);
	PntsCount = dwA / sizeof(Points3DWord);

	SetFilePointer(h, 0, 0, 0);
	dwRW = 0;
	ReadFile(h, Pnts, dwA, &dwRW, 0);
	if(dwRW != dwA)
	{
		CloseHandle(h);
		sprintf(str, "pnts read error: %s", points);
		ErrorMsg(str);
		goto failed;
	}

	CloseHandle(h);

	for(dwA = 0; dwA < ObjsCount; dwA++)
	{
		if(!Objs[dwA].NumFaces) continue;
		if(!Objs[dwA].NumPoints) continue;

		CHUNK root;
		memset(&root, 0, sizeof(root));
		root.hdr.id = M3DMAGIC;

		CHUNK *mdata;
		mdata = new CHUNK;
		memset(mdata, 0, sizeof(CHUNK));
		mdata->hdr.id = MDATA;
		LINK(root.Child, mdata);

		PART		*IdxList = 0,
					*p;
		PART_FACE	*pp;

#ifdef _DEBUG
		assert(Objs[dwA].StartFace < FacsCount);
		assert(Objs[dwA].StartPoint < PntsCount);
		assert(Objs[dwA].StartFace > 0);
		assert(Objs[dwA].StartPoint > 0);
#endif

		DWORD dwEF = Objs[dwA].StartFace + Objs[dwA].NumFaces - 1;
		for(DWORD dwF = Objs[dwA].StartFace - 1; dwF < dwEF; dwF++)
		{
#ifdef _DEBUG
			assert(dwF < FacsCount);
#endif
			if(Facs[dwF].NumPoints != 3 && Facs[dwF].NumPoints != 4) continue;

			PART *partidx = 0;

			p = IdxList;

			if(p) do
			{
				if(p->TextureIdx == Facs[dwF].TextureIdx)
				{
					partidx = p;
					break;
				}
				p = p->Next;
			}
			while(p != IdxList);

			if(!partidx)
			{
				partidx = new PART;
				memset(partidx, 0, sizeof(PART));
				partidx->TextureIdx = Facs[dwF].TextureIdx;
				LINK(IdxList, partidx);
			}

			pp = new PART_FACE;
			memset(pp, 0, sizeof(PART_FACE));
			LINK(partidx->Child, pp);

			pp->PointsCount = Facs[dwF].NumPoints;

			DWORD a;

			a = sizeof(_3DS_POINT) * pp->PointsCount;
			pp->Points = (_3DS_POINT*)malloc(a);
			memset(pp->Points, 0, a);

			a = sizeof(PART_UV) * pp->PointsCount;
			pp->UV = (PART_UV*)malloc(a);
			memset(pp->UV, 0, a);

			for(a = 0; a < pp->PointsCount; a++)
			{
#ifdef _DEBUG
				assert((Objs[dwA].StartPoint + Facs[dwF].PointOffsets[a] - 1) < PntsCount);
#endif
				pp->Points[a].x = (float)Pnts[Objs[dwA].StartPoint + Facs[dwF].PointOffsets[a] - 1].WX / ((float)Objs[dwA].Scale * 3.0f);
				pp->Points[a].y = (float)Pnts[Objs[dwA].StartPoint + Facs[dwF].PointOffsets[a] - 1].WY / ((float)Objs[dwA].Scale * 3.0f);
				pp->Points[a].z = (float)Pnts[Objs[dwA].StartPoint + Facs[dwF].PointOffsets[a] - 1].WZ / ((float)Objs[dwA].Scale * 3.0f);
				pp->UV[a].u = (float)Facs[dwF].TmapSrcCoords[a].X / (float)0x200000;
				pp->UV[a].v = 1.0f - ((float)Facs[dwF].TmapSrcCoords[a].Y / (float)0x200000);
			}
		}

		DWORD n = 0;

		p = IdxList;
		if(p) do
		{
			CHUNK *c, *c2, *c3, *c4, *matname;
			char  str[STR_SIZE];

			// mat entry

			c = new CHUNK;
			memset(c, 0, sizeof(CHUNK));
			c->hdr.id = MAT_ENTRY;
			LINK(mdata->Child, c);

			// mat entry - mat name

			c2 = new CHUNK;
			memset(c2, 0, sizeof(CHUNK));
			c2->hdr.id = MAT_NAME;
			LINK(c->Child, c2);

			matname = c2;

			sprintf(str, "t%d", p->TextureIdx);
			c2->data_size = strlen(str) + 1;
			c2->data = malloc(c2->data_size);
			memcpy(c2->data, str, c2->data_size);

			// mat entry - mat diffuse

			c2 = new CHUNK;
			memset(c2, 0, sizeof(CHUNK));
			c2->hdr.id = MAT_DIFFUSE;
			LINK(c->Child, c2);

			// mat entry - mat diffuse - color 24

			c3 = new CHUNK;
			memset(c3, 0, sizeof(CHUNK));
			c3->hdr.id = COLOR_24;
			LINK(c2->Child, c3);

			c3->data_size = sizeof(_3DS_MATERIAL_BYTE);
			c3->data = malloc(c3->data_size);
			((_3DS_MATERIAL_BYTE*)c3->data)->r =
			((_3DS_MATERIAL_BYTE*)c3->data)->g =
			((_3DS_MATERIAL_BYTE*)c3->data)->b = 0xFF;

			if(p->TextureIdx != -1 && p->TextureIdx < 256)
			{
				// mat entry - mat texmap

				c2 = new CHUNK;
				memset(c2, 0, sizeof(CHUNK));
				c2->hdr.id = MAT_TEXMAP;
				LINK(c->Child, c2);

				// mat entry - mat texmap - mat mapname

				c3 = new CHUNK;
				memset(c3, 0, sizeof(CHUNK));
				c3->hdr.id = MAT_MAPNAME;
				LINK(c2->Child, c3);

				sprintf(str, "tx%d.bmp", p->TextureIdx);
				c3->data_size = strlen(str) + 1;
				c3->data = malloc(c3->data_size);
				memcpy(c3->data, str, c3->data_size);
			}

			// named objects

			c = new CHUNK;
			memset(c, 0, sizeof(CHUNK));
			c->hdr.id = NAMED_OBJECT;
			LINK(mdata->Child, c);

			sprintf(str, "obj%d", n++);
			c->data_size = strlen(str) + 1;
			c->data = malloc(c->data_size);
			memcpy(c->data, str, c->data_size);

			// n tri object

			c2 = new CHUNK;
			memset(c2, 0, sizeof(CHUNK));
			c2->hdr.id = N_TRI_OBJECT;
			LINK(c->Child, c2);

			c = c2;

			// count

			WORD VCount = 0,
				 FCount = 0;

			pp = p->Child;
			if(pp) do
			{
				VCount += pp->PointsCount;

				if(pp->PointsCount == 3)
					FCount += 1;
				else //if(pp->PointsCount == 4)
					FCount += 2;

				pp = pp->Next;
			}
			while(pp != p->Child);

			// point array

			c2 = new CHUNK;
			memset(c2, 0, sizeof(CHUNK));
			c2->hdr.id = POINT_ARRAY;
			LINK(c->Child, c2);

			c2->data_size = sizeof(WORD) + (sizeof(_3DS_POINT) * VCount);
			c2->data = malloc(c2->data_size);
			memset(c2->data, 0, c2->data_size);

			*(WORD*)c2->data = VCount;
			_3DS_POINT *_3dspoint = (_3DS_POINT*)((BYTE*)c2->data + sizeof(WORD));

			// tex verts

			c3 = new CHUNK;
			memset(c3, 0, sizeof(CHUNK));
			c3->hdr.id = TEX_VERTS;
			LINK(c->Child, c3);

			c3->data_size = sizeof(WORD) + (sizeof(_3DS_UV) * VCount);
			c3->data = malloc(c3->data_size);
			memset(c3->data, 0, c3->data_size);

			*(WORD*)c3->data = VCount;
			_3DS_UV *_3dsuv = (_3DS_UV*)((BYTE*)c3->data + sizeof(WORD));

			// face array

			c4 = new CHUNK;
			memset(c4, 0, sizeof(CHUNK));
			c4->hdr.id = FACE_ARRAY;
			LINK(c->Child, c4);

			c4->data_size = sizeof(WORD) + (sizeof(_3DS_FACE) * FCount);
			c4->data = malloc(c4->data_size);
			memset(c4->data, 0, c4->data_size);

			*(WORD*)c4->data = FCount;
			_3DS_FACE *_3dsface = (_3DS_FACE*)((BYTE*)c4->data + sizeof(WORD));

			c2 = new CHUNK;
			memset(c2, 0, sizeof(CHUNK));
			c2->hdr.id = MSH_MAT_GROUP;
			LINK(c4->Child, c2);

			c2->data_size = matname->data_size + (sizeof(WORD) * (FCount + 1));
			c2->data = malloc(c2->data_size);
			memcpy(c2->data, matname->data, matname->data_size);

			WORD *w = (WORD*)((BYTE*)c2->data + matname->data_size);
			*w = FCount;
			w++;

			WORD wf;
			for(wf = 0; wf < FCount; wf++)
			{
				*w = wf;
				w++;
			}

			//

			wf = 0;
			WORD wp = 0;

			pp = p->Child;
			if(pp) do
			{
				//

				_3dspoint[wp].x = pp->Points[0].x;
				_3dspoint[wp].y = pp->Points[0].y;
				_3dspoint[wp].z = pp->Points[0].z;

				_3dsuv[wp].u = pp->UV[0].u;
				_3dsuv[wp].v = pp->UV[0].v;

				_3dsface[wf].a = wp;
				wp++;

				//

				_3dspoint[wp].x = pp->Points[1].x;
				_3dspoint[wp].y = pp->Points[1].y;
				_3dspoint[wp].z = pp->Points[1].z;

				_3dsuv[wp].u = pp->UV[1].u;
				_3dsuv[wp].v = pp->UV[1].v;

				_3dsface[wf].b = wp;
				wp++;

				//

				_3dspoint[wp].x = pp->Points[2].x;
				_3dspoint[wp].y = pp->Points[2].y;
				_3dspoint[wp].z = pp->Points[2].z;

				_3dsuv[wp].u = pp->UV[2].u;
				_3dsuv[wp].v = pp->UV[2].v;
				
				_3dsface[wf].c = wp;
				wf++;
				wp++;

				if(pp->PointsCount == 4)
				{
					_3dspoint[wp].x = pp->Points[3].x;
					_3dspoint[wp].y = pp->Points[3].y;
					_3dspoint[wp].z = pp->Points[3].z;

					_3dsuv[wp].u = pp->UV[3].u;
					_3dsuv[wp].v = pp->UV[3].v;

					_3dsface[wf].a = wp - 1;
					_3dsface[wf].b = wp;
					_3dsface[wf].c = wp - 3;
					wf++;
					wp++;
				}

				pp = pp->Next;
			}
			while(pp != p->Child);

			//

			p = p->Next;
		}
		while(p != IdxList);

		while(IdxList)
		{
			p = IdxList;
			UNLINK(IdxList, p);

			while(p->Child)
			{
				pp = p->Child;
				UNLINK(p->Child, pp);
				if(pp->Points) free(pp->Points);
				if(pp->UV) free(pp->UV);
				delete pp;
			}

			delete p;
		}

		char filename[STR_SIZE];
		sprintf(filename, "%s\\%s%d.3ds", destpath, prefix, dwA);

		if(!_3dsWriteFile(filename, &root))
		{
			sprintf(str, "_3ds Write Failed: %s", filename);
			ErrorMsg(str);
			_3dsDestroyChunk(&root);
			goto failed;
		}

		_3dsDestroyChunk(&root);
	}

	rs = true;
	goto skip;
failed:
	rs = false;
skip:
	if(Objs) free(Objs);
	if(Facs) free(Facs);
	if(Pnts) free(Pnts);

	return rs;
}


int __stdcall WinMain(HINSTANCE, HINSTANCE, char*, int)
{
	ExtractTo3ds("objs0-0.dat", "facs0-0.dat", "pnts0-0.dat", "3ds", "obj0_");
	ExtractTo3ds("objs0-1.dat", "facs0-1.dat", "pnts0-1.dat", "3ds", "obj1_");
	ExtractTo3ds("objs0-2.dat", "facs0-2.dat", "pnts0-2.dat", "3ds", "obj2_");
	ExtractTo3ds("objs0-3.dat", "facs0-3.dat", "pnts0-3.dat", "3ds", "obj3_");
	ExtractTo3ds("objs0-4.dat", "facs0-4.dat", "pnts0-4.dat", "3ds", "obj4_");
	ExtractTo3ds("objs0-5.dat", "facs0-5.dat", "pnts0-5.dat", "3ds", "obj5_");
	ExtractTo3ds("objs0-6.dat", "facs0-6.dat", "pnts0-6.dat", "3ds", "obj6_");
	ExtractTo3ds("objs0-7.dat", "facs0-7.dat", "pnts0-7.dat", "3ds", "obj7_");
	ExtractTo3ds("objs0-8.dat", "facs0-8.dat", "pnts0-8.dat", "3ds", "obj8_");

	MessageBox(0, "done", "by ALACN", 0);

	return 0;
}
