/*********************************************
kasm.h

************************************************/
#ifndef __KASM_H_
#define __KASM_H_

#include "defs.h"


unsigned int read_msw();

void lidt (IDTR *idtr);
void lgdt (GDTR *gdtr);

void mascaraPIC1 (byte mascara);  /* Escribe mascara de PIC1 */
void mascaraPIC2 (byte mascara);  /* Escribe mascara de PIC2 */

void Cli(void);		/* Deshabilita interrupciones */
void Sti(void);		/* Habilita interrupciones */

void int_08_hand(void);		/* Timer tick */
void int_09_hand(void);		/* Keyboard */
void int_1_hand(void);
void invop_hand(void);
void snp_hand(void);
void bounds_hand(void);
void ssf_hand(void);
void div0_hand(void);
void gpf_hand(void);
void hacersnp(void);
void hacerbounds(void);
void hacergpf(void);
void hacerssf(void);
void hacerinvop(void);
void getgdt(void);
void debugBuenaOnda (void);
void movercursor(void);
void beep(void);
void seteards(void);
void setearcs(void);
void setearss(void);
void ejecutar(void);


#endif
