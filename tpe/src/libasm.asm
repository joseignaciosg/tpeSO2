



_Cli:
	cli						; limpia flag de interrupciones
	ret

_Sti:
	sti						; habilita interrupciones por flag
	ret

_int_08_hand:			; Handler de INT 8 ( Timer tick)
	cli
	pushad
	mov eax, esp
	push eax
	call _SaveESP
	pop eax
	call _GetTemporaryESP
	mov esp, eax
	call _GetNextProcess
	push eax
	call _LoadESP
	pop ebx
	mov esp,eax
	;call _debug;
	popad

	mov al,20h			; Envio de EOI generico al PIC
	out 20h,al

	sti

	iret


