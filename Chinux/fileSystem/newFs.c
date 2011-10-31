/********************************** 
*
*  newFs.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*  	Loreti, Nicolas
*		ITBA 2011
*
***********************************/

#include "../include/fs.h"
#include "../include/kernel.h"
#include "../include/defs.h"

extern user currentUsr;


int write_disk(int ata, int sector, void * msg, int count, int offset){
	
	return _disk_write(0x1f0, (char *)msg,count/512,sector+1);
	
}

int read_disk(int ata,int sector, void * msg, int count, int lenght){
	
	return _disk_read(0x1f0,(char*)msg,count/512,sector+1);
	
}

void init_filesystem( char * filesystem_name, masterBootRecord * mbr){
	
	/*mbr sector*/
	int fd;
	user * users = calloc(sizeof(user), 100);
	memcpy(users[0].name, "chinux", str_len("chinux"));
	memcpy(users[0].password, "teta", str_len("teta"));
	users[0].usrID = 1;
	users[0].group = ADMIN;
	mbr->existFS = 1;

	write_disk(0,MBRSECTOR,mbr,BLOCK_SIZE,0);//BLOCK_SIZE
	
	/* superBlock sector */
	superblock->name = "Chinux";
	superblock->blockSize = BLOCK_SIZE;
	superblock->freeBlocks = DISK_SIZE/BLOCK_SIZE;
	superblock->usedBlocks = 0;
	superblock->root = NULL; 
	write_disk(0,SUPERBLOCKSECTOR,superblock,BLOCK_SIZE,0);

	/* bitmap Sectors */
	init_bitmap();
	
	/* inodemap Sectors */
 	init_inodemap();

	/* Root node & current node */
	
	init_root();
	write_disk(0,1,superblock,BLOCK_SIZE,0);
	current = superblock->root;
	
	makeDir("users");
	makeDir("etc");

	
	fd = do_creat("usersfile", 777);
	write(fd, (char *)users, sizeof(user) * 100);
	close(fd);
	
	return;
}

void create_n_bytes( char * name ){
	
	int size,fd,i,a;
	size = 10485760;//10mb
	int cant = size / (512*128);
	
	char * buffer = malloc(BLOCK_SIZE*128);
	
	printf("SIZE:%d\n",size);	
	for( i = 0; i < (512*128); i++){
		buffer[i] = '2';
	}
	buffer[i]='\0';

	printf("len:%d\n",str_len(buffer));
	fd = do_creat(name,777);
	printf("cant:%d\n",cant);
	for ( i = 0; i<cant;i++){
		a = write(fd,buffer,str_len(buffer));
		printf("i:%d,a:%d\n",i,a);
	}
	printf("Se escrbieron:%d\n",getsize(fd));
	
	close(fd);

	return;
	
}

void load_filesystem(){
	read_disk(0,SUPERBLOCKSECTOR,superblock,BLOCK_SIZE,0);
	read_disk(0,BITMAPSECTOR,bitmap,BITMAP_SIZE,0);
	read_disk(0,INODEMAPSECTOR,inodemap,INODEMAP_SIZE,0);
}


void init_bitmap(){
	
	int i;
	clear_all(BITMAP);
	
	for(i=0;i<HEADER_BLOCKS;i++){
		set_bit(i,BITMAP);
	}
	
	write_disk(0,BITMAPSECTOR,bitmap->data,BITMAP_SIZE,0);
	return;
}


