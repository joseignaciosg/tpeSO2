#include<stdio.h>
#include<stdlib.h>
#include<string.h>


/*
 *
 * Defines 
 * 
 * 
 */

#define OK_STATUS 1
#define WRONG_PATH 2
#define PARSING_PATH 1
#define END_PATH 3
#define MAX_LENGHT_NAME 15
#define MAX_COMMAND_LENGHT 200
#define BARRA 20
#define PUNTO 21
#define CARACTER 22
#define FREE 0
#define USED 1
#define NO_SPACE -1

/* BitMap Defines */

#define N 32
#define FREE 0

/* File System Defines */
#define NUMBER_DIRECT_BLOCKS 12
#define MAX_BLOCKS_NUMBER 500
//#define DISK_SIZE 262144 // Son BLOCK_SIZE bloques de BLOCK_SIZEK

/* Function parser Defines */

#define MKDIR 0
#define CD 1
#define LS 2
#define RMDIR 3

/* File Types */

#define UNKNOWN 0
#define REGULAR_FILE 1
#define DIRECTORY 2
#define CHARACTER_DEVICE 3
#define BLOCK_DEVICE 4 
#define NAMED_PIPE 5
#define SOCKET 6 //NO USAR
#define SYMBOLIC_LINK 7 //NO USAR

/* NEW DEFINES */

#define BLOCK_SIZE 512
#define	BITMAPSECTOR (SUPERBLOCKSECTOR+1)
#define	SUPERBLOCKSECTOR 1
#define INODEMAPSECTOR (BITMAPSECTOR + (BITMAP_SIZE/512) )
#define INODETABLESECTOR (INODEMAPSECTOR + (INODEMAP_SIZE/512) )
#define BITMAP_SIZE 2048
#define INODEMAP_SIZE	512
#define DISK_SIZE 1048576
#define INODETABLE_SIZE (INODEMAP_SIZE * 4)
#define BITMAP 1
#define INODEMAP 2
#define HEADER_BLOCKS ((BLOCK_SIZE + BLOCK_SIZE + BITMAP_SIZE + INODEMAP_SIZE + INODETABLE_SIZE + BLOCK_SIZE)/512)
/*
 *
 * Structures 
 *
 *
 */

typedef struct{
	int direct_blocks[NUMBER_DIRECT_BLOCKS];
	//Indirect blocks.
	//Double Indirectblocks.
}dataStream;

typedef struct{
	int type;
	int inode;
	int lenght;	
	char name[52];
}directoryEntry;

typedef struct{
	int identifier;
	int iNode_number;
	int uid;
	int gid;
	int mode;
	int size;
	dataStream data;
	char relleno[56];
}iNode;

typedef struct fsNode{
	struct fsNode * son;
	struct fsNode * father;
	iNode * inode;
	char * name;
	int last;
}fsNode;

typedef struct{
	char * name;
	int blockSize;
	int freeBlocks;
	int usedBlocks;
	fsNode * root;
}masterBlock;

typedef struct{
	int existFS;
}masterBootRecord;

typedef struct{
	int data[BITMAP_SIZE/512];
}BM;

typedef struct{
	int data[INODEMAP_SIZE/512];
}IM;

typedef struct{
	void * disk;
}virtualDisk;

typedef struct{
	int fd;
	char * path;
}fileDescriptor;

typedef int word_t;

enum { WORD_SIZE = sizeof(word_t) * 8 };

/* Functions */

inline int bindex(int b);
inline int boffset(int b);
void set_bit(int b, int mode);
void clear_bit(int b, int mode); 
int get_bit(int b, int mode);
void clear_all(int mode);
void set_all(int mode);
int search_free_blocks(int quantityBlocks);
void free_used_blocks(int init_block, int quantity, int mode);

void write_disk(void * disk, int sector, void * msg, int count, int offset);
void * read_disk(void * disk,int sector, void * msg, int count, int lenght);

void init_bitmap();
void init_inodemap();
void init_filesystem( char * filesystem_name, masterBootRecord * mbr);
void load_filesystem();

/*FS*/

iNode * fs_creat_inode(int identifier, int mode, int size);
void fs_init_inode( iNode * inode, int id, int md, int sz);
dataStream * fs_init_dataStream(int size,int id);
iNode * fs_get_inode(int number);
void fs_insert_inode(iNode * node);

/* Global Variables*/

//char * userName = "nloreti";
//fsNode * current;
//int session_uid;
//int session_gid;
//char * username;
virtualDisk * vDisk = NULL;
//word_t data[N / 32 + 1];
BM * bitmap;
IM * inodemap;
masterBlock * superblock;


