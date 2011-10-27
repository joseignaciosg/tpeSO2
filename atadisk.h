#ifndef __ATADISK_H__
#define __ATADISK_H__
#include "smdio.h"

#define BLOCK_SIZE       512    /* physical sector size in bytes */

#define ATA_GET_CAPACITY  123

void init_atadisk_fd ( void );

#endif


