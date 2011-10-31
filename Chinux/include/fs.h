/********************************** 
*
*  fs.h
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*  	Loreti, Nicolas
*		ITBA 2011
*
***********************************/

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


/* File System Defines */

#define NUMBER_DIRECT_BLOCKS 12
#define MAX_BLOCKS_NUMBER 500
#define NULL 0
#define BLOCK_SIZE 512
#define	BITMAPSECTOR (SUPERBLOCKSECTOR+1)
#define MBRSECTOR 0
#define	SUPERBLOCKSECTOR (MBRSECTOR + 1)
#define INODEMAPSECTOR (BITMAPSECTOR + (BITMAP_SIZE/512) )
#define INODETABLESECTOR (INODEMAPSECTOR + (INODEMAP_SIZE/512) )
#define BITMAP_SIZE (DISK_SIZE/512)
#define INODEMAP_SIZE	(DISK_SIZE/512/4)
#define DISK_SIZE 20971520 //20mb
#define INODETABLE_SIZE (INODEMAP_SIZE * 4)
#define BITMAP 1
#define INODEMAP 2
#define HEADER_BLOCKS ((BLOCK_SIZE + BLOCK_SIZE + BITMAP_SIZE + INODEMAP_SIZE + INODETABLE_SIZE + BLOCK_SIZE)/512)
#define MAX_FIFO_SIZE 1000
#define N 32

/* Defines for Functions */

#define MKDIR 0
#define CD 1
#define LS 2
#define RMDIR 3

/* File Types */

#define UNKNOWN 0
#define FILE 1
#define DIRECTORY 2
#define LINK 8
#define FIFO 9

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
	int link;
	dataStream data;
	char relleno[52];
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


typedef struct{
	int fd;
	int inode;
	int sem_key;
}filedescriptor;

typedef int word_t;

enum { WORD_SIZE = sizeof(word_t) * 8 };

/* 
 *
 *	Read & Write Disk Functions
 *
 */

int write_disk(int ata, int sector, void * msg, int count, int offset);
int read_disk(int ata,int sector, void * msg, int count, int lenght);

/*
 *	
 *	BITMAP & INODEMAPFunctions
 *
 */

/************************************************************************
 * init_bitmap()
 *
 * Initialize bitmap structure 
 * *********************************************************************/
void init_bitmap();

/************************************************************************
 * init_inodemap()
 *
 * Init inodemap structure
 * *********************************************************************/
void init_inodemap();

/************************************************************************
 * init_filesystem()
 *
 * Init all filesystem structures
 * *********************************************************************/
void init_filesystem( char * filesystem_name, masterBootRecord * mbr);

/************************************************************************
 * load_filesystem()
 *
 * load all the data structures for fs in disk.
 * *********************************************************************/
void load_filesystem();

/************************************************************************
 * bindex()
 *
 * bitmap function for shiftting.
 * *********************************************************************/
inline int bindex(int b);

/************************************************************************
 * boffset()
 *
 * bitmap function for shiftting
 * *********************************************************************/
inline int boffset(int b);

/************************************************************************
 * set_bit()
 *
 * set bit b in 1
 * *********************************************************************/
void set_bit(int b, int mode);

/************************************************************************
 * clear_bit()
 *
 * set bit in 0.
 * *********************************************************************/
void clear_bit(int b, int mode); 

/************************************************************************
 * get_bit()
 *
 * returns the value of the bit b.
 * *********************************************************************/
int get_bit(int b, int mode);

/************************************************************************
 * clear_all()
 *
 * set all bits in 0.
 * *********************************************************************/
void clear_all(int mode);

/************************************************************************
 * set_all()
 *
 * set all bits in 1.
 * *********************************************************************/
void set_all(int mode);

/************************************************************************
 * search_free_blocks()
 *
 * search the amount of freeblocks in disk given by the parameter
 * *********************************************************************/
int search_free_blocks(int quantityBlocks);

/************************************************************************
 * free_used_blocks()
 *
 * set in 0 the amount of blocks set in quantity, starting at init_block
 * *********************************************************************/
void free_used_blocks(int init_block, int quantity, int mode);


/*
 *	
 *	FileSystem Structures Functions
 *
 */

/************************************************************************
 * fs_creat_inode()
 *
 * Creates an inode based on the parameters.
 * *********************************************************************/
iNode * fs_creat_inode(int identifier, int mode, int size, iNode * current);

/************************************************************************
 * fs_init_inode()
 *
 * initialize an inode based on the parameters.
 * *********************************************************************/
void fs_init_inode( iNode * inode, int id, int md, int sz, iNode * current);

/************************************************************************
 * fs_init_dataStream()
 *
 * init a dataStream structure based on the paramaters.
 * *********************************************************************/
dataStream * fs_init_dataStream(int size,int id, int number, iNode * current);

/************************************************************************
 * fs_get_inode()
 *
 * returns an inode base on the number of inode.
 * *********************************************************************/
iNode * fs_get_inode(int number);

/************************************************************************
 * fs_insert_inode()
 *
 * insert an inode in disk.
 * *********************************************************************/
int fs_insert_inode(iNode * node);

