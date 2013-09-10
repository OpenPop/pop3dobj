/*
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

made by ALACN
alacn@bol.com.br
uhahaa@msn.com
http://www.strategyplanet.com/populous

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
*/


typedef signed char					SBYTE;
typedef unsigned char				UBYTE;
typedef signed short				SWORD;
typedef unsigned short				UWORD;
typedef signed long					SLONG;
typedef unsigned long				ULONG;



#pragma pack(push, 1)


struct Points3DWord
{
	SWORD							WX;
	SWORD							WY;
	SWORD							WZ;
};


struct TMapSrcCoords // 8 bytes
{
	SLONG							X;
	SLONG							Y;
};


struct ObjEditFacesInfo // 60 bytes
{
	SWORD							CurrNormalIdx;
	SWORD							TextureIdx;
	UWORD							Flags;
	UBYTE							NumPoints;
	SBYTE							PolyDrawMode;
	TMapSrcCoords					TmapSrcCoords[4]; // 32 bytes
	SWORD							PointOffsets[4]; // 8 bytes
	UWORD							BaseNormalIdxs[4]; // 8 bytes
	UBYTE							UVCursorInfo;
	UBYTE							General;
	SBYTE							BucketAdj;
	UBYTE							BldgConstructFlags;
};


struct ObjEditObjectsInfo // 54 bytes
{
	UWORD							Flags;
	SWORD							NumFaces;
	SWORD							NumPoints;
	SBYTE							BucketOffset;
	SBYTE							SeqNum;
	SLONG							EditorScale;
	SLONG							Scale;

	//struct ObjEditFacesInfo		*StartFace;
	//struct ObjEditFacesInfo		*EndFace;
	DWORD							StartFace;
	DWORD							EndFace;

	//struct Points3DWord			*StartPoint;
	//struct Points3DWord			*EndPoint;
	DWORD							StartPoint;
	DWORD							EndPoint;

	Points3DWord					ScaledMins;
	Points3DWord					ScaledMaxs;
	
	//SBYTE							ShapeIdxs[BLDG_MARKER_NUM_ORIENTS];
	SBYTE							ShapeIdxs[4];
	
	struct Points3DWord				ScaledOffsets;
};


#pragma pack(pop)
