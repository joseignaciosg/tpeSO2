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
	iNode * root;
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

iNode * search_directory( char * name, iNode * node);
void print_directories(iNode * current);


iNode * parser_path(char * path, iNode * posible_inode);
void makeDir(char *);
void removeDir(char *);
void cd(char *);
void ls(char *);

/*FS*/

iNode * fs_creat_inode(int identifier, int mode, int size, iNode * current);
void fs_init_inode( iNode * inode, int id, int md, int sz, iNode * current);
dataStream * fs_init_dataStream(int size,int id, int number, iNode * current);
iNode * fs_get_inode(int number);
int fs_insert_inode(iNode * node);
void init_root();
void insert_directory( char * name, iNode * current );
void insert_directory_entry(iNode * newDirectory, iNode * current, char * name);

void substr(char dest[], char src[], int offset, int len);
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
iNode * current;


/* Main */
void 
main(){

	/* Init Disco Virtual */
	vDisk = (virtualDisk*)malloc(sizeof(virtualDisk));
	vDisk->disk = (void *)malloc(DISK_SIZE);

	/* Init Global Variables */	
	masterBootRecord * mbr = (masterBootRecord *)malloc(512);
	superblock = (masterBlock*)malloc(512);		
	bitmap = (BM*)calloc(BITMAP_SIZE,1);	
	inodemap = (IM*)calloc(INODEMAP_SIZE,1);
	
	
	
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
	printf("Directory:%d\n",sizeof(directoryEntry));//16 Directorios o archivos en bloques directos..

	iNode * nodo = fs_creat_inode(DIRECTORY,777,512,superblock->root);
	fs_insert_inode(nodo);
	iNode * nodo3 = fs_creat_inode(DIRECTORY,737,512,superblock->root);
	fs_insert_inode(nodo3);

	//nodo->iNode_number = 20;
	printf("\n\nTABLA DATOS\n");
	printf("inode-number:%d\n",nodo->iNode_number);		
	printf("mode:%d\n",nodo->mode);
	printf("size:%d\n",nodo->size);
	printf("iden:%d\n",nodo->identifier);

	
	iNode * nodo2 = fs_get_inode(nodo->iNode_number);
	printf("inode-number:%d\n",nodo->iNode_number);		
	printf("mode:%d\n",nodo2->mode);
	printf("size:%d\n",nodo2->size);
	printf("iden:%d\n",nodo2->identifier);

	insert_directory("Hola",superblock->root);
	//ls("asd");
	makeDir("comostas");
	print_directories(superblock->root);
	makeDir("Hola/Comocomo");
	cd("Hola");
	print_directories(current);

	
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
	superblock->freeBlocks = 10000;//TODO:PONER LA CANTIDAD POSTA.
	superblock->usedBlocks = 0;
	superblock->root = NULL; 
	write_disk(vDisk->disk,1,superblock,sizeof(masterBlock),0);

	/* bitmap Sectors */
	init_bitmap();
	
	/* inodemap Sectors */
 	init_inodemap();

	/* Root node & current node */
	
	init_root();
	write_disk(vDisk->disk,1,superblock,512,0);
	current = superblock->root;

	return;
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
	return;
}


void init_inodemap(){
	
	clear_all(INODEMAP);
	set_bit(0,INODEMAP);
	write_disk(vDisk->disk,6,bitmap->data,INODEMAP_SIZE,0);
}

