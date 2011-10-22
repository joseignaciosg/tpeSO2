#include "../include/atadisk.h"

#define BIT0 1
#define BIT1 1<<1
#define BIT2 1<<2
#define BIT3 1<<3
#define BIT4 1<<4
#define BIT5 1<<5
#define BIT6 1<<6
#define BIT7 1<<7
#define BIT8 1<<8
#define BIT9 1<<9
#define BIT10 1<<10
#define BIT11 1<<11
#define BIT12 1<<12
#define BIT13 1<<13
#define BIT14 1<<14
#define BIT15 1<<15

/* This defines are to check the sepecifications of the disk.
#TODO: check size!*/
#define IS_REMOVABLE(D) (D & BIT7) ? printf("Is removable\n") : printf("Is not removable\n")
#define IS_ATA_DRIVE(D) (D & BIT15) ? printf("Is not ATA\n") : printf("Is ATA\n")

#define DMA_SUP(D) (D & BIT8) ? printf("DMA is supported\n") : printf("DMA is not supported\n")
#define LBA_SUP(D) (D & BIT9) ? printf("LBA is supported\n") : printf("LBA is not supported\n")
#define DMA_QUEUED_SUP(D) (D & BIT1) ? printf("DMA QUEUED supported\n") : printf("DMA QUEUED is not supported\n")


// To read N bytes from hard disk, must alloc N+1 bytes for ans, as N+1 byte is used to null-character
void _disk_read(int ata, char * ans, int count, unsigned short sector, int offset){

// Just a sector...
if(count > 512 - offset)
return;

char tmp[512];
int i = 0;
for(i = 0; i < 512; ++i)
{
tmp[i] = 0;
}

sendComm(ata, LBA_READ, sector);

_Halt(); // TODO: Make the 500ns wait

int b;
unsigned short data;
int errors = 0;
for(b=0;b<512 && errors < 10;b+=2){
data = getDataRegister(ata);
if(data == 65535) {
errors++;
b-=2;
}
translateBytes(tmp+b, data);
}


for(i=0;i<count;i++) {
ans[i] = tmp[offset+i];
}
}

// Translate one word into two char
void translateBytes(char * ans, unsigned short databyte){
ans[0] = databyte & 0xFF;
ans[1] = databyte >> 8;
}

// Writes to the ata chosen the msg received the ammount of bytes requested starting from the secto chose + the offset
void _disk_write(int ata, char * msg, int bytes, unsigned short sector, int offset){

ata=ATA0;
int i = 0;

char write_tmp[512];

for(i = 0; i < 512; ++i)
{
write_tmp[i] = 0;
}
i = 0;


// Prepare sectors with new data
for ( i = 0; i < bytes; i++ ) {
write_tmp[ offset + i ] = msg[i];
}



// Send write command
sendComm(ata, LBA_WRITE, sector);

_Halt(); // TODO: Make the 500ns wait

// Now write all the sector
int b;
for (b=0; b<512; b+=2) {
writeDataToRegister(ata, write_tmp[b+1], write_tmp[b]);
}
}

void writeDataToRegister(int ata, char upper, char lower){
_Cli();
unsigned short out;

// Wait for driver's ready signal.
while (!(_inw(ata + WIN_REG7) & BIT3));

out = (upper << 8) | lower;
_outw(ata + WIN_REG0, out);

_Sti();
}

unsigned short getDataRegister(int ata){
_Cli();
unsigned short ans;

// Wait for driver's ready signal.
while (!(_inw(ata + WIN_REG7) & BIT3));

ans = _inw(ata + WIN_REG0);

_Sti();
return ans;
}

unsigned short getErrorRegister(int ata){
_Cli();
unsigned short rta = _in(ata + WIN_REG1) & 0x00000FFFF;
_Sti();
return rta;
}

// Send a command to the disk in order to read or write
void sendComm(int ata, int rdwr, unsigned short sector){
_Cli();

_out(ata + WIN_REG1, 0);
_out(ata + WIN_REG2, 1); // Set count register sector in 1

_out(ata + WIN_REG3, (unsigned char)sector); // LBA low
_out(ata + WIN_REG4, (unsigned char)(sector >> 8)); // LBA mid
_out(ata + WIN_REG5, (unsigned char)(sector >> 16)); // LBA high
_out(ata + WIN_REG6, 0);

// Set command
_out(ata + WIN_REG7, rdwr);


_Sti();
}

unsigned short getStatusRegister(int ata){
unsigned short rta;
_Cli();
rta = _in(ata + WIN_REG7) & 0x00000FFFF;
_Sti();
return rta;
}

void identifyDevice(int ata){
_Cli();
_out(ata + WIN_REG6, 0);
_out(ata + WIN_REG7, WIN_IDENTIFY);
_Sti();
}

// Check disk features
void check_drive(int ata){
printf("-----------------------\n");
printf("Identifying device ");
switch(ata){
case ATA0: printf("ATA0...");break;
case ATA1: printf("ATA1...");break;
}
printf("\n");

    identifyDevice(ata);


unsigned short data = 0;

char msg[512];
unsigned short sector=12;
int offset=0;
int count=512;

    int i;
    for(i=0;i<255;i++){
        data = getDataRegister(ata);
switch(i){
case 0:
//printf("Data returned (%d): %d\n", i,data);
IS_REMOVABLE(data);
IS_ATA_DRIVE(data);
break;
case 49:
DMA_SUP(data);
LBA_SUP(data);
break;
case 83:
DMA_QUEUED_SUP(data);
break;
}
    }
}