/************************************************************************
 * init_root()
 *
 * initialize the root variable.
 * *********************************************************************/
void init_root();

/************************************************************************
 * insert_directory()
 *
 * insert a directory in the filesystem
 * *********************************************************************/
void insert_directory( char * name, iNode * current );

/************************************************************************
 * insert_file()
 *
 * insert a file in the filesystem.
 * *********************************************************************/
iNode * insert_file( char * name, int mode, iNode * current );

/************************************************************************
 * insert_fifo()
 *
 * insert a fifo file in the filesystem.
 * *********************************************************************/
iNode * insert_fifo( char * name, int size, iNode * current );

/************************************************************************
 * insert_file_entry()
 *
 * Insert a file entry in fs.
 * *********************************************************************/
void insert_file_entry(iNode * newFile, iNode * current, char * name);

/************************************************************************
 * insert_fifo_entry()
 *
 * insert fifo entry in fs.
 * *********************************************************************/
void insert_fifo_entry(iNode * newFile, iNode * current, char * name);

/************************************************************************
 * insert_directory_entry()
 *
 * insert a directory entry in the fs.
 * *********************************************************************/
void insert_directory_entry(iNode * newDirectory, iNode * current, char * name);

/************************************************************************
 * recursive_remove()
 *
 * remove the child directories.
 * *********************************************************************/
int recursive_remove( iNode * current );

/************************************************************************
 * is_base_case()
 *
 * returns the basecase of the recursive
 * *********************************************************************/
int is_base_case( iNode * current );

/************************************************************************
 * search_directory
 *
 * given a name, search for it in the directory and returns.
 * *********************************************************************/
iNode * search_directory( char * name, iNode * node);

/************************************************************************
 *print_directories() 
 *
 * print the name of the directories.
 * *********************************************************************/
void print_directories(iNode * current);

/************************************************************************
 * parser_path()
 *
 * return an inode based on the path in the parameters.
 * *********************************************************************/
iNode * parser_path(char * path, iNode * posible_inode);

/************************************************************************
 * fs_init_fifoStream()
 *
 * initialize a fifo stream structure.
 * *********************************************************************/
dataStream * fs_init_fifoStream(int size,int id,int number,iNode * current);


/*
 *	
 *	Read & Write Inode Functions
 *
 */


/************************************************************************
 *read_inode() 
 *
 * reads n bytes of data from the inode.
 * *********************************************************************/
int read_inode(iNode * inode, char * buf, int n);

/************************************************************************
 * write_inode()
 *
 * writes n bytes of data from the buffer to the disk.
 * *********************************************************************/
int write_inode(iNode * inode, char * buf, int n);

/************************************************************************
 * search_for_inode()
 *
 * return the inode number from fdtable.
 * *********************************************************************/
int search_for_inode( int inodenumber );


/*
 *	
 *	VFS Functions
 *
 */

/************************************************************************
 * creat()
 *
 * creates a file.
 * *********************************************************************/
int creat (char *filename, int mode);

/************************************************************************
 * open()
 *
 * opens a file.
 * *********************************************************************/
int open (char *filename, int flags, int mode);

/************************************************************************
 * read()
 *
 * reads data from file.
 * *********************************************************************/
int read(int fd, char *buf, int n);

/************************************************************************
 *write() 
 *
 * write data from buffer to disk.
 * *********************************************************************/
int write(int fd, char *buf, int n);

/************************************************************************
 * close()
 *
 * close filedescriptor.
 * *********************************************************************/
int close(int fd);

/************************************************************************
 * do_creat()
 *
 * creates a file.
 * *********************************************************************/
int do_creat(char * filename, int mode);

/************************************************************************
 * do_open()
 *
 * open a file.
 * *********************************************************************/
int do_open(char * filename, int flags, int mode);

/************************************************************************
 * do_write()
 *
 * write data from buffer to disk.
 * *********************************************************************/
int do_write(int fd, char * buf, int n);

/************************************************************************
 * do_read()
 *
 * read data from disk.
 * *********************************************************************/
int do_read(int fd, char * buf, int n);

/************************************************************************
 * do_close()
 *
 * close a filedescriptor.
 * *********************************************************************/
int do_close(int fd);


/*
 *	
 *	Shell Assosiated Functions
 *
 */

void touch(char * filename);

void cat(char * filename);

void link (char * path1, char * path2);

void copy_link_inode(iNode * inode, iNode * reciever_inode);

void links(char * path1, char * path2);

void create_n_bytes( char * name );

void writefile_in_kernel( char * name, char * buffer );

void makeDir(char *);

void rmDir(char *);

void cd(char *);

void ls(char *);

/*
 *	
 *	File Descriptor Assosiated Functions
 *
 */

int getidentifier(int filedescriptor);

int getsize(int filedescriptor);

int search_for_fd(int fd);

int insert_fd(int inode);

int delete_fd(int filedescriptor);

/*
 *	
 *	Aux Functions
 *
 */

void substr(char dest[], char src[], int offset, int len);



//word_t data[N / 32 + 1];
BM * bitmap;
IM * inodemap;
masterBlock * superblock;
iNode * current;
filedescriptor * fd_table;