void init_inodemap(){
	
	clear_all(INODEMAP);
	set_bit(0,INODEMAP);
	write_disk(0,6,bitmap->data,INODEMAP_SIZE,0);
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
	if ( superblock->freeBlocks < 100){
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
	//If the candidate is OK, proceed tu fullfill the array of bits. 
	for ( i = candidate; i < (candidate + quantityBlocks); i++){
		set_bit(i,BITMAP);
	}

	superblock->freeBlocks = superblock->freeBlocks - quantityBlocks;
	superblock->usedBlocks = superblock->usedBlocks + quantityBlocks;
	
	printf("Cantidad de bloques libres:%d\n",superblock->freeBlocks);
	
	write_disk(0,BITMAPSECTOR,bitmap,BITMAP_SIZE,0);

	return candidate;

}

void free_used_blocks(int init_bit, int quantity, int mode){
	
	int i;
	for( i = init_bit; i < (init_bit + quantity); i++){
		clear_bit(i,mode);
	}

	superblock->freeBlocks = superblock->freeBlocks + quantity;
	superblock->usedBlocks = superblock->usedBlocks - quantity;

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
	
	inode->identifier = id;
	inode->iNode_number = search_free_inode();
	inode->uid = currentUsr.usrID;
	inode->gid = currentUsr.group;
	inode->mode = md;
	inode->size = sz;
	if( id == FIFO ){
		inode->data = *fs_init_fifoStream(sz,id,inode->iNode_number,current);	
	}else{
		inode->data = *fs_init_dataStream(sz,id,inode->iNode_number,current);

	}
	return;
}

dataStream * fs_init_fifoStream(int size,int id,int number,iNode * current){

	dataStream * ret = (dataStream *)malloc(sizeof(dataStream));
	char * pointer = (char *)malloc(sizeof(char) * MAX_FIFO_SIZE);
	
	ret->direct_blocks[0] = (int)pointer;
	ret->direct_blocks[1] = size;
	
	return ret;
}

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
		write_disk(0,ret->direct_blocks[0],dr,BLOCK_SIZE*12,0);
	}else{
		quantityBlocks = (int)(size/BLOCK_SIZE) + 1;
		freeblock = search_free_blocks(quantityBlocks);
		if ( freeblock != -1){
			ret->direct_blocks[0] = freeblock;
			ret->direct_blocks[1] = quantityBlocks;
		}else{
			printf("FAIL: not enoguht memory\n");
		}
	}
	return ret;
}

