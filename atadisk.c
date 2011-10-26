#include "atadisk.h"
#include "smdio.h"
#include "smdlib.h"
#include "file_descriptors.h"
#include "libasm.h"

/* WTF??? if not 1<<x at least use hexa :P */

#define BIT0 1
#define BIT1 2
#define BIT2 4
#define BIT3 8
#define BIT4 16
#define BIT5 32
#define BIT6 64
#define BIT7 128
#define BIT8 256
#define BIT9 512
#define BIT10 1024
#define BIT11 2048
#define BIT12 4096
#define BIT13 8192
#define BIT14 16384
#define BIT15 32768


#define IS_REMOVABLE(D) (D & BIT7) ? printf("Is removable\n") : printf("Is not removable\n")
#define IS_ATA_DRIVE(D) (D & BIT15) ? printf("Is not ATA\n") : printf("Is ATA\n")

#define DMA_SUP(D) (D & BIT8) ? printf("DMA is supported\n") : printf("DMA is not supported\n")
#define LBA_SUP(D) (D & BIT9) ? printf("LBA is supported\n") : printf("LBA is not supported\n")
#define DMA_QUEUED_SUP(D) (D & BIT1) ? printf("DMA QUEUED supported\n") : printf("DMA QUEUED is not supported\n")


#define UNUSED(x) ((void)(x))

typedef struct disk_cmd{
	int ata;
	int sector;
	int offset;
	int count; // Bytes count
	char * buffer;
}disk_cmd;

typedef struct disk_cmd * disk_cmd_t;

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
#define WIN_CAPACITY    0x25	/* command to ask for the capacity */
#define WIN_WRITE       0x30	/* command for the drive to write */
#define WIN_SPECIFY     0x91	/* command for the controller to accept params */

#define LBA_READ        WIN_READ
#define LBA_WRITE       WIN_WRITE

/* Parameters for the disk drive. */
#define SECTOR_SIZE      512	/* physical sector size in bytes */

#define _DISK_GET_NUM_BLOCKS 1

/* Error codes */
#define ERR		  -1	/* general error */

/* Miscellaneous. */
#define MAX_ERRORS         4	/* how often to try rd/wt before quitting */
#define NR_DEVICES        10	/* maximum number of drives */
#define MAX_WIN_RETRY  10000	/* max # times to try to output to WIN */
#define PART_TABLE     0x1C6	/* IBM partition table starts here in sect 0 */
#define DEV_PER_DRIVE      5	/* hd0 + hd1 + hd2 + hd3 + hd4 = 5 */

enum{
	READ_DISK = 0,
	WRITE_DISK,
	OK_DISK,
	ERROR_DISK
};

struct disk_data {
	fpos_t pos;
};

static void sendComm(int ata, int rdwr, unsigned short sector){

	// Wait for driver's ready signal.
	while (!(inw(ata + WIN_REG7) & BIT3));

	outb(ata + WIN_REG1, 0);
	outb(ata + WIN_REG2, 0);	// Set count register sector in 1

	outb(ata + WIN_REG3, (unsigned char)sector);			// LBA low
	outb(ata + WIN_REG4, (unsigned char)(sector >> 8));		// LBA mid
	outb(ata + WIN_REG5, (unsigned char)(sector >> 16));	// LBA high

	// Set LBA bit in 1 and the rest in 0
	outb(ata + WIN_REG6, 0xE0 | (ata << 4) | ((sector >> 24) & 0x0F));

	// Set command
	outb(ata + WIN_REG7, rdwr);
}

static void writeDataToRegister(int ata, uint8_t upper, uint8_t lower){

	uint16_t out;

	// Wait for driver's ready signal.
	while (!(inw(ata + WIN_REG7) & BIT3));

	out = (upper << 8) | lower;
	outw(ata + WIN_REG0, out);
}

static uint16_t getDataRegister(int ata){

	uint16_t ans;

	// Wait for driver's ready signal.
	while (!(inw(ata + WIN_REG7) & BIT3));

	ans = inw(ata + WIN_REG0);

	return ans;
}

static void translateBytes (char *ans, uint16_t databyte) {

	ans[0] = databyte & 0xFF;
	ans[1] = databyte >> 8;
}

static char read_tmp[512];

/**
 * This function is to be wrapped around by read() (only allows one sector to be
 * read)
 */
static void _read(int ata, char * ans, unsigned short sector, int offset, int count){

	// Just a small town sector... living in a lonely world
	if(count > 512 - offset)
		return;

	sendComm(ata, LBA_READ, sector);

	// Now read sector
	int b;
	uint16_t data;
	for ( b = 0; b < 512; b += 2 ) {
		data = getDataRegister(ata);
		translateBytes(read_tmp+b, data);
	}

	int i;
	for ( i=0; i<count; i++ ) {
		ans[i] = read_tmp[offset+i];
	}
}

static char write_tmp[512];