void init_root(){
	
	set_bit(HEADER_BLOCKS,BITMAP);
	set_bit(0,INODEMAP);
	superblock->root = fs_creat_inode(DIRECTORY,777,512,NULL);
	fs_insert_inode(superblock->root);
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

int search_free_inode(){
	int i, ret;
	for( i = 0; i < INODEMAP_SIZE; i++){
		if ( (ret = get_bit(i,INODEMAP)) == FREE ){
			set_bit(i,INODEMAP);
			return i;		
		}
	}
	return -1;
}

int search_free_blocks(int quantityBlocks)
{
	
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
	printf("CANDIDATO:%d\n",candidate);
	if ( candidate == -1 ){
		return -1;
	}
	//Si el candidato es bueno entonces procedo a rellenar con 1 los bloques que voy a usar.
	for ( i = candidate; i < (candidate + quantityBlocks); i++){
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


iNode * fs_creat_inode(int identifier, int mode, int size, iNode * current){
	
	iNode * ret = (iNode *)malloc(sizeof(iNode));
	fs_init_inode(ret,identifier,mode,size,current);
	return ret;
}

void fs_init_inode( iNode * inode, int id, int md, int sz, iNode * current){
	//TODO: Funcion para inicializar inodos FALTA: las fechas de creacion y modificacion;
	inode->identifier = id;
	inode->iNode_number = search_free_inode();
	//inode->uid = session_id;
	//inode->gid = session_gid;
	inode->mode = md;
	inode->size = sz;
	
	inode->data = *fs_init_dataStream(sz,id,inode->iNode_number,current);
	return;
}

/*typedef struct{
	int direct_blocks[NUMBER_DIRECT_BLOCKS];
	//Indirect blocks.
	//Double Indirectblocks.
}dataStream;

typedef struct{
	int type;
	int inode;
	int lenght;	
	char name[52];
}directoryEntry;*/

dataStream * fs_init_dataStream(int size, int id, int number, iNode * current){
	
	dataStream * ret = (dataStream *)malloc(sizeof(dataStream));
	int quantityBlocks,freeblock,i;	
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
 	directoryEntry dr1 = {DIRECTORY, number, 0, "."};
	directoryEntry dr2 = {DIRECTORY, number,0,".."};
	if ( current != NULL ){
		dr2.inode = current->iNode_number;
	}
	
	dr[0] = dr1;
	dr[1] = dr2;

	if ( id == DIRECTORY ){
		quantityBlocks = 12;
		freeblock = search_free_blocks(quantityBlocks);
		for (i=0;i<12;i++){
			ret->direct_blocks[i] = freeblock + (i);//Para aumentar esto hay uqe multiplicarlo			
		}
		write_disk(vDisk->disk,ret->direct_blocks[0],dr,BLOCK_SIZE*12,0);
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
	void * recieve = malloc(512);
	
	if ( get_bit(number, INODEMAP) != 0 ){
		recieve = read_disk(vDisk->disk,INODETABLESECTOR + sector,recieve,BLOCK_SIZE,0);
		memcpy(ret,recieve + (128*offset),128);
	}else{
		return NULL;
	}

	return ret;
}



int fs_insert_inode(iNode * node){
	
	int number = node->iNode_number;
	int sector = number/4;
	int offset = number%4;
	iNode * node2 = malloc(sizeof(iNode));
	void * recieve = malloc(BLOCK_SIZE);
	void * recieve2 = malloc(BLOCK_SIZE);
	
	if ( get_bit(number, INODEMAP) == 0 ){
		set_bit(number,INODEMAP);	
	}

	recieve = read_disk(vDisk->disk,INODETABLESECTOR+sector,recieve, BLOCK_SIZE,0);
	memcpy(recieve+(128*offset),node,128);//+(128*offset)
	write_disk(vDisk->disk,INODETABLESECTOR+sector,recieve,BLOCK_SIZE,0);
	
	//recieve2 = read_disk(vDisk->disk,INODETABLESECTOR+sector,recieve2, BLOCK_SIZE,0);
	//memcpy(node2,recieve2,128);

	return number;
}


//TODO:Poder buscar un archivo o directorio por nombre a partir de un iNode y retornar el inodo que corresponda.(CD)

iNode * search_directory(char * name, iNode * actual_node){
	int init_block = actual_node->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(vDisk->disk,init_block,dr,BLOCK_SIZE*12,0);
	
	int i;
	for(i=0;i<96;i++){
		if( strcmp(name,dr[i].name) == 0){
			return fs_get_inode(dr[i].inode);
		}
	}
	return NULL;
}

void print_directories(iNode * current){
	
	int init_block = current->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(vDisk->disk,init_block,dr,BLOCK_SIZE*12,0);
	
	int i;
	for(i=0;i<96;i++){
		if( dr[i].type != 0){
		printf("%s ", dr[i].name);
		}
	}
	putchar(10);
	return;
}
//TODO:Otro que te liste todos los directorios y archivos.

void insert_directory( char * name, iNode * current ){
	//TODO: CRear el inodo de directorio con todas sus entradas.
	iNode * newDirectory = (iNode *)malloc(sizeof(iNode));
	newDirectory =  fs_creat_inode(DIRECTORY,777,512,current);
	fs_insert_inode(newDirectory);

	insert_directory_entry(newDirectory,current,name);
	//TODO: Actualizar el inodo actual para que tenga la informacion del nuevo.
	//TODO: No chequea que no alla repetidos.
}

void insert_directory_entry(iNode * newDirectory, iNode * current, char * name){
	int init_block = current->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(vDisk->disk,init_block,dr,BLOCK_SIZE*12,0);
	int i;
	for ( i = 0; i < 96; i++){
		if ( dr[i].type == 0 ){
			dr[i].type = DIRECTORY;
			dr[i].inode = newDirectory->iNode_number;
			dr[i].lenght = 0;
			memcpy(dr[i].name,name,strlen(name));
			break;
		}
	}
	write_disk(vDisk->disk,init_block,dr,BLOCK_SIZE*12,0);

	return;
	
}



/*
 *
 *	Manage tree Functions ( CD , MKDIR, RMDIR and LS )
 *
 */

iNode * parser_path(char * path, iNode * posible_inode){
	
	int path_status = OK_STATUS;
	int path_parsing = PARSING_PATH;
	int i=0;
	int j=0;
	int last = 0;
	int status;
	iNode * old_current = posible_inode;
	iNode * temp_inode;
	char buffer[MAX_COMMAND_LENGHT];
	int index = 0;

	/* Veo que tengo al empezar */

	if ( path[i] == '/' ){
		posible_inode = superblock->root;
		status = BARRA;
	}
	else if ( path[i] == '.' )
	{
		status = PUNTO;
	}
	else
	{
		status = CARACTER;
		buffer[j++] = path [i];
	}
	i++;
	
	/* Voy Switecheando el status y armando los nombres hasta que termine de parsear u obtenga un path incorrecto */
	while( path_parsing != WRONG_PATH && path_parsing != END_PATH ){
		
		if ( path[i] == '\0' )
		{
			path_parsing = END_PATH;
		}
		if( status == BARRA )
		{
			if ( path[i] == '/' )
			{
				path_status = WRONG_PATH;			
			}else if ( path[i] == '.' )
			{
				status = PUNTO;
			}else
			{
				status = CARACTER;
				buffer[j++] = path[i]; 			
			}
		}
		else if (status == PUNTO )
		{
			if( path[i] == '\0'){
				path_status = OK_STATUS;
				path_parsing = END_PATH;			
			}
			else if ( path[i] == '/')
			{
				status = BARRA;
			}
			else if ( path[i] == '.' )
			{
				if ( path[i+1] != '/' && path[i+1] != '\0')
				{
					if ( i > 2 && path[i-2] != '/')
						path_status = WRONG_PATH;				
				}
				else
				{
					posible_inode = search_directory("..",posible_inode);
					i++;
					
				}
				status = PUNTO;

			}else
			{
				status = CARACTER;
				buffer[j++] = '.';
				buffer[j++] = path[i];
			}
		}
		else if ( status == CARACTER )
		{
			if ( path[i] == '\0')
			{
				path_parsing = END_PATH;
				path_status = OK_STATUS;
				buffer[j] = '\0';
			
				if( ( temp_inode = search_directory(buffer, posible_inode) ) != NULL){
					posible_inode = temp_inode;
					j=0;								
				}else
				{
					path_status = WRONG_PATH;
				}			
			}
			if ( path[i] == '/')
			{
				status = BARRA;
				buffer[j] = '\0';
				if( ( temp_inode = search_directory(buffer, posible_inode) ) != NULL){
					posible_inode = temp_inode;
					j=0;								
				}else
				{
					path_status = WRONG_PATH;
				}
			}
			if ( path[i] == '.' )
			{
				status = PUNTO;
				buffer[j++] = path[i];
			}else
			{
				status = CARACTER;
				buffer[j++] = path[i];
			}
	
			i++;
		}
	}

	if ( path_status == WRONG_PATH ){
		return NULL;	
	}
	return posible_inode;
}
void cd(char * path){
	
	iNode * posible_inode = current;
	posible_inode = parser_path(path, posible_inode);

	if ( posible_inode == NULL )
	{
		printf("Wrong name or path\n");
	}else
	{
		current = posible_inode;
	}
	return;

}

void makeDir(char * newName){

	char * parcialName = (char*)malloc(sizeof(25));
	iNode * makeDir_current = current;
	iNode * posibleDir_current;
	int i,j,index;

	for( i = 0; newName[i] != '\0'; i++) //Parseo el string hasta el final
	{ 	
		j = i;		
		if ( newName[j] == '/' )
			{
				i++;
				j++;				
			}
		
		for( ; !(newName[j] == '/' || newName[j] == '\0'); j++); //Recorro paralelamente hasta encontrar o una / o un '/0'
			
			substr(parcialName, newName, i,j);
			if( ( posibleDir_current = search_directory(parcialName, makeDir_current) ) == NULL){
				insert_directory(parcialName,makeDir_current);
				makeDir_current = search_directory(parcialName,makeDir_current);
			}else{
				makeDir_current =posibleDir_current;
			}
			i=j-1;
	
	}

	return;
}


//DONE!
void ls(char * path){
	
	int i;
	iNode * posible_inode = current;
	posible_inode = parser_path(path, posible_inode);

	if ( posible_inode == NULL )
	{
		printf("Wrong name or path\n");
	} 
	else 
	{
		print_directories(posible_inode);
	}
	return;
	
}

void rmdir( char * path){

	return; 

/*	int i;
	fsNode * posible_fsnode = current;
	int path_status = parser_path(path, posible_fsnode);

	if ( path_status == WRONG_PATH )
	{
		printf("Wrong name or path\n");
	} 
	else if ( path_status == OK_STATUS )
	{
		//TODO:REMOVER EL DIRECTORIO!
	}
	return;*/
}


void substr(char dest[], char src[], int offset, int len)
{
	int i;
	for(i = 0; i < len && src[offset + i] != '\0'; i++)
	dest[i] = src[i + offset];
	dest[i] = '\0';
}

