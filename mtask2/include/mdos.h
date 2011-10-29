#ifndef MDOS_H
#define MDOS_H

typedef int (*InitFunc_t)(void);
typedef int (*OpenFunc_t)(char *name);
typedef int (*ReadWriteFunc_t)(unsigned minor, void *buf, unsigned size);
typedef int (*ControlFunc_t)(unsigned minor, unsigned command, 
								void *info, unsigned size);


#endif
