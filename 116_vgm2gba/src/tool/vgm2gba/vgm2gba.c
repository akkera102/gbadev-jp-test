#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//---------------------------------------------------------------------------

typedef struct {
	uint8_t* pBuf;
	uint32_t size;

} ST_VGM;

//---------------------------------------------------------------------------
void openFile(ST_VGM* p, char* filename)
{
	FILE* fp;
	fp = fopen(filename, "rb");

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	p->pBuf = malloc(size);

	fseek(fp,0,SEEK_SET);
	if(fread(p->pBuf, 1, size, fp) < 1)
	{
		printf("Error: vgm file fread\n");

		exit(EXIT_FAILURE);
	}
	fclose(fp);

	p->size = size;
}
//---------------------------------------------------------------------------
void checkHeader(ST_VGM* p)
{
	if(p->pBuf[0] != 'V' || p->pBuf[1] != 'g' || p->pBuf[2] != 'm')
	{
		printf("Error: vgm ident\n");

		exit(EXIT_FAILURE);
	}

	if(p->pBuf[0x08] != 0x61 || p->pBuf[0x09] != 0x01 || p->pBuf[0x10] != 0x00 || p->pBuf[0x11] != 0x00)
	{
		printf("Error: not version 1.61\n");

		exit(EXIT_FAILURE);
	}

	if(p->pBuf[0x80] != 0x00 || p->pBuf[0x81] != 0x00 || p->pBuf[0x82] != 0x40 || p->pBuf[0x83] != 0x00)
	{
		printf("Error: not use Game Boy\n");

		exit(EXIT_FAILURE);
	}
}
//---------------------------------------------------------------------------
void convertReg(ST_VGM* pVgm)
{
	uint8_t* p = pVgm->pBuf + 0xC0;

	// end of mark
	while(*p != 0x66)
	{
		// wait: 0x61 nn nn
		if(*p == 0x61)
		{
			p += 3;
			continue;
		}

		// write reg: 0xb3 aa dd
		if(*p == 0xb3)
		{
			p++;

			uint8_t adr;
			uint8_t dat;

			switch(*p + 0x10)
			{
			case 0x10: adr = 0x60; break;	// NR 10
			case 0x11: adr = 0x62; break;	// NR 11
			case 0x12: adr = 0x63; break;	// NR 12
			case 0x13: adr = 0x64; break;	// NR 13
			case 0x14: adr = 0x65; break;	// NR 14
			case 0x16: adr = 0x68; break;	// NR 21
			case 0x17: adr = 0x69; break;	// NR 22
			case 0x18: adr = 0x6c; break;	// NR 23
			case 0x19: adr = 0x6d; break;	// NR 24
			case 0x1a: adr = 0x70; break;	// NR 30
			case 0x1b: adr = 0x72; break;	// NR 31
			case 0x1c: adr = 0x73; break;	// NR 32
			case 0x1d: adr = 0x74; break;	// NR 33
			case 0x1e: adr = 0x75; break;	// NR 34
			case 0x20: adr = 0x78; break;	// NR 41
			case 0x21: adr = 0x79; break;	// NR 42
			case 0x22: adr = 0x7c; break;	// NR 43
			case 0x23: adr = 0x7d; break;	// NR 44
			case 0x24: adr = 0x80; break;	// NR 50
			case 0x25: adr = 0x81; break;	// NR 51
			case 0x26: adr = 0x84; break;	// NR 52

			// RAM
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			case 0x38:
			case 0x39:
			case 0x3a:
			case 0x3b:
			case 0x3c:
			case 0x3d:
			case 0x3e:
			case 0x3f:
				adr = 0x90 + (*p + 0x10) - 0x30;
				break;

			default:
				printf("Error: offset %x = %x\n", p - pVgm->pBuf, *p);
				exit(EXIT_FAILURE);
			}

			dat = *(p + 1);

			// NR 30
			if(adr == 0x70)
			{
				dat &= 0x80;
			}

			// NR 32
			if(adr == 0x73)
			{
				dat &= 0x60;
			}

			// NR 50
			if(adr == 0x80 && dat & 0x08)
			{
				printf("Warning: no use GBA bit. NR 50(FF24) Right Flag.\n");
			}

			// NR 50
			if(adr == 0x80 && dat & 0x80)
			{
				printf("Warning: no use GBA bit. NR 50(FF24) Left Flag.\n");
			}

			*p++ = adr;
			*p++ = dat;

			continue;
		}

		printf("Error: Commands. offset %x = %x\n", p - pVgm->pBuf, *p);
		exit(EXIT_FAILURE);
	}
}
//---------------------------------------------------------------------------
void saveFile(ST_VGM* pVgm, char* filename)
{
	char savname[0x100];

	int len = strlen(filename);

	strcpy(savname, filename);
	savname[len-4] = '\0';
	strcat(savname, ".bin");

	FILE* fp = fopen(savname, "wb");

	if(fp == NULL)
	{
		printf("Error: savname open\n");

		exit(EXIT_FAILURE);
	}

	uint8_t* p = pVgm->pBuf + 0xC0;

	// end of mark
	while(*p != 0x66)
	{
		// wait: 0x61 nn nn
		if(*p == 0x61)
		{
			uint8_t d1 = *p++;
			uint8_t d2 = *p++;
			uint8_t d3 = *p++;

			// GBA patch

			// sec = GBA clock / samples rate
			uint32_t sec  = (16 * 1024 * 1024) / 44100;

			// n samples
//			uint32_t samp = (d3 << 8) | d2;

			// TODO WIP 1.97 is irresponsible number. why is that? :(
			uint32_t samp = ((d3 << 8) | d2) * 1.972;

			// GBA timer2,3 + cascade
			uint64_t time = 0x100000000 - sec * samp;

			if(time == 0x100000000)
			{
				printf("Error: invalid time\n");

				exit(EXIT_FAILURE);
			}

			fputc(0x61, fp);
			fputc((uint8_t)(time >>  0), fp);
			fputc((uint8_t)(time >>  8), fp);
			fputc((uint8_t)(time >> 16), fp);
			fputc((uint8_t)(time >> 24), fp);

/*
			printf("sec: %x\n", sec);
			printf("samp: %x\n", samp);
			printf("time: %x\n", time);

			printf("time0: %x\n", (uint8_t)(time >>  0));
			printf("time1: %x\n", (uint8_t)(time >>  8));
			printf("time2: %x\n", (uint8_t)(time >> 16));
			printf("time3: %x\n", (uint8_t)(time >> 24));

//			exit(0);
*/
			continue;
		}

		// write reg: 0xb3 aa dd
		if(*p == 0xb3)
		{
			uint8_t d1 = *p++;
			uint8_t d2 = *p++;
			uint8_t d3 = *p++;

			fputc(d1, fp);
			fputc(d2, fp);
			fputc(d3, fp);

			// GBA patch

			// REG_SOUND3CNT_L = 0x40;
			if(d2 >= 0x90 && d2 <= 0x9f)
			{
				fputc(0xb3, fp);
				fputc(0x70, fp);
				fputc(0x40, fp);
			}

			continue;
		}
	}

	// write end of mark
	fputc(*p++, fp);

	// zero pading
	int pad = 0x10 - (ftell(fp) & 0xf);
	int i;

	for(i=0; i<pad; i++)
	{
		fputc(0x00, fp);
	}

	fclose(fp);
}
//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("input filename\n");
		exit(0);
	}

	printf("convert... %s\n", argv[1]);

	ST_VGM Vgm;
	memset(&Vgm, 0x00, sizeof(ST_VGM));

	openFile(&Vgm, argv[1]);
	checkHeader(&Vgm);
	convertReg(&Vgm);
	saveFile(&Vgm, argv[1]);

	free(Vgm.pBuf);

    return 0;
}