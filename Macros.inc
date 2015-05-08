.ifdef MACROS_INC
	.exit
.endif

.equ MACROS = 1

.macro _out	; Port, R

   	.if	@0 < 0x40
      	out	@0, @1         
	.else
      	sts	@0, @1
   	.endif

.endm
	
.macro _in	; R, Port

   	.if	@1 < 0x40
      	in	@0, @1         
	.else
      	lds	@0, @1
	.endif

.endm

.macro _ClearMemory	; Address, Size

	clr R16
	ldi R17, @1
	ldi ZL, Low(@0)
	ldi ZH, High(@0)

Loop:
	st Z+, R16
	dec R17
	brne Loop

.endm

; Инициализация стека
.macro _InitStack

	.if RAMEND >= 256
		ldi R16, Low(RAMEND)
		out SPL, R16
		ldi R16, High(RAMEND)
		out SPH, R16
	.else
		ldi R16, RAMEND
		out SP, R16
	.endif

.endm

.macro _outW ; Port(L), RegL, RegH

	out @0 + 1, @2
	out @0, @1

.endm

.macro _inW	; RegL, RegH, Port(L)

	in @0, @2
	in @1, @2 + 1

.endm
