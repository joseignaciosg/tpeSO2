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
#define NUMBER_DIRECT_BLOCKS 500
#define MAX_BLOCKS_NUMBER 500
#define BLOCK_SIZE 1024
#define DISK_SIZE 262144 // Son 512 bloques de 512K

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
	int day;
	int month;
	int year;
	int hour;
	int minutes;
}date;

typedef struct{
	int identifier;
	int iNode_Number;
	int uid;
	int gid;
	int mode;
	int size;
	date creationTime;
	date modifiedTime;
	dataStream data;
}iNode;

typedef struct fsNode{
	struct fsNode * son;
	struct fsNode * father;
	iNode * inode;
	char * name;
	int last;
}fsNode;

typedef struct{
	char name[MAX_NAME_LENGHT];
	int blockSize;
	int freeBlocks;
	int usedBlocks;
	fsNode * root;
}masterBlock;

typedef struct{
	int number; //check number
	int size = BLOCK_SIZE;
}normalBlock;

typedef struct{
	void * disk;
}virtualDisk;

typedef struct{
	int uid;
	int gid;
	char * name;
	int logged; //Para saber si esta logueado.
}userInfo;

typedef struct{
	int fd;
	char * path;
}fileDescriptor;

typedef int word_t;

enum { WORD_SIZE = sizeof(word_t) * 8 };

/*Functions Declarations */

void makeDir(char *);
void removeDir(char *);
void cd(char *);
void ls(char *);
void substr(char dest[], char src[], int offset, int len);
void set_bit(int b);
void clear_bit(int b);
void clear_all();
void set_all();
void free_used_blocks(int init_block, int quantity);
inline int bindex(int b);
inline int boffset(int b);
int get_bit(int b);
int search_free_blocks(int quantityBlocks);
fsNode * newFileSystem();
fsNode * newFsNode( fsNode * newFather, char * newName);

/* VFS Functions Declartions */

int creat (const char *filename, mode_t mode);
int open (const char *filename, int flags[, mode_t mode]);
int read(int fd, char *buf, int n);
int write(int fd, char *buf, int n);
int close(int fd);

/* Global Variables*/

char * userName = "nloreti";
fsNode * current;
int session_uid;
int session_gid;
char * username;
virtualDisk * vDisk = NULL;
word_t data[N / 32 + 1];


/* Main */
void 
main(){

	//TODO: Tratar de levantar de Disco el File System
	if ( vDisk == NULL ) { //Esto simula lo anterior
		vDisk = newFileSystem();
	}
	current = vDisk->root;

	//TODO: Ejectura consola y pedir comandos

	makeDir("Direc1");
	makeDir("Directorio2");
	makeDir("D3");
	makeDir("D3/nuevo");
	ls("");
	cd("D3");
	makeDir("D313");
	makeDir("D52");
	ls("");
	cd("..");
	ls("");
}



/*
 *
 *	Functions Code
 *
 */

/*
 *
 *	Manage tree Functions ( CD , MKDIR, RMDIR and LS )
 *
 */

