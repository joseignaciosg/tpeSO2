#ifndef SOFTINT_H
#define SOFTINT_H

typedef struct
{
	unsigned short		bp, di, si, ds, es;
	unsigned short		dx, cx, bx, ax;
	unsigned short		ip, cs, flags;
}
WordRegs_t;

typedef struct
{
	unsigned short		bp, di, si, ds, es;
	unsigned char		dl, dh, cl, ch, bl, bh, al, ah;
	unsigned short		ip, cs, flags;
}
ByteRegs_t;

typedef union
{
	WordRegs_t			x;
	ByteRegs_t			h;
}
InterruptRegs_t;

typedef void interrupt	(*SoftHandler_t)(InterruptRegs_t regs);

SoftHandler_t	HookInterrupt(unsigned number, SoftHandler_t handler);
void 			CallInterrupt(SoftHandler_t handler, InterruptRegs_t *regs);

#endif