/* Main */
void 
main(){

	/* Init Disco Virtual */
	vDisk = (virtualDisk*)malloc(sizeof(virtualDisk));
	vDisk->disk = (void *)malloc(sizeof(DISK_SIZE));

	/* Init Global Variables */	
	masterBootRecord * mbr = (masterBootRecord *)malloc(512);
	superblock = (masterBlock*)malloc(512);
	printf("BITMAP1:%d\n",BITMAP_SIZE);
	printf("BITMAP2:%d\n",sizeof(BITMAP_SIZE));		
	bitmap = (BM*)malloc(BITMAP_SIZE);	
	inodemap = (IM*)malloc(INODEMAP_SIZE);
	
	
	
	/* Create & Init File System */
	mbr = (masterBootRecord *)read_disk(vDisk->disk,0,mbr,BLOCK_SIZE,0);

	if ( mbr->existFS == 0 ){
		init_filesystem("Chinux", mbr);
	}else{
		load_filesystem();
	}

	printf("mbr:%d\n",mbr->existFS);
	superblock = read_disk(vDisk->disk,1,superblock,BLOCK_SIZE,0);
	printf("name:%s\nblock:%d\nfreeBlocks:%d\nusedBlocks:%d\n",superblock->name, superblock->blockSize, superblock->freeBlocks, superblock->usedBlocks);
	printf("InodeSize:%d\n",sizeof(iNode));
	printf("%d\n",512/8);
	printf("Directory:%d\n",sizeof(directoryEntry));//16 Directorios o archivos en bloques directos..

	iNode * nodo = fs_creat_inode(DIRECTORY,777,512);
	printf("mode:%d\n",nodo->mode);
	printf("size:%d\n",nodo->size);
	printf("iden:%d\n",nodo->identifier);
	fs_insert_inode(nodo);
}

void write_disk(void * disk, int sector, void * msg, int count, int offset){
	memcpy(disk+(sector*BLOCK_SIZE),msg,count);
	return;
}

void * read_disk(void * disk,int sector, void * msg, int count, int lenght){
	return memcpy(msg,disk+(sector*BLOCK_SIZE),count);
}

void init_filesystem( char * filesystem_name, masterBootRecord * mbr){
	
	/*mbr sector*/
	mbr->existFS = 1;
	write_disk(vDisk->disk, 0,mbr,BLOCK_SIZE,0);
	
	/* superBlock sector */
	superblock->name = "Chinux";
	superblock->blockSize = BLOCK_SIZE;
	superblock->freeBlocks = 10000;
	superblock->usedBlocks = 0;
	superblock->root = NULL; 
	write_disk(vDisk->disk,1,superblock,sizeof(masterBlock),0);

	/* bitmap Sectors */
	init_bitmap();
	
	/* inodemap Sectors */
 	init_inodemap();
}

void load_filesystem(){
	superblock = (masterBlock *)read_disk(vDisk->disk,SUPERBLOCKSECTOR,superblock,BLOCK_SIZE,0);
	bitmap = (BM *)read_disk(vDisk->disk,BITMAPSECTOR,bitmap,BITMAP_SIZE,0);
	inodemap =  (IM *)read_disk(vDisk->disk,INODEMAPSECTOR,inodemap,INODEMAP_SIZE,0);
}


void init_bitmap(){
	
	int i;
	clear_all(BITMAP);
	for(i=0;i<HEADER_BLOCKS;i++){
		set_bit(i,BITMAP);
	}
	write_disk(vDisk->disk,2,bitmap->data,BITMAP_SIZE,0);
}


void init_inodemap(){
	
	clear_all(INODEMAP);
	set_bit(0,INODEMAP);
	write_disk(vDisk->disk,6,bitmap->data,INODEMAP_SIZE,0);
}


/* 
 *
 * 	BITMAP Funcitons
 *
 */

inline int bindex(int b)
{ 
	return b / WORD_SIZE; 
}

inline int boffset(int b)
{ 
	return b % WORD_SIZE; 
}

void set_bit(int b, int mode)
{
	if ( mode == BITMAP)
	{
		bitmap->data[bindex(b)] |= 1 << (boffset(b)); 
	}
	else
	{
		inodemap->data[bindex(b)] |= 1 << (boffset(b)); 
	}
}

void clear_bit(int b, int mode) 
{ 
	if ( mode == BITMAP )
	{
		bitmap->data[bindex(b)] &= ~(1 << (boffset(b)));
	}
	else
	{
		inodemap->data[bindex(b)] &= ~(1 << (boffset(b)));	
	}
}

int get_bit(int b, int mode) 
{ 
	if( mode == BITMAP )
	{	
		return bitmap->data[bindex(b)] & (1 << (boffset(b)));
	}
	else
	{
		return inodemap->data[bindex(b)] & (1 << (boffset(b)));
	}
}