int parser_path(char * path, fsNode * posible_fsnode){
	
	int path_status = OK_STATUS;
	int path_parsing = PARSING_PATH;
	int i=0;
	int j=0;
	int last = 0;
	int status;
	posible_fsnode = current;
	fsNode * old_current = current;
	char buffer[MAX_COMMAND_LENGHT];
	int index = 0;	

	/* Veo que tengo al empezar */

	if ( path[i] == '/' ){
		posible_fsnode = root;
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
		
		/*if ( path[i] == '\0' )
		{
			path_parsing = END_PATH;
		}*/
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
					if ( posible_fsnode->father != NULL){
						posible_fsnode = posible_fsnode->father;
						i++;
					}
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
				if ( (index = search_inode_list(posible_fsnode, buffer)) != -1 )
				{
					posible_fsnode = &posible_fsnode->son[index];
					j = 0;				
				}else
				{
					path_status = WRONG_PATH;
				}			
			}
			if ( path[i] == '/')
			{
				status = BARRA;
				buffer[j] = '\0';
				if ( (index = search_inode_list(posible_fsnode, buffer)) != -1 )
				{
					posible_fsnode = &posible_fsnode->son[index];
					j = 0;				
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

	return path_status;
}
void cd(char * path){
	
	fsNode * posible_fsnode = current;
	int path_status = parser_path(path, posible_fsnode);

	if ( path_status == WRONG_PATH )
	{
		current = old_current;
		printf("Wrong name or path\n");
	} 
	else if ( path_status == OK_STATUS )
	{
		current = posible_fsnode;
	}
	
	return;

}

void makeDir(char * newName){

	char * parcialName = (char*)malloc(sizeof(15));
	fsNode * makeDir_current = current;
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
			if( (index = search_inode_list(makeDir_current, parcialName)) == -1){		
				index = makeDir_current->last;
				makeDir_current->son[makeDir_current->last] = *newFsNode(makeDir_current, parcialName);
				makeDir_current->last++;
				makeDir_current = &((makeDir_current->son)[index]);
			}else{
				makeDir_current = &((makeDir_current->son)[index]);
			}
			i=j-1;
	
	}

	return;
}


void ls(char * path){
	
	int i;
	fsNode * posible_fsnode = current;
	int path_status = parser_path(path, posible_fsnode);

	if ( path_status == WRONG_PATH )
	{
		printf("Wrong name or path\n");
	} 
	else if ( path_status == OK_STATUS )
	{
		for (i = 0; i < current->last; i++)
		{
		printf("%s\t", current->son[i].name);	
		}
		if ( i != 0 )
		{
			putchar(10);
		}
	}
	return;
	
}

void rmdir( char * path){

	int i;
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
	return;
}
/*
 *
 *	Node & FileSystem Functions
 *
 */

virtualDisk * newFileSystem( char * fsName ){

	virtualDisk * ret = (virtualDisk *)malloc(sizeof(virtualDisk));
	ret->disk = (void *) malloc (sizeof(SIZE_DISK));	
	init_superBlock(ret->disk);
	init_bitmap(ret->disk);
	init_inodetable(ret->disk);
	return ret;
	
}

fsNode * newFSRoot(){
	fsNode * ret = (fsNode*)malloc(sizeof(fsNode));
	ret->son = (fsNode *)malloc(sizeof(fsNode)*100);
	ret->father = NULL;
	ret->last = 0;
	ret->name = "root";
	return ret;
}

fsNode * newFsNode( fsNode * newFather, char * newName){

	fsNode * ret = (fsNode *)malloc(sizeof(fsNode));
	ret->son = (fsNode *)malloc(sizeof(fsNode)*100);
	ret->father = newFather;
	ret->name = newName;
	ret->last = 0;
	return ret;
}

/* 
 *	Initialization codes for FileSystem
 */


void init_superBlock(void * disk){
	masterBlock ret = (masterBlock)malloc(sizeof(masterBlock));	
	ret->name = fsName;
	ret->blocksize = BLOCKSIZE;
	ret->freeBlocks = DISKSIZE/BLOCKSIZE;
	ret->usedBlocks = 0;
	ret->root = newFSRoot();
	*disk = ret;//TODO: Asociar el superBlock al segundo bloque de esta memoria.
}

void init_bitmap(ret->disk){
	word_t bitMap = (word_t)malloc(sizeof(BLOCKSIZE/32+1);
	clear_all(bitMap);
	//TODO: Copy this structure to the correct sector in disk
}

void init_inodetable(){
	//TODO: Inicializar table y ponerla en disco.
}


/*
 *
 * 	Auxilary Functions
 *
 */

int search_inode_list( fsNode * fsnode, char * name){
	int i;
	for( i = 0; i < fsnode->last; i++ ){
		if ( strcmp(fsnode->son[i].name, name) == 0 )
			return i;
	}

	return -1;
}

void substr(char dest[], char src[], int offset, int len)
{
	int i;
	for(i = 0; i < len && src[offset + i] != '\0'; i++)
	dest[i] = src[i + offset];
	dest[i] = '\0';
}

/*int strcmp(const char *s1, const char *s2)
 {
     unsigned char uc1, uc2;

     while (*s1 != '\0' && *s1 == *s2) {
         s1++;
         s2++;
     }

     uc1 = (*(unsigned char *) s1);
     uc2 = (*(unsigned char *) s2);
     return ((uc1 < uc2) ? -1 : (uc1 > uc2));
 }*/


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

void set_bit(int b)
{
	data[bindex(b)] |= 1 << (boffset(b)); 
}

void clear_bit(int b) 
{ 
	data[bindex(b)] &= ~(1 << (boffset(b)));
}

int get_bit(int b) 
{ 
	return data[bindex(b)] & (1 << (boffset(b)));
}

void clear_all()
{

	int i;
	for ( i = 0; i < N; i++)
	{
		clear_bit(i);
	}
	return;
}

void set_all()
{

	int i;
	for ( i = 0; i < N; i++)
	{
		set_bit(i);
	}
	return;
}

int search_free_blocks(int quantityBlocks)
{
	//TODO: Recorrer el bitmap hasta encontrar quantity of Blocks libres y retornar el numero de bloques 
	int i;
	int count = 0;
	int candidate = -1;
	/*if ( vDisk->freeBlocks == 0){
		return NO_SPACE;	
	}*/
	for( i = 0; i < (N) && count < quantityBlocks; i++){
		
		if ( candidate == -1 && get_bit(i) == FREE ){
			candidate = i;		
		}
		if ( get_bit(i) == FREE ){
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
		set_bit(i);
	}

	/*vDisk->freeBlocks = vDisk->freeBlocks - quantityBlocks;
	vDisk->usedBlocks = vDisk->usedBlocks + quantityBlocks*/

	return candidate;
}

void free_used_blocks(int init_block, int quantity){
	
	int i;
	for( i = init_block; i < (init_block + quantity); i++){
		clear_bit(i);
	}
	return;
}

/*
 *
 * 	INODE Functions
 * 	
 *
 */

iNode * fs_creat_inode(int identifier, int mode, int size){
	//TODO:Funcion para crear inodos
	iNode * ret = (iNode *)malloc(sizeof(iNode));
	fs_init_inode(ret,identifier,mode,size);
	return ret;
}

void fs_init_inode( iNode * inode, int id, int md, int sz){
	//TODO: Funcion para inicializar inodos FALTA: iNode_number y las fechas de creacion y modificacion;
	inode->identifier = id;
	inode->uid = session_id;
	inode->gid = session_gid;
	inode->mode = md;
	inode->size = sz;
	inode->data = fs_init_dataStream(sz);
	return;
}

dataStream fs_init_dataStream(int size){
	dataStream ret = (dataStream)malloc(sizeof(dataStream);
	int quantityBlocks = (int)(size/BLOCKSIZE) + 1;
	int freeblock, i;	
	int freeblock = search_free_blocks(quantityBlocks);
	for ( int = 0; i < quantityBlocks; i ++){
		ret.direct_blocks[i] = freeblock + i;
	}
	return ret;
}

/*
typedef struct{
	int identifier;
	int iNode_Number;
	int uid;
	int gid;
	int mode;
	int size;
	date creationTime;
	date modifiedTime;
	dataStream data;
}iNode;*/

void fs_write_inode(void * data){	
	
	//TODO: Busco el inodo y me lo traigo.
	//TODO: Dependiendo de la opcion o escribo al principio de todo o al final lo apendeo
	//TODO: En caso de escribir al principio me traigo el primer bloque y escribo hasta terminar.
	//TODO: En caso de escribir al final me traigo el ultimo bloque y escribo.
	//TODO: Si necesito mas bloques me fijo lo que me falta escribir y me traigo el numero de bloques que me faltan libres y escribo en todos ellos.
	//TODO: Una vez que tengo toda la data actualizada en el inodo lo pongo en disco actualizado y listo.
}

void fs_read_inode(){
	//TODO: Busco inodo y me lo traigo
	//TODO: En base al offset y cuanto quiero leer traigo el bloque correspondiente.
	//TODO: Leo y voy metiendo en el buffer hasta terminar.
	//TODO: Retorno.
}

/*
 *
 *	Virtual FileSystem Functions
 *
 */

//TODO: CREAT, OPEN, WRITE, READ, CLOSE

int creat (const char *filename, mode_t mode){
	return 0;
}

int open (const char *filename, int flags[, mode_t mode]){
	return 0;
}

int read(int fd, char *buf, int n){
	return 0;
}

int write(int fd, char *buf, int n){
	return 0;
}

int close(int fd){
	return 0;
}

//TODO: Yo recibo un buffer.

//TODO: Si creo y remuevo directorios no va a funcionar porque quedan index vacios en el medio sin usar.

//TODO: VFS
//	Falta el nodo virtual que permita toda la funcionalidad del lado del usuario para que el lado del kernel no se vea.