iNode * fs_get_inode(int number){

	int sector = number/4;
	int offset = number%4;
	iNode * ret = (iNode*)malloc(sizeof(iNode));
	void * recieve = (void *)malloc(512);
	
	if ( get_bit(number, INODEMAP) != 0 ){
		read_disk(0,INODETABLESECTOR + sector,recieve,BLOCK_SIZE,0);
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
	void * recieve = (void *)malloc(BLOCK_SIZE);
	
	
	if ( get_bit(number, INODEMAP) == 0 ){
		set_bit(number,INODEMAP);	
	}

	read_disk(0,INODETABLESECTOR+sector,recieve, BLOCK_SIZE,0);
	memcpy(recieve+(128*offset),node,128);//+(128*offset)
	write_disk(0,INODETABLESECTOR+sector,recieve,BLOCK_SIZE,0);

	return number;
}



iNode * search_directory(char * name, iNode * actual_node){
			
	int init_block = actual_node->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(64*96,1);
	read_disk(0,init_block,dr,(BLOCK_SIZE*12),0);
	int i;
	for(i=1;i<96;i++){
		if( strcmp(name,dr[i].name) == 1){
			return fs_get_inode(dr[i].inode);
		}
	}
	return NULL;
}

void print_directories(iNode * current){
	
	int init_block = current->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(0,init_block,dr,BLOCK_SIZE*12,0);
	
	int i;
	for(i=0;i<96;i++){
		if( dr[i].type != 0 && (dr[i].name[0] != '.' || !dr[i].name[1] || (dr[i].name[1] == '.' && !dr[i].name[2]))){
			printf("%s ", dr[i].name);
		}
	}
	printf("\n");
	return;
}

void insert_directory( char * name, iNode * current ){
	
	iNode * newDirectory = (iNode *)malloc(sizeof(iNode));
	newDirectory =  fs_creat_inode(DIRECTORY,777,512,current);
	fs_insert_inode(newDirectory);
	insert_directory_entry(newDirectory,current,name);
	
}

void insert_directory_entry(iNode * newDirectory, iNode * current, char * name){
	int init_block = current->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(0,init_block,dr,BLOCK_SIZE*12,0);
	int i;
	for ( i = 0; i < 96; i++){
		if ( dr[i].type == 0 ){
			dr[i].type = DIRECTORY;
			dr[i].inode = newDirectory->iNode_number;
			dr[i].lenght = 0;
			memcpy(dr[i].name,name,str_len(name));
			break;
		}
	}
	write_disk(0,init_block,dr,BLOCK_SIZE*12,0);

	return;
	
}


iNode * insert_file( char * name, int mode, iNode * current ){

	
	iNode * newFile = (iNode *)malloc(sizeof(iNode));
	if( ( newFile = search_directory(name, current) ) != NULL){		
		return NULL;
	}	
	newFile =  fs_creat_inode(FILE,mode,0,current);
	fs_insert_inode(newFile);

	insert_file_entry(newFile,current,name);

	return newFile;
	
	
}

iNode * insert_fifo( char * name, int size, iNode * current2 ){

	
	iNode * newFifo = (iNode *)malloc(sizeof(iNode));

	newFifo =  fs_creat_inode(FIFO,1,size,NULL);
	fs_insert_inode(newFifo);

	insert_fifo_entry(newFifo,current,name);

	return newFifo;
	
}

void insert_fifo_entry(iNode * newFile, iNode * current, char * name){
	int init_block = current->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(0,init_block,dr,BLOCK_SIZE*12,0);
	int i;
	for ( i = 0; i < 96; i++){
		if ( dr[i].type == 0 ){
			dr[i].type = FIFO;
			dr[i].inode = newFile->iNode_number;
			dr[i].lenght = 0;
			memcpy(dr[i].name,name,str_len(name));
			break;
		}
	}
	write_disk(0,init_block,dr,BLOCK_SIZE*12,0);

	return;
	
}

void insert_file_entry(iNode * newFile, iNode * current, char * name){
	int init_block = current->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(0,init_block,dr,BLOCK_SIZE*12,0);
	int i;
	for ( i = 0; i < 96; i++){
		if ( dr[i].type == 0 ){
			dr[i].type = FILE;
			dr[i].inode = newFile->iNode_number;
			dr[i].lenght = 0;
			memcpy(dr[i].name,name,str_len(name));
			break;
		}
	}
	write_disk(0,init_block,dr,BLOCK_SIZE*12,0);

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
				i++;
			}else
			{
				status = CARACTER;
				buffer[j++] = path[i++]; 
				buffer[j];			
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
				i++;
				
			}
			else if ( path[i] == '.' )
			{
				if ( path[i+1] != '/' && path[i+1] != '\0')
				{
					if ( i > 2 && path[i-2] != '/')
						path_status = WRONG_PATH;				
				}
				else{
					posible_inode = search_directory("..",posible_inode);
					i++;
					
				}
				status = PUNTO;

			}else
			{
				status = CARACTER;
				buffer[j++] = path[i++];
				buffer[j];			
			}
		}
		else if ( status == CARACTER )
		{
			if ( path[i] == '\0')
			{
				path_parsing = END_PATH;
				path_status = OK_STATUS;
				buffer[j] = '\0';
			
				if( ( temp_inode = search_directory(buffer, posible_inode) ) != NULL)
				{
					posible_inode = temp_inode;
					j=0;								
				}else
				{
					path_status = WRONG_PATH;
				}			
			}
			else if ( path[i] == '/')
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
			else if ( path[i] == '.' )
			{
				status = PUNTO;
				buffer[j++] = path[i];
			}else
			{
				status = CARACTER;
				buffer[j++] = path[i];
				buffer[j];
			}
	
			i++;
		}
	}

	if ( path_status == WRONG_PATH ){
		return NULL;	
	}
	return posible_inode;
}


void cd_in_kernel(char * path){

	iNode * posible_inode = current;
	posible_inode = parser_path(path, posible_inode);

	if(posible_inode->gid < currentUsr.group && posible_inode->iNode_number != superblock->root->iNode_number)
	{
		printf("\nCan not acces directory %s. Admin permissions required.", path);
		return ;
	}

	if ( posible_inode == NULL )
	{
		printf("\nWrong name or path");
	}else
	{
		current = posible_inode;
	}
	return;

}


void makeDir(char * newName){

	char * parcialName = (char*)malloc(25);
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
				makeDir_current = posibleDir_current;
			}
			i=j-1;
	
	}

	return;
}


void ls_in_kernel(char * path){
	
	printf("\n");
	print_directories(current);
	
	
}

