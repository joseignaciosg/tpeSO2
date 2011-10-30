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
#define FILE 1
#define DIRECTORY 2
#define CHARACTER_DEVICE 3
#define BLOCK_DEVICE 4 
#define NAMED_PIPE 5
#define SOCKET 6 //NO USAR
#define SYMBOLIC_LINK 7 //NO USAR

/* NEW DEFINES */

#define NULL 0
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


typedef struct{
	int fd;
	int inode;
}filedescriptor;

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

int write_disk(int ata, int sector, void * msg, int count, int offset);
int read_disk(int ata,int sector, void * msg, int count, int lenght);

void init_bitmap();
void init_inodemap();
void init_filesystem( char * filesystem_name, masterBootRecord * mbr);
void load_filesystem();

void recursive_remove( iNode * current );
int is_base_case( iNode * current );

iNode * search_directory( char * name, iNode * node);
void print_directories(iNode * current);


iNode * parser_path(char * path, iNode * posible_inode);
void makeDir(char *);
void rmDir(char *);
void cd(char *);
void ls(char *);

/* VFS Functions Declartions */

int creat (char *filename, int mode);
int open (const char *filename, int flags, int mode);
int read(int fd, char *buf, int n);
int write(int fd, char *buf, int n);
int close(int fd);


int search_for_fd(int fd);
int insert_fd(int inode);
int delete_fd(int filedescriptor);
int do_close(int fd);

void insert_file_entry(iNode * newFile, iNode * current, char * name);
iNode * insert_file( char * name, int mode, iNode * current );

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

/* READ AND WRITE */

int do_creat(char * filename, int mode);
int do_write(int fd, char * buf, int n);
int do_read(int fd, char * buf, int n);
int read_inode(iNode * inode, char * buf, int n);
void write_inode(iNode * inode, char * buf, int n);
int do_open(char * filename, int flags, int mode);
int search_for_inode( int inodenumber );
void touch();
void cat(char * filename);
//word_t data[N / 32 + 1];
BM * bitmap;
IM * inodemap;
masterBlock * superblock;
iNode * current;
filedescriptor * fd_table;
