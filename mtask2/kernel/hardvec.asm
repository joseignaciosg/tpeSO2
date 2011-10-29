include model.inc

	extrn		_mt_hw_vector: far
	extrn		_mt_irq_number: word

create_vec		macro i

	public		_mt_hw_vec_&i

_mt_hw_vec_&i	proc
	push		ds
	push		seg _mt_irq_number
	pop			ds
	mov			_mt_irq_number, i
	pop			ds
	jmp			_mt_hw_vector
_mt_hw_vec_&i	endp

	endm

	.code

i = 0
rept 16

	create_vec %i

i = i + 1
endm

	end
