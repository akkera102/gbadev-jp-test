#include "mem.h"


//---------------------------------------------------------------------------
ST_MEM Mem;


//---------------------------------------------------------------------------
EWRAM_CODE void MemInit(void)
{
	_Memset(&Mem, 0x00, sizeof(ST_MEM));
}
//---------------------------------------------------------------------------
EWRAM_CODE void MemInc(void* src, void* dst, u32 size)
{
	if((size & 0x3) == 0)
	{
		CpuSet(src, dst, (size / 4) | CPUSET_32BIT);
	}
	else
	{
		CpuSet(src, dst, (size / 2));
	}
}
//---------------------------------------------------------------------------
EWRAM_CODE void MemFix(void* src, void* dst, u32 size)
{
	if((size & 0x3) == 0)
	{
		CpuSet(src, dst, (size / 4) | CPUSET_32BIT | CPUSET_SRC_FIX);
	}
	else
	{
		CpuSet(src, dst, (size / 2) | CPUSET_SRC_FIX);
	}
}
//---------------------------------------------------------------------------
EWRAM_CODE void MemClear(void* dst, u32 size)
{
	MemFix(&Mem.dummy, dst, size);
}
