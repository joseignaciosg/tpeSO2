#ifndef _AT_WINI_H_
#define _AT_WINI_H_


/*
//extern struct system_t System;



typedef struct disk_cmd{
	int ata;
	int sector;
	int offset;
	int count; // Bytes count
	char * buffer;
}disk_cmd;

typedef struct disk_cmd * disk_cmd_t; 

struct disk_t {
	void (*read)(int, char *, unsigned short, int, int);
	void (*write)(int, char *, int, unsigned short, int);
};
*/
/* I/O Ports used by winchester disk controller. */

#define ATA0 0x1f0
#define ATA1 0x170

#define WIN_REG0       0x0
#define WIN_REG1       0x1 // Error
#define WIN_REG2       0x2
#define WIN_REG3       0x3
#define WIN_REG4       0x4
#define WIN_REG5       0x5
#define WIN_REG6       0x6
#define WIN_REG7       0x7 // Command|Status
#define WIN_REG8       0x3f6




/* Winchester disk controller command bytes. */
#define WIN_IDENTIFY	0xEC
#define MEDIA_STATUS	0xDA
#define READ_DMA		0xC8
#define WIN_RECALIBRATE	0x10	/* command for the drive to recalibrate */
#define WIN_READ        0x20	/* command for the drive to read */
#define WIN_WRITE       0x30	/* command for the drive to write */
#define WIN_SPECIFY     0x91	/* command for the controller to accept params */

#define LBA_READ        WIN_READ
#define LBA_WRITE       WIN_WRITE

/* Parameters for the disk drive. */
#define SECTOR_SIZE      512	/* physical sector size in bytes */

/* Error codes */
#define ERR		  -1	/* general error */

/*
read example
		int i=0;
	for (i=0;i<450000;i++);
	i=25;
		char msg[2048];
	for(i=12;i<18;i+=4){
	unsigned short sector=i;
	int offset=0;
	int count=2048;
	int ata=ATA0;

	disk_read(ata, msg, sector, offset, count);
	//msg[0]='a';
	msg[511]='O';
	msg[1023]='O';
	msg[1535]='O';
	printf("res %d: %s.\n",i,msg);
}
}


write example

char ans[2048];
int j;
for(j=12;j<18;j+=4){
unsigned short sector = j;
int offset            = 0;
// int count             = 512;
int ata               = ATA0;
int i;
for (i = 0; i < 2047; i++) {
	ans[i++] = 'A';
	ans[i++] = 'X';
	ans[i++] = 'A';
	ans[i] = ' ';
}

ans[511]=0;
ans[1023]=0;
ans[1535]=0;
ans[2047]=0;

int bytes=2048;
disk_write(ata, ans, bytes, sector, offset);
printf("escribo: %s.\n",ans);	
}

*/

#define MAX_ERRORS         4	/* how often to try rd/wt before quitting */
#define NR_DEVICES        10	/* maximum number of drives */
#define MAX_WIN_RETRY  10000	/* max # times to try to output to WIN */
#define PART_TABLE     0x1C6	/* IBM partition table starts here in sect 0 */
#define DEV_PER_DRIVE      5	/* hd0 + hd1 + hd2 + hd3 + hd4 = 5 */


enum{
	READ_DISK = 0,
	WRITE_DISK,
	OK,
	ERROR
};

int driver(char * ata);

void sendComm(int ata, int rdwr, unsigned short sector);

void _disk_read(int ata, char * ans, int count, unsigned short sector, int offset);
unsigned short getDataRegister(int ata);

void _disk_write(int ata, char * msg, int bytes, unsigned short sector, int offset);
void writeDataToRegister(int ata, char upper, char lower);
void translateBytes(char ans[], unsigned short sector);

unsigned short getStatusRegister(int ata);
void identifyDevice(int ata);
void check_drive(int ata);
unsigned short getErrorRegister(int ata);



#endif

