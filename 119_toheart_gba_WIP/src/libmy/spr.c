#include "spr.h"
#include "mem.h"
#include "sjis.h"
#include "../res.h"


//---------------------------------------------------------------------------
ST_SPR Spr EWRAM_BSS;


//---------------------------------------------------------------------------
EWRAM_CODE void SprInit(void)
{
	MemClear(&Spr, sizeof(ST_SPR));
	MemClear(BITMAP_OBJ_BASE_ADR, 0x20 * 512);

	// cursor spr chr + pal
	MemInc((u16*)spr_itemTiles, BITMAP_OBJ_BASE_ADR + SPR_MAX_DAT_SIZE, spr_itemTilesLen);
	MemInc((u16*)spr_itemPal, OBJ_COLORS, spr_itemPalLen);

	// font buffer
	SprSetChr(0,  18,  22,  512, ATTR0_SQUARE, ATTR1_SIZE_64);
	SprSetChr(1,  82,  22,  576, ATTR0_SQUARE, ATTR1_SIZE_64);
	SprSetChr(2, 146,  22,  640, ATTR0_SQUARE, ATTR1_SIZE_64);
	SprSetChr(3, 210,  22,  704, ATTR0_TALL,   ATTR1_SIZE_32);
	SprSetChr(4, 210,  54,  712, ATTR0_TALL,   ATTR1_SIZE_32);

	SprSetChr(5,  18,  86,  720, ATTR0_SQUARE, ATTR1_SIZE_64);
	SprSetChr(6,  82,  86,  784, ATTR0_SQUARE, ATTR1_SIZE_64);
	SprSetChr(7, 146,  86,  848, ATTR0_SQUARE, ATTR1_SIZE_64);
	SprSetChr(8, 210,  86,  912, ATTR0_TALL,   ATTR1_SIZE_32);
	SprSetChr(9, 210, 118,  920, ATTR0_TALL,   ATTR1_SIZE_32);

	// cursor
	SprSetChr(10, 240, 160, 936, ATTR0_WIDE,   ATTR1_SIZE_8);
	SprSetChr(11, 240, 160, 938, ATTR0_WIDE,   ATTR1_SIZE_8);


	SprSetImgWhite();
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprExec(void)
{
	if(Spr.isDrawOam == true)
	{
		MemInc(Spr.oam, OAM, sizeof(Spr.oam));

		Spr.isDrawOam = false;
	}

	if(Spr.isDrawDat == true)
	{
		MemInc(Spr.dat, BITMAP_OBJ_BASE_ADR, sizeof(Spr.dat));

		Spr.isDrawDat = false;
	}
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprSetChr(u32 no, u32 x, u32 y, u16 tile, u16 shape, u16 size)
{
	Spr.oam[no].d0 = (y & 0x00ff) | ATTR0_COLOR_16 | shape;
	Spr.oam[no].d1 = (x & 0x01ff) | size;
	Spr.oam[no].d2 = tile | ATTR2_PRIORITY(0);

	Spr.isDrawOam = true;
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprShow(u32 no)
{
	Spr.oam[no].d0 &= ~ATTR0_DISABLED;

	Spr.isDrawOam = true;
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprHide(u32 no)
{
	Spr.oam[no].d0 |= ATTR0_DISABLED;

	Spr.isDrawOam = true;
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprMove(u32 no, u32 x, u32 y)
{
	Spr.oam[no].d0 &= 0xff00;
	Spr.oam[no].d1 &= 0xfe00;

	Spr.oam[no].d0 |= (y & 0x00ff);
	Spr.oam[no].d1 |= (x & 0x01ff);

	Spr.isDrawOam = true;
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprClearDat(void)
{
	MemClear(Spr.dat, SPR_MAX_DAT_SIZE);

	Spr.isDrawDat = true;
}
//---------------------------------------------------------------------------
IWRAM_CODE void SprDrawDatChr(u32 x, u32 y, u16 code)
{
	u16* pS = SjisGetImgPointer(code);
	s32 ix, iy;

	for(iy=0; iy<10; iy++)
	{
		for(ix=0; ix<3; ix++)
		{
			u16* pD = (u16*)(Spr.dat + tbl_spr1d_bin[(x * 3) + ix + (y * 52 * 12) + (iy * 52)]);

			*pD++ = *pS++ & Spr.mask;
		}
	}

	Spr.isDrawDat = true;
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprSetImgWhite(void)
{
	Spr.mask = SPR_FONT_MASK_WHITE;
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprSetImgGray(void)
{
	Spr.mask = SPR_FONT_MASK_GRAY;
}
//---------------------------------------------------------------------------
EWRAM_CODE bool SprIsImgWhite(void)
{
	return (Spr.mask == SPR_FONT_MASK_WHITE) ? true : false;
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprShowCursor(void)
{
	SprShow(10);
	SprShow(11);
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprHideCursor(void)
{
	SprHide(10);
	SprHide(11);
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprMoveCursor(u32 x, u32 y)
{
	u32 sx = 18 + x * (SPR_FONT_IMG_CX);
	u32 sy = 22 + y * (SPR_FONT_IMG_CY + SPR_FONT_IMG_BLANK_CY);

	SprMove(10, sx, sy);
	SprMove(11, sx, sy+8);
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprShowWindow(void)
{
	s32 i;

	for(i=0; i<10; i++)
	{
		SprShow(i);
	}
}
//---------------------------------------------------------------------------
EWRAM_CODE void SprHideWindow(void)
{
	s32 i;

	for(i=0; i<10; i++)
	{
		SprHide(i);
	}
}
