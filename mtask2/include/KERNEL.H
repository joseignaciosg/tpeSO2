#ifndef KERNEL_H
#define KERNEL_H

#include "mtask.h"
#include "softint.h"

#define FAR_DATA	(sizeof(void *) == sizeof(long))
#define NEAR_DATA	(!FAR_DATA)
#define FAR_CODE	(sizeof(void(*)(void)) == sizeof(long))
#define NEAR_CODE	(!FAR_CODE)
#define even(n)		((n) & ~1)

/* Simbolos globales internos del kernel */

/* kernel.c */

extern Task_t *		mt_curr_task;
bool				mt_select_task(void);

/* queue.c */

void				mt_enqueue(Task_t *task, TaskQueue_t *queue);
void				mt_dequeue(Task_t *task);
Task_t *			mt_peeklast(TaskQueue_t *queue);
Task_t *			mt_getlast(TaskQueue_t *queue);

void				mt_enqueue_time(Task_t *task, Time_t ticks);
void				mt_dequeue_time(Task_t *task);
Task_t *			mt_peekfirst_time(void);
Task_t *			mt_getfirst_time(void);

/* irq.c */

extern unsigned 	mt_irq_number;
extern unsigned 	mt_int_level;
void interrupt		mt_hw_vector(void);
void 				mt_setup_irqs(void);
void 				mt_restore_irqs(void);

/* hardvec.asm */

void interrupt		mt_hw_vec_0(void);
void interrupt		mt_hw_vec_1(void);
void interrupt		mt_hw_vec_2(void);
void interrupt		mt_hw_vec_3(void);
void interrupt		mt_hw_vec_4(void);
void interrupt		mt_hw_vec_5(void);
void interrupt		mt_hw_vec_6(void);
void interrupt		mt_hw_vec_7(void);
void interrupt		mt_hw_vec_8(void);
void interrupt		mt_hw_vec_9(void);
void interrupt		mt_hw_vec_10(void);
void interrupt		mt_hw_vec_11(void);
void interrupt		mt_hw_vec_12(void);
void interrupt		mt_hw_vec_13(void);
void interrupt		mt_hw_vec_14(void);
void interrupt		mt_hw_vec_15(void);

/* math.c */

extern unsigned		mt_reserved_stack;
void				mt_setup_math(void);
void				mt_init_math(Task_t *task);
void				mt_save_math(Task_t *task);
void				mt_restore_math(Task_t *task);

/* hiregs.asm */

unsigned			mt_gethi_eax(void);
unsigned			mt_gethi_ebx(void);
unsigned			mt_gethi_ecx(void);
unsigned			mt_gethi_edx(void);
unsigned			mt_gethi_esi(void);
unsigned			mt_gethi_edi(void);
void				mt_sethi_eax(unsigned value);
void				mt_sethi_ebx(unsigned value);
void				mt_sethi_ecx(unsigned value);
void				mt_sethi_edx(unsigned value);
void				mt_sethi_esi(unsigned value);
void				mt_sethi_edi(unsigned value);

#endif
