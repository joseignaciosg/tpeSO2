GLOBAL  _read_msw,_lidt
GLOBAL  _int_08_hand
GLOBAL  _int_09_hand
GLOBAL  _int_80_hand
GLOBAL  _int_80_caller
GLOBAL  _int_79_hand
GLOBAL  _int_79_caller

GLOBAL  _inport
GLOBAL  _export
GLOBAL  _getCPUSpeed
GLOBAL  _mascaraPIC1,_mascaraPIC2,_Cli,_Sti
GLOBAL  _debug
GLOBAL	_yield
GLOBAL	_out
GLOBAL	_in
GLOBAL	_Halt
GLOBAL	_outb
GLOBAL	_outw
GLOBAL	_inb
GLOBAL	_inw

EXTERN  int_08
EXTERN  int_09
EXTERN  int_80
EXTERN  int_79



EXTERN backuper
EXTERN Schedule 
EXTERN ExecuteProcess
EXTERN LoadESP
EXTERN SaveESP
EXTERN GetNextProcess
EXTERN isTimeSlot
EXTERN GetTemporaryESP

SECTION .text


_Cli:
	cli			; limpia flag de interrupciones
	ret

_Sti:
	sti			; habilita interrupciones por flag
	ret

_mascaraPIC1:			; Escribe mascara del PIC 1
	push    ebp
        mov     ebp, esp
        mov     ax, [ss:ebp+8]  ; ax = mascara de 16 bits
        out	21h,al
        pop     ebp
        retn

_mascaraPIC2:			; Escribe mascara del PIC 2
	    push    ebp
        mov     ebp, esp
        mov     ax, [ss:ebp+8]  ; ax = mascara de 16 bits
        out	    0A1h,al
        pop     ebp
        retn

_read_msw:
        smsw    ax		; Obtiene la Machine Status Word
        retn


_lidt:				; Carga el IDTR
        push    ebp             ;defines a new interupt table
        mov     ebp, esp
        push    ebx
        mov     ebx, [ss: ebp + 6] ; ds:bx = puntero a IDTR
	rol	    ebx,16
	lidt    [ds: ebx]          ; carga IDTR
        pop     ebx
        pop     ebp
        retn


_inport:
		push ebp
		mov ebp,esp
		mov dx, [ss:ebp+8]
		in  al,dx
		mov esp,ebp
		pop ebp
		ret

_export:
		push ebp
		mov ebp,esp
		mov dx,[ss:ebp+8]
		mov al,[ss:ebp+12]
		out dx,al
		mov esp, ebp
		pop ebp
		ret

_inb:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]    ; Puerto
		mov		eax, 0          ; Limpio eax
		in byte		al, dx
		pop		ebp
		ret

_outb:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]   	; Puerto
		mov		eax, [ebp+12]  	; Lo que se va a mandar
		out 	dx, al
		pop		ebp
		ret


_inw:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]    ; Puerto
		mov		eax, 0          ; Limpio eax
		in		ax, dx
		pop		ebp
		ret

_outw:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]   	; Puerto
		mov		eax, [ebp+12]  	; Lo que se va a mandar
		out		dx, ax
		pop		ebp
		ret

_getCPUSpeed:
		cli
		push ebp
		mov ebp,esp
		rdtsc
		mov esp,ebp
		pop ebp
		sti
		ret

_yield:
	int 8
	ret

_int_08_hand:				; Handler de INT 8 ( Timer tick)
	cli
	pushad
		call isTimeSlot
		cmp eax,0
		jne processRunning
		mov eax, esp
		push eax
		call SaveESP
		pop eax
		call GetNextProcess
		push eax
		call LoadESP
		pop ebx
		mov esp,eax
	
processRunning:	mov al,20h			; Envio de EOI generico al PIC
		out 20h,al
	popad
	
	sti
	
	iret


_int_09_hand:      ;Handler de INT 09 (IN y OUT)
	cli
      push    ds
      push    es    ; Se salvan los registros
      pusha         ; Carga de DS y ES con el valor del selector

      mov     ax, 10h	; a utilizar.
      mov     ds, ax
      mov     es, ax

      call    int_09
      mov     al,20h	; Envio de EOI generico al PIC
      out     20h,al
      popa
      pop     es
      pop     ds
	sti
      iret



_int_80_caller:
	  push	ebp
   	  mov	ebp,esp
   	  pusha
	  mov   eax, [ebp+8]  ; call
	  mov   ebx, [ebp+12] ; fd
	  mov   ecx, [ebp+16] ; buffer
	  mov   edx, [ebp+20] ; count

	  int   80h

	  popa
	  mov	esp,ebp
	  pop	ebp
	  ret


_int_80_hand:      ;Handler de INT 80
   	  push    ds
      push    es          ; Se salvan los registros
      pusha               ; Carga de DS y ES con el valor del selector
  	  push   edx ;count
  	  push   ecx ;buffer
  	  push   ebx ;fd
  	  push   eax ;call

      call   int_80

	  pop eax
	  pop ebx
	  pop ecx
	  pop edx

      mov   al,20h			; Envio de EOI generico al PIC
      out   20h,al
	  popa
      pop    es
      pop    ds
      iret


_int_79_caller:
	  push	ebp
   	  mov	ebp,esp
   	  pusha
	  mov   eax, [ebp+8]  ; call
	  mov   ebx, [ebp+12] ; pid

	  int   79h

	  popa
	  mov	esp,ebp
	  pop	ebp
	  ret


_int_79_hand:      ;Handler de INT 79
   	  push    ds
      push    es          ; Se salvan los registros
      pusha               ; Carga de DS y ES con el valor del selector
  	  push   ebx ;pid
  	  push   eax ;call

      call   int_79

	  pop eax
	  pop ebx

      mov   al,20h			; Envio de EOI generico al PIC
      out   20h,al
	  popa
      pop    es
      pop    ds
      iret


_Halt:			; Should lock everything?
		hlt			; wait for HPET/PIT
		ret

_in:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]    ; Puerto
		mov		eax, 0          ; Limpio eax
		in		al, dx
		pop		ebp
		ret

_out:
		push	ebp
		mov		ebp, esp		; Stack frame
		mov		edx, [ebp+8]   	; Puerto
		mov		eax, [ebp+12]  	; Lo que se va a mandar
		out		dx, al
		pop		ebp
		ret



; Debug para el BOCHS, detiene la ejecución. Para continuar colocar en el BOCHSDBG: set $eax=0

_debug:
        push    bp
        mov     bp, sp
        push	ax
vuelve:	mov     ax, 1
        cmp	ax, 0
	jne	vuelve
	pop	ax
	pop     bp
        retn
