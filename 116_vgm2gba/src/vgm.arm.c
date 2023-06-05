#include "vgm.arm.h"

//---------------------------------------------------------------------------
ST_VGM Vgm;


//---------------------------------------------------------------------------
EWRAM_CODE void VgmInit(void)
{
	_Memset(&Vgm, 0x00, sizeof(ST_VGM));
}
//---------------------------------------------------------------------------
EWRAM_CODE void VgmPlay(u8* pFile, bool isLoop)
{
	VgmStop();
	VgmInit();

	Vgm.id     = VGM_ID_PLAY;
	Vgm.pBuf   = pFile;
	Vgm.pFile  = pFile;
	Vgm.isLoop = isLoop;

	REG_TM3CNT_L = 0xffff;
	REG_TM2CNT_L = 0xffff;
	REG_TM3CNT_H = TIMER_CASCADE    | TIMER_START | TIMER_IRQ;
	REG_TM2CNT_H = TIMER_FREQ_PER_1 | TIMER_START;
}
//---------------------------------------------------------------------------
IWRAM_CODE void VgmIntrTimer(void)
{
	if(Vgm.id == VGM_ID_STOP)
	{
		return;
	}

cmdChk:

	u8 cmd = *Vgm.pBuf++;

	if(cmd == VGM_CMD_WREG)
	{
		u8 adr = *Vgm.pBuf++;
		u8 dat = *Vgm.pBuf++;
		*(u8*)(REG_BASE + adr) = dat;

		goto cmdChk;
	}

	if(cmd == VGM_CMD_WAIT)
	{
		u16 tm2 = *Vgm.pBuf++;
		tm2    |= *Vgm.pBuf++ << 8;

		u16 tm3 = *Vgm.pBuf++;
		tm3    |= *Vgm.pBuf++ << 8;

		REG_TM3CNT_H = 0;
		REG_TM2CNT_H = 0;

		REG_TM3CNT_L = tm3;
		REG_TM2CNT_L = tm2;
		REG_TM3CNT_H = TIMER_CASCADE    | TIMER_START | TIMER_IRQ;
		REG_TM2CNT_H = TIMER_FREQ_PER_1 | TIMER_START;

		return;
	}

	if(cmd == VGM_CMD_EOM)
	{
		if(Vgm.isLoop == TRUE)
		{
			u32 loop = *Vgm.pBuf++;
			loop    |= *Vgm.pBuf++ << 8;
			loop    |= *Vgm.pBuf++ << 16;
			loop    |= *Vgm.pBuf++ << 24;

			TRACE("%x\n", loop);

			Vgm.pBuf = Vgm.pFile + loop;
		}
		else
		{
			VgmStop();
		}

		return;
	}


	TRACE("ERR\n");
}
//---------------------------------------------------------------------------
IWRAM_CODE void VgmStop(void)
{
	REG_TM3CNT_H = 0;
	REG_TM2CNT_H = 0;
	Vgm.id = VGM_ID_STOP;

	// REG_SOUNDCNT
	*(u8*)(REG_BASE + 0x84) = 0x00;

	// ch1
	*(u8*)(REG_BASE + 0x60) = 0x00;
	*(u8*)(REG_BASE + 0x62) = 0x00;
	*(u8*)(REG_BASE + 0x63) = 0x00;
	*(u8*)(REG_BASE + 0x64) = 0x00;
	*(u8*)(REG_BASE + 0x65) = 0x00;

	// ch2
	*(u8*)(REG_BASE + 0x68) = 0x00;
	*(u8*)(REG_BASE + 0x69) = 0x00;
	*(u8*)(REG_BASE + 0x6c) = 0x00;
	*(u8*)(REG_BASE + 0x6d) = 0x00;

	// ch3
	for(u32 i=0; i<0x10; i++)
	{
		*(u8*)(REG_BASE + 0x90 + i) = 0;
	}
	*(u8*)(REG_BASE + 0x70) = 0x00;
	*(u8*)(REG_BASE + 0x72) = 0x00;
	*(u8*)(REG_BASE + 0x74) = 0x80;

	// ch4
	*(u8*)(REG_BASE + 0x78) = 0x00;
	*(u8*)(REG_BASE + 0x79) = 0x00;
	*(u8*)(REG_BASE + 0x7c) = 0x00;
	*(u8*)(REG_BASE + 0x7d) = 0x00;

	// REG_SOUNDCNT
	*(u8*)(REG_BASE + 0x80) = 0x77;
	*(u8*)(REG_BASE + 0x81) = 0xff;
	*(u8*)(REG_BASE + 0x84) = 0x80;
}