void rmDir( char * path ){

	int i , j, ret;
	iNode * posible_inode = current;
	posible_inode = parser_path(path, posible_inode);

	if ( posible_inode == NULL )
	{
		printf("Wrong name or path\n");
		return;
	} 

	if(posible_inode->gid < currentUsr.group)
	{
		printf("\nCan not remove %s. Permission denied.", path);
		return ;
	}
	if( posible_inode->identifier != DIRECTORY ){
		
		int inode_number = posible_inode->iNode_number;
		int init_block = current->data.direct_blocks[0];
		directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
		read_disk(0,init_block,dr,BLOCK_SIZE*12,0);
		for ( i = 2; i < 96; i++){
			if ( dr[i].inode == inode_number){
				char * empty_name = "\0";				
				dr[i].type = 0;
				dr[i].inode = 0;
				dr[i].lenght = 0;
				strcopy(dr[i].name,empty_name,1);				
				break; 
			}
		}
		write_disk(0,init_block,dr,BLOCK_SIZE*12,0);
	}
	else
	{		
		//BORRADO RECURSIVO.
		ret = recursive_remove(posible_inode);
		
		if(ret)
			return;
		int inode_number = posible_inode->iNode_number;
		int init_block = current->data.direct_blocks[0];
		directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
		read_disk(0,init_block,dr,BLOCK_SIZE*12,0);
		iNode * parent = fs_get_inode(dr[1].inode);
		int father_init_block = current->data.direct_blocks[0];
		directoryEntry * father_dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
		read_disk(0,father_init_block,father_dr,BLOCK_SIZE*12,0);
		
		for ( i = 2; i < 96; i++){
			if ( father_dr[i].inode == inode_number){
				char * empty_name = "\0";				
				dr[i].type = 0;
				dr[i].inode = 0;
				dr[i].lenght = 0;
				strcopy(dr[i].name,empty_name,1 );				
				break; 
			}
		}
		write_disk(0,init_block,dr,BLOCK_SIZE*12,0);
	}
	return;
}

int is_base_case( iNode * current ){

	if ( current->identifier != DIRECTORY ){
		return 1;
	}
	int init_block = current->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(0,init_block,dr,BLOCK_SIZE*12,0);
	int i;
	for ( i = 2; i < 96; i++){
			if ( dr[i].type != 0 ){
				return 0;
			}
	}
	return 1;
}

int recursive_remove( iNode * current ){

	int ret;
	

	if(current->gid < currentUsr.group)
		return 1;

	if( is_base_case(current)) //CASOBASE QUE ES QUE EL DIRECTORIO ESTE VACIO O SEA UN ARCHIVO)
		return 0;

	int init_block = current->data.direct_blocks[0];
	directoryEntry * dr = (directoryEntry*)calloc(sizeof(directoryEntry),96);
	read_disk(0, init_block, dr, BLOCK_SIZE * 12, 0);
	int i;
	for ( i = 2; i < 96; i++){
		if ( dr[i].type != 0 ){
			ret = recursive_remove(fs_get_inode(dr[i].inode));
			if(!ret)
			{
				dr[i].type = 0;
				dr[i].inode = 0;
				dr[i].lenght = 0;
			}
			
		}
	}
	write_disk(0,init_block,dr,BLOCK_SIZE*12,0);

	return ret;
}

int write_inode(iNode * inode, char * buf, int n){	
		
	int file_size = inode->size;
	int posible_requeried = (int)((file_size+n)/BLOCK_SIZE) + 1;
	int newrequeried_blocks,freeblock;
	if ( posible_requeried > inode->data.direct_blocks[1] ){
		newrequeried_blocks = posible_requeried;
		freeblock = search_free_blocks(newrequeried_blocks);
	}else{
		freeblock = inode->data.direct_blocks[0];
		newrequeried_blocks = inode->data.direct_blocks[1];
	}
	
	int i,lastblock;
	int init_block = inode->data.direct_blocks[0]; 
	int quantity = inode->data.direct_blocks[1];

	if ( freeblock != -1 ){	
		char * buffer = (char *)malloc(quantity*512);
		char * insert_buffer = (char *)malloc(newrequeried_blocks * 512);

		read_disk(0,init_block,buffer,quantity*BLOCK_SIZE,0);

		memcpy(insert_buffer,buffer,quantity*512);
		memcpy((insert_buffer+file_size),buf,n);
		write_disk(0,freeblock,insert_buffer,(newrequeried_blocks*512),0);

		inode->data.direct_blocks[0] = freeblock;
		inode->data.direct_blocks[1] = newrequeried_blocks;
		inode->size = file_size + n;

		free_used_blocks(init_block,quantity, BITMAP);

		fs_insert_inode(inode);	
	}
	
	return n;	
}


