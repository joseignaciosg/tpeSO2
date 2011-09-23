include model.inc

; CallInterrupt - Llama a un vector de interrupcion de software con un juego de 
;		          registros.
;
;		   void CallInterrupt(SoftHandler_t handler, InterruptRegs_t *regs);
;
;		   Devuelve en la misma estructura los valores de los registros
;		   segun quedaron al terminar la rutina de interrupcion.
;		   Esta funcion puede llamarse desde C; resguarda DS, BP, SI y DI.

INTERRUPT_REGS struc		; Estructura con los valores de los
							; registros, sirve para entrada y
	i_bp		dw ?		; salida del manejador de interrupcion.
	i_di		dw ?		; Pueden usarse todos los registros,
	i_si		dw ?		; salvo CS e IP.
	i_ds		dw ?		; Mantener en sincronismo con softint.h
	i_es		dw ?		;
	i_dx		dw ?		;
	i_cx		dw ?		;
	i_bx		dw ?		;
	i_ax		dw ?		;
	i_ip		dw ?		;
	i_cs		dw ?		;
	i_flags		dw ?		;
							;
ends						;

CHAIN_STACK struc			; Estructura del stack frame de la funcion
							; 
	saved_bp	dw ?		;
	saved_si	dw ?		;
	saved_di	dw ?		;
	saved_ds	dw ?		;
							;
if FAR_CODE					;
	return_addr	dd ?		;
else						;
	return_addr	dw ?		;
endif						;
							;
	handler		dd ?		;
							;
if FAR_DATA					;
	regs		dd ?		;
else						;
	regs		dw ?		;
endif						;
							;
ends						;	

	public	_CallInterrupt

	.code

_CallInterrupt	proc

	push	ds				; Guardar registros que deben conservarse
	push	di				; y apuntar BP a base del stack frame.
	push	si				;
	push	bp				;
	mov		bp,sp			;

if FAR_DATA
	lds		bx,[bp].regs	; DS:BX apunta a estructura de registros.
else
	mov		bx,[bp].regs	; DS:BX apunta a estructura de registros.
endif

	push	[bx].i_flags	; Cargar todos los registros salvo DS, BX y
	popf					; BP con los valores de la estructura.
	mov		si,[bx].i_si	;
	mov		di,[bx].i_di	;
	mov		es,[bx].i_es	;
	mov		dx,[bx].i_dx	;
	mov		cx,[bx].i_cx	;
	mov		ax,[bx].i_ax	;

	push	[bx].i_bp		; Guardar valor de BP de la estructura.

	push	[bx].i_bx		; Cargar DS y BX con los valores de la 
	push	[bx].i_ds		; estructura.
	pop		ds				;
	pop		bx				;

	push	bp				; Guardar base de stack frame.

	pushf					; Armar stack para que la instruccion retf
	push	seg return		; ejecute handler con direccion de retorno
	push	offset return	; return.
	push	[bp].handler	; (Este es un push de 4 bytes).
	mov		bp,[bp-2]		; Cargar BP con valor de la estructura.
	retf					; Ejecutar handler (retorna a return).
return:						;
	push	bp				; Copiar el BP devuelto por handler
	mov		bp,sp			; al valor de BP de la estructura
	pop		[bp+4]			; previamente guardado en el stack.

	pop		bp				; Recuperar base del stack frame.

	push	bx				; Guardar DS y BX devueltos por handler.
	push	ds				;

if FAR_DATA
	lds		bx,[bp].regs	; DS:BX apunta a estructura de registros.
else
	mov		bx, ss
	mov		ds, bx
	mov		bx,[bp].regs	; DS:BX apunta a estructura de registros.
endif

	pop		[bx].i_ds		; Recuperar DS y BX devueltos por handler
	pop		[bx].i_bx		; sobre la estructura de registros.

	pop		[bx].i_bp		; Idem BP devuelto por handler.

	pushf					; Copiar los restantes registros, con
	pop		[bx].i_flags	; los valores devueltos por handler,
	mov		[bx].i_si,si	; sobre la estructura de registros.
	mov		[bx].i_di,di	;
	mov		[bx].i_es,es	;
	mov		[bx].i_dx,dx	;
	mov		[bx].i_cx,cx	;
	mov		[bx].i_ax,ax	;

	pop		bp				; Recuperar registros originalmente
	pop		si				; guardados.
	pop		di				;
	pop		ds				;

	ret						; Uf!

_CallInterrupt	endp

	end