void clear_all(int mode)
{

	int i,limit;
	if ( mode == BITMAP ){
		limit = BITMAP_SIZE;
	}
	else
	{
		limit =	INODEMAP_SIZE;
	}
	for ( i = 0; i < limit; i++)
	{
		clear_bit(i,mode);
	}
	
	return;
}

void set_all(int mode)
{

	int i,limit;
	if ( mode == BITMAP ){
		limit = BITMAP_SIZE;
	}
	else
	{
		limit =	INODEMAP_SIZE;
	}
	for ( i = 0; i < limit; i++)
	{
		set_bit(i,mode);
	}
	return;
}

int search_free_blocks(int quantityBlocks)
{
	//TODO: Recorrer el bitmap hasta encontrar quantity of Blocks libres y retornar el numero de bloques 
	int i;
	int count = 0;
	int candidate = -1;
	if ( superblock->freeBlocks == 0){
		return NO_SPACE;	
	}

	for( i = 0; i < (BITMAP_SIZE) && count < quantityBlocks; i++){
		
		if ( candidate == -1 && get_bit(i,BITMAP) == FREE ){
			candidate = i;		
		}
		if ( get_bit(i,BITMAP) == FREE ){
			count++;	
		}else{
			count = 0;
			candidate = -1;		
		}
	}
	if ( candidate == -1 ){
		return -1;
	}
	//Si el candidato es bueno entonces procedo a rellenar con 1 los bloques que voy a usar.
	for ( i = candidate; i < quantityBlocks; i++){
		set_bit(i,BITMAP);
	}

	superblock->freeBlocks = superblock->freeBlocks - quantityBlocks;
	superblock->usedBlocks = superblock->usedBlocks + quantityBlocks;

	return candidate;

}

void free_used_blocks(int init_bit, int quantity, int mode){
	
	int i;
	for( i = init_bit; i < (init_bit + quantity); i++){
		clear_bit(i,mode);
	}
	return;
}


/* 
 *
 * 	iNode Funcitons
 *
 */


iNode * fs_creat_inode(int identifier, int mode, int size){
	
	iNode * ret = (iNode *)malloc(sizeof(iNode));
	fs_init_inode(ret,identifier,mode,size);
	return ret;
}

void fs_init_inode( iNode * inode, int id, int md, int sz){
	//TODO: Funcion para inicializar inodos FALTA: iNode_number y las fechas de creacion y modificacion;
	inode->identifier = id;
	//inode->uid = session_id;
	//inode->gid = session_gid;
	inode->mode = md;
	inode->size = sz;
	//inode->data = *fs_init_dataStream(sz,id);
	return;
}

dataStream * fs_init_dataStream(int size, int id){
	
	dataStream * ret = (dataStream *)malloc(sizeof(dataStream));
	int quantityBlocks,freeblock,i;	
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),16);
 
	if ( id == DIRECTORY ){
		quantityBlocks = 24;
		freeblock = search_free_blocks(quantityBlocks);
		for (i=0;i<12;i++){
			ret->direct_blocks[i] = freeblock + (i*2);
			//ARMAR LOS 12 DIRECTORIE ENTRY Y PEGARLOS EN DISCO			
			write_disk(vDisk->disk,ret->direct_blocks[i],dr,BLOCK_SIZE*2,0);
			
		}
	}else{
		quantityBlocks = (int)(size/BLOCK_SIZE) + 1;
		freeblock = search_free_blocks(quantityBlocks);
		if ( freeblock != -1){
			ret->direct_blocks[0] = freeblock;
			ret->direct_blocks[1] = quantityBlocks;
		}else{
			printf("FAIL: NO ENCONTRE MEMORIA LIBRE\n");
			//TODO: Implementar una seccion donde se maneje si no encuentra archiva.		
		}
	}

	return ret;
}

iNode * fs_get_inode(int number){

	int sector = number/4;
	int offset = number%4;
	iNode * ret = (iNode*)malloc(sizeof(iNode));
	void * recieve = (void *)malloc(sizeof(BLOCK_SIZE));
	
	if ( get_bit(number, INODEMAP) != 0 ){
		recieve = read_disk(vDisk->disk,INODETABLESECTOR + sector,recieve,BLOCK_SIZE,0);
		ret = (recieve + 128*offset);
	}else{
		return NULL;
	}

	return ret;
}

void fs_insert_inode(iNode * node){
	
	int number = node->iNode_number;
	int sector = number/4;
	int offset = number%4;
	void * recieve;
	if ( get_bit(number, INODEMAP) != 0 ){
		set_bit(number,INODEMAP);	
	}
	recieve = read_disk(vDisk->disk,INODETABLESECTOR+sector,recieve, BLOCK_SIZE,0);
	memcpy(recieve+(128*offset),node,128);
	write_disk(vDisk->disk,INODETABLESECTOR+sector,recieve,BLOCK_SIZE,0);
	
	return;
}