int read_inode(iNode * inode, char * buf, int n){
	
	int file_size = inode->size;
	int newrequeried_blocks = inode->data.direct_blocks[1] + (int)(n/BLOCK_SIZE) + 1;
	int i,lastblock;
	int init_block = inode->data.direct_blocks[0]; 
	int quantity = inode->data.direct_blocks[1];

	char * receive_buffer = (char *) malloc(quantity*512);

	if ( n < (quantity*BLOCK_SIZE) ){
		read_disk(0,init_block,receive_buffer,quantity*512,0);
		memcpy(buf,receive_buffer,n);
	}else{
		read_disk(0,init_block,receive_buffer,quantity*512,0);
		memcpy(buf,receive_buffer,n);

	}
	
	return n;	

}

int do_creat(char * filename, int mode){

	int i;
	iNode * ret;
	if ( (ret  = insert_file(filename,mode,current)) == NULL){
		return -1;	
	} 
	int fd = insert_fd(ret->iNode_number);
	return fd;

}

int do_open(char * filename, int flags, int mode){
	
	iNode * posible_file = search_directory(filename, current);

	int fd;	
	if ( posible_file != NULL)
	{	
		if(posible_file->gid < currentUsr.group)
			return -2;

		if( posible_file->identifier == LINK ){
			posible_file = fs_get_inode(posible_file->link);		
		}		
		if ( (fd = search_for_inode(posible_file->iNode_number)) != -1){
			return fd;
		}else{
			return insert_fd(posible_file->iNode_number);
		}
	}else
	{
		if(flags == 0){
			do_creat(filename,mode);
		}else{
			return -1;
		}
				
	}
			
}

int do_write(int fd, char * buf, int n){
	
	int inode_number = search_for_fd(fd);//search fd in inode;
	if ( inode_number == -1){
		return -1;
	}
	iNode * inode =	fs_get_inode(inode_number);
	return write_inode(inode, buf,n);
	
}

int do_read(int fd, char * buf, int n){

	int inode_number,quant;
	inode_number = search_for_fd(fd);
	if ( inode_number == -1){
		return -1;
	}
	iNode * inode =	fs_get_inode(inode_number);
	if( n == -1 ){ //to read all the file.
		n = inode->size;	
	}
	quant = read_inode(inode, buf,n);
	return quant;
}

int do_close(int fd){
	return delete_fd(fd);
}



int search_for_inode( int inodenumber ){
	int i;
	for ( i=3; i<100;i++){
		if ( inodenumber == fd_table[i].inode ){
			return fd_table[i].fd;
		}
	}
	return -1;
}

int search_for_fd(int fd){
	int i;
	for ( i=3; i<100;i++){
		if ( fd == fd_table[i].fd ){
			return fd_table[i].inode;
		}
	}
	return -1;
}

int insert_fd(int inode_number){
	int i;
	for(i=3;i<100;i++){
		if ( fd_table[i].fd == 0){
			fd_table[i].fd = i;
			fd_table[i].inode = inode_number;
			return i;
		}
	}
	return -1;
}

int delete_fd(int filedescriptor){
	fd_table[filedescriptor].fd = 0;
	fd_table[filedescriptor].inode = 0;
	return 1;
}




/*
 *
 *	Auxiliary Functions
 *
 */

void substr(char dest[], char src[], int offset, int len)
{
	int i;
	for(i = 0; (i+offset) < len && src[offset + i] != '\0'; i++){
	dest[i] = src[i + offset];
	}
	dest[i] = '\0';
	
	return;
}

/*
 *
 *	Virtualization Functions
 *
 */



int creat_in_kernel(creat_param * param){
	
	char * filename = malloc(str_len(param->filename));
	int mode = param->mode;
	memcpy(filename,param->filename,str_len(filename));
	return do_creat(filename,777);

}


int open (char *filename, int flags, int mode){
	do_open(filename,flags,mode);
}

int read(int fd, char * buf, int n){
	do_read(fd,buf,n);
}

int write(int fd, char *buf, int n){
	do_write(fd,buf,n);
}

int close(int fd){
	do_close(fd);
}


void touch_in_kernel( char * filename ){
	
	int fd;
	if ( (fd = do_creat(filename,777)) == -1){
		printf("\nCreat error");
	}else{
		printf("\nFile created succesfully");
	}
	
	return;
	
}

