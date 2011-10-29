#include "../include/atadisk.h"

#define BIT0 (1)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)
#define BIT8 (1<<8)
#define BIT9 (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)

#define DATA_PORT		BIT0
#define ATAPI_PORT		BIT1
#define SECTOR_COUNT	BIT2
#define LBA_LO			BIT3
#define LBA_MID			BIT4
#define LBA_HI			BIT5
#define DRIVE_PORT		BIT6
#define STATUS_PORT		BIT7

#define ERR				BIT0
#define	DRQ				BIT3
#define	SRV				BIT4
#define	DF				BIT5
#define	RDY				BIT6
#define	BSY				BIT7

/* This defines are to check the sepecifications of the disk.
	#TODO: check size!*/
#define IS_REMOVABLE(D) (D & BIT7) ? printf("Is removable\n") : printf("Is not removable\n")
#define IS_ATA_DRIVE(D) (D & BIT15) ? printf("Is not ATA\n") : printf("Is ATA\n")

#define DMA_SUP(D) (D & BIT8) ? printf("DMA is supported\n") : printf("DMA is not supported\n")
#define LBA_SUP(D) (D & BIT9) ? printf("LBA is supported\n") : printf("LBA is not supported\n")
#define DMA_QUEUED_SUP(D) (D & BIT1) ? printf("DMA QUEUED supported\n") : printf("DMA QUEUED is not supported\n")

#define SECTOR_SIZE 512

int  hdddebug = 0;

int  lastsect = 0;

void translateBytes(char * ans, unsigned short databyte);
void writeDataToRegister (int ata, char upper, char lower);

void _400ns() {
	_inb(0x3F6);
	_inb(0x3F6);
	_inb(0x3F6);
}

int _drq_wait() {
	
	// _400ns();

	int test;
	while ((test = _inb(0x1F7)) && 1) {
		if(!(test & BSY))
		{
			if(test & DRQ)	{
				break;
			} else if(test & (ERR | DF))
			{
				if(test & ERR)	{
					printf("OUCH ERROR! %d %d %d\n", !!(test & DF), !!(test & ERR), lastsect);	
					while(1);
				}

				return 1;
				break;
			}
		}
	}
	return 0;
}

int _next_io() {
	int test;
	// _400ns();
	while ((test = _inb(0x1F7)) && 1) {
		if(!(test & BSY))
		{
			if(test & (ERR | DF))	{
				if(test & ERR)	{
					printf("IO Error %d %d %d\n", !!(test & DF), !!(test & ERR), lastsect );
					while(1);
				}
				return -1;
				break;
			} else  {
				break;
			}
		}
	}
	return 0;
}

// To read N bytes from hard disk, must alloc N+1 bytes for ans, as N+1 byte is used to null-character
int _disk_read(int ata, char * ans, int numreads, unsigned int sector){
	

	ata= ATA0;
	_outw(0x3F6, BIT2);

	int i = 0;
	for(i = 0; i < SECTOR_SIZE * numreads; ++i)
	{
		ans[i] = 0;
	}

	lastsect = sector;
	
	// _next_io();
	int retry = 1;
	while(retry) {
		unsigned long long addr = ((unsigned long long)sector) & 0x00000000ffffffff;


		_outb(0x1F1, 0x00); 
		_outb(0x1F1, 0x00);
		_outb(0x1F2, 0x00); 
		_outb(0x1F2, numreads);
		_outb(0x1F3, (unsigned char)(addr >> 24));
		_outb(0x1F3, (unsigned char)addr);
		_outb(0x1F4, 0);
		_outb(0x1F4, (unsigned char)(addr >> 8));
		_outb(0x1F5, 0);
		_outb(0x1F5, (unsigned char)(addr >> 16));
		_outb(0x1F6, 0);
		_outb(0x1F7, 0x24);

		retry = _drq_wait() == -1;
	}
	
	int b;
	unsigned short data;
	int errors = 0;
	int c = 0;
	for(i = 0; i < numreads;)
	{
		// printf("r3\n");
		for(b=0 ; b < SECTOR_SIZE ; b+=2, c+=2){
			data = getDataRegister(ata);
			translateBytes(ans+c, data);
		}
		++i;
		if(i == numreads)	{
			break;
		}
		retry = 1;
		while(retry) {
			retry = _next_io() == -1;
		}
	}


	return 1;
}

// Translate one word into two char
void translateBytes(char * ans, unsigned short databyte){	
	ans[0] = databyte & 0x000000FF;
	ans[1] = (databyte >> 8) & 0x000000FF;
}




// Writes to the ata chosen the msg received the ammount of bytes requested starting from the secto chose + the offset
int _disk_write(int ata, char * msg, int numreads, unsigned int sector){
	

	// Sti();
	//_Halt();
	_outw(0x3F6, BIT2);
	
	lastsect = sector;
	// _next_io();

	ata=ATA0;
	int i = 0;

	int retry = 1;
	while(retry) {
	unsigned long long addr = ((unsigned long long)sector) & 0x00000000ffffffff;

	_outb(0x1F1, 0x00); 
	_outb(0x1F1, 0x00);
	_outb(0x1F2, 0x00); 
	_outb(0x1F2, numreads);
	_outb(0x1F3, (unsigned char)(addr >> 24));
	_outb(0x1F3, (unsigned char)addr);
	_outb(0x1F4, (unsigned char)(addr >> 32));
	_outb(0x1F4, (unsigned char)(addr >> 8));
	_outb(0x1F5, (unsigned char)(addr >> 40));
	_outb(0x1F5, (unsigned char)(addr >> 16));
	_outb(0x1F6, 0);
	_outb(0x1F7, 0x34);
	
		// printf("r1 %d\n", sector);
		retry = _drq_wait() == -1;
	}


	// printf("Write wait...\n");
	
	// printf("w1\n");
	_drq_wait();
	// printf("w2\n");
	
	// while (!(_inb(0x1F7) & DRQ)) {}	


	// Now write all the sector
	int b;
	int c = 0;
	for(i = 0; i < numreads;)
	{
		// printf("w3\n");
		for (b=0; b<SECTOR_SIZE; b+=2, c+=2) {
			writeDataToRegister(ata, msg[c+1], msg[c]);
		}
		if(i == numreads)	{
			break;
		}
		++i;
		retry = 1;
		while(retry) {
			retry = _next_io() == -1;
		}
	}
	// Cli();
	return 1;
}


void writeDataToRegister (int ata, char upper, char lower) {
	unsigned short out = 0;
	out = ((upper << 8) & 0xFF00) | (lower & 0xFF);
	
	_outw(ata + WIN_REG0, out);	
}

unsigned short getDataRegister (int ata) {
	unsigned short ans;
	ans = _inw(ata + WIN_REG0);
	return ans;
}

void identifyDevice (int ata) {
	_out(ata + WIN_REG6, 0xA0);
	_out(ata + WIN_REG7, 0xEC);
}

// Check disk features
void check_drive(int ata) {
	ata = ATA0;
    identifyDevice(ata);

	unsigned short data = 0;
	unsigned short sector=12;
	int offset=0;
	int count=512;
	_next_io();
    int i;
 	short _60, _61, _100, _101, _102, _103;

    for(i=0;i<255;i++){
        data = getDataRegister(ata);
		if(i >= 27 && i <= 46)
		{
			if(data > 0)
			{
				printf("%c%c", (data & 0xFF00) >> 8, data & 0xFF);
			}
		}

		if(i == 60)
			_60 = data;
		if(i == 61)
			_61 = data;
    }
	printf("\n");
	printf("%d %d %d MB addressable\n", _61, _60, (_61 << 14 + _60) / 512);
}
