include model.inc
.386

; Funciones para leer y escribir la parte superior de los registros de 32 bits
; en procesadores 386 y superiores (eax, ebx, ecx, edx, esi, edi)
;
; Ejemplos: 
;
;	unsigned	mt_gethi_eax(void);
;	void		mt_sethi_eax(unsigned value);
;

declare_gethi_ereg macro reg

	public 		_mt_gethi_e&reg
_mt_gethi_e&reg	proc
	push		e&reg
	pop			ax
	pop			ax
	ret
_mt_gethi_e&reg	endp

endm

declare_sethi_ereg macro reg

	public 		_mt_sethi_e&reg
_mt_sethi_e&reg	proc
	push		bp
	mov			bp, sp
if FAR_CODE
	push		word ptr [bp+6]
else
	push		word ptr [bp+4]
endif
	push		reg
	pop			e&reg
	pop			bp
	ret
_mt_sethi_e&reg	endp

endm

.code

declare_gethi_ereg	ax
declare_gethi_ereg	bx
declare_gethi_ereg	cx
declare_gethi_ereg	dx
declare_gethi_ereg	si
declare_gethi_ereg	di

declare_sethi_ereg	ax
declare_sethi_ereg	bx
declare_sethi_ereg	cx
declare_sethi_ereg	dx
declare_sethi_ereg	si
declare_sethi_ereg	di

end