void writefile_in_kernel( char * name, char * buffer ){

	int fd;
	int lenght = str_len(name);
	name[lenght -1] = '\0';
	printf("Buffer:%s\n",buffer);
	if ( (fd = do_open(name,1,777) ) == -1 ){
		printf("File not exist");
	}else{
		write(fd,buffer,str_len(buffer));
		printf("tam:%d\n",getsize(fd));
	}
	return;
}

int getsize(int filedescriptor){

	iNode * nodo = fs_get_inode(search_for_fd(filedescriptor));
	return nodo->size;

}

int getidentifier(int filedescriptor){
	iNode * nodo = fs_get_inode(search_for_fd(filedescriptor));
	return nodo->identifier;
}

void cat_in_kernel( char * filename ){
	
	int fd;
	if ( ( fd = do_open(filename,1,2) ) == -1){
		printf("File not exist\n");
		return;
	}
	if ( getidentifier(fd) != FILE && getidentifier(fd) != LINK ){
		printf("Error: files only\n");
	}else{
		char * buffer = malloc(getsize(fd));
		read(fd,buffer,-1);
		printf("\n%s",buffer);
	}
}


void link_in_kernel(link_struct * param){
	char * path1 = param->path1;
	char * path2 = param->path2;
	
	if ( strcmp("hola",path1) == 1){
		printf("ENTRO\n");
	}	
	int path2_len, i, index_file_name,quant_chars;
	char * directory_path;
	char * name;
	iNode * path1_inode = current;
	
	path1_inode = search_directory(path1,superblock->root);
	
	if ( path1_inode == NULL )
	{
		printf("Wrong name or path\n");
	}

	path2_len = str_len(path2);
	for ( i = path2_len; i >= 0; i--){
		if ( path2[i] == '/' ){
			index_file_name = i;
			break;
		}
	}

	if( i >= 0 ){
		quant_chars = path2_len - (path2_len - index_file_name) + 1;
		directory_path = malloc(quant_chars);
		name = malloc(path2_len - quant_chars);
		memcpy(directory_path,path2,quant_chars);
		memcpy(name,path2+quant_chars,path2_len-quant_chars);
		
		iNode * path2_inode = current;
		path2_inode = parser_path(directory_path, path2_inode);
		iNode * link_node = insert_file(name,777,path2_inode);
		copy_link_inode(path1_inode, link_node);
	}else{

		iNode * path2_inode = current;
		iNode * link_node = insert_file(path2,2,path2_inode);
		
		ls("");
		copy_link_inode(path1_inode, link_node);
		fs_insert_inode(link_node);
	}
	
	return;
}


void links(char * path1, char * path2){
	

	
	int path1_len = str_len(path1);
	path1[path1_len-1] = '\0';
		
	int path2_len, i, index_file_name,quant_chars;
	char * directory_path;
	char * name;
	iNode * path1_inode = current;
	path1_inode = parser_path(path1, path1_inode);

	
	if ( path1_inode == NULL )
	{
		printf("Wrong name or path\n");
	}

	// Search for the first / from right to left, to get the path and the
	//filename.
	path2_len = str_len(path2);
	for ( i = path2_len; i >= 0; i--){
		if ( path2[i] == '/' ){
			index_file_name = i;
			break;
		}
	}
	//if I find one
	if( i >= 0 ){
		quant_chars = path2_len - (path2_len - index_file_name) + 1;
		directory_path = malloc(quant_chars);
		name = malloc(path2_len - quant_chars);
		memcpy(directory_path,path2,quant_chars);
		memcpy(name,path2+quant_chars,path2_len-quant_chars);
		name[path2_len - quant_chars] = '\0';
		directory_path[quant_chars-1] = '\0';		
		
		iNode * path2_inode = current;
		path2_inode = parser_path(directory_path, path2_inode);
		
		iNode * link_node = insert_file(name,777,path2_inode);
		copy_link_inode(path1_inode, link_node);
	}else{
	
		printf("ENTRO");
		iNode * path2_inode = current;
		iNode * link_node = insert_file(path2,2,path2_inode);
		
		copy_link_inode(path1_inode, link_node);
		printf("ID:%d\n",link_node->identifier);
		fs_insert_inode(link_node);
	}
	
	return;
}


void copy_link_inode(iNode * inode, iNode * reciever_inode){
	
	reciever_inode->identifier = LINK;
	reciever_inode->link = inode->iNode_number;

}