// Single sector write.
static void _write(int ata, char * msg, int bytes, unsigned short sector, int offset){

	int i = 0;

	if (offset || bytes < 512) {
		_read(ata, write_tmp, sector, 0, 512);
	}

	// Prepare sectors with new data
	for ( i = 0; i < bytes; i++ ) {
		write_tmp[ offset + i ] = msg[i];
	}

	// Send write command
	sendComm(ata, LBA_WRITE, sector);

	// Now write all the sector
	int b;
	for (b=0; b<512; b+=2) {
		writeDataToRegister(ata, write_tmp[b+1], write_tmp[b]);
	}
}

// Q: what are these here for?? (the next 3 functions)

static uint16_t getErrorRegister( void ){

	int ata = ATA0;
	unsigned short rta = inb(ata + WIN_REG1) & 0x00000FFFF;
	return rta;
}

static unsigned short getStatusRegister( void ){
	int ata = ATA0;
	unsigned short rta;
	rta = inb(ata + WIN_REG7) & 0x00000FFFF;
	return rta;
}

static void identifyDevice( void ){
	int ata  = ATA0;
	outb(ata + WIN_REG6, 0);
	outb(ata + WIN_REG7, WIN_IDENTIFY);
}

static size_t getDeviceCapacity( void ){

	// TODO: FIXME: STUB! Replace
	return 41803776;

/*	int ata  = ATA0;
	sendComm(ata, WIN_CAPACITY, 0);
	int ret = 0;
	ret += getDataRegister(ata);
	ret <<= 16;
	ret += getDataRegister(ata);

	return ret;*/

}


static ssize_t read ( size_t _fd, char* buf, size_t count ){

	struct file_descriptor* fd = get_file_descriptor(_fd);
	int ata = ATA0;
	int i;

	// Quantity of necessary sectors
	int sectors = ((count-1) / 512) + 1;
	int sector = ((struct disk_data*)(fd->data))->pos.dim1/ BLOCK_SIZE;
	int offset = ((struct disk_data*)(fd->data))->pos.dim1 % BLOCK_SIZE;

	for ( i=0; i<sectors; i++ ) {
		int size =  (i == sectors-1) ? (count%513) : 512;

		if (!i){
			_read(ata, buf, sector, offset, (offset+count>512)? size-offset : size);
		} else {
			_read(ata, buf+(i*512)-offset, sector+i, 0, size);
		}
	}
	((struct disk_data*)(fd->data))->pos.dim1 += count;

	return count;
}

static ssize_t write ( size_t _fd, char* buf, size_t bytes ){

	int i;
	int size;
	int ata = ATA0;
	struct file_descriptor* fd = get_file_descriptor(_fd);

	int sector = ((struct disk_data*)(fd->data))->pos.dim1/ BLOCK_SIZE;
	int offset = ((struct disk_data*)(fd->data))->pos.dim1 % BLOCK_SIZE;
	if (bytes == 0) return 0;

	// Quantity of necessary sectors
	int sectors = ((bytes  - 1)/ 512) + 1;
	int written = bytes;

	for ( i=0; i<sectors; i++ ) {

		if ( i == 0 ){
		    if (bytes > 512) {
				size = 512 - offset;
		    }
		    else {
				if (512 - offset <= (int)bytes) {
					size = 512 - offset;
				} else {
					size = bytes;
				}
		    }
		} else if ( i == sectors-1 ){
		    size = bytes;
		} else {
		    size = 512;
		}
		bytes -= size;

		if (!i) {
			_write(ata, buf, size, sector, offset);
		} else {
			_write(ata, buf+(i*512)-offset, size, sector+i, 0);
		}

	}
	((struct disk_data*)(fd->data))->pos.dim1 += written;

	return written;
}

static fpos_t  lseek ( size_t fd, fpos_t offset, int whence ){

	struct disk_data *data = get_file_descriptor( fd )->data;

	switch ( whence ) {

		case SEEK_SET:
			data->pos.dim1 = offset.dim1;
			break;
		case SEEK_CUR:
			data->pos.dim1 += offset.dim1;
			break;
		case SEEK_END:
			break; // NOT SUPPORTED
	}

	return data->pos;
}

static fpos_t  ltell ( size_t fd ){
	return ((struct disk_data*)get_file_descriptor(fd)->data)->pos;
}

static ssize_t fstat ( size_t fd, struct stat* buf ){

	UNUSED(fd);
	UNUSED(buf);

	// TODO

	return 0;
}

static ssize_t ioctl ( size_t fd, size_t request, ...){

	UNUSED(fd);
	UNUSED(request);

	return getDeviceCapacity();
}

void init_atadisk_fd( void ) {

	struct file_descriptor* fd = (struct file_descriptor*)
	                              malloc( sizeof( struct file_descriptor ) );
	fd->read  = read;
	fd->write = write;
	fd->lseek = lseek;
	fd->ltell = ltell;
	fd->fstat = fstat;
	fd->ioctl = ioctl;

	register_file_descriptor(__FD_ATA_DISK, fd);
}

