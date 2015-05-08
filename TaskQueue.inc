/* Структура описания задачи
	State			.db	Текущее состояние задачи. Бит TASK_STATE_ACTIVE_BIT = 1 - задача активна, бит TASK_STATE_CONT_BIT = 1 - задача выполняется постояно (после выполнения ее состояние не сбрасывается)
	RoutineAddress	.dw	Адрес процедуры обработки
	Delay			.dw	Период выполнения, если бит TASK_STATE_CONT_BIT установлен
	TimerCounter	.dw Текущее состояние таймера
*/

/* Структура массива задач

	Flags			.db Флаги состояния очереди задач. Бит TASKS_UPDATED_BIT = 1 - задачи обновлены, необходимо еще раз их обработать
	TaskId			.db Номер текущей выполняемой задачи

	Массив структур описаний задач

*/

.equ TASKS_UPDATED_BIT = 0
.equ TASK_STATE_ACTIVE_BIT = 0
.equ TASK_STATE_CONT_BIT = 1
.equ TASK_STATE_DELAYED_BIT = 2

.equ TASK_DATA_SIZE = 7
.equ TASKS_DATA_SIZE = TASK_DATA_SIZE * TASK_COUNT + 2

.equ TIME_COUNTER = 1
.equ TIME_COUNTER_SIZE = 6

.macro _AddTask	; TaskId, Task routine, Task state, Repeat delay

	.if @0 >= TASK_COUNT
		.error "Invalid task number"
	.else
		ldi ZL, Low(TasksData + 2 + (@0 * TASK_DATA_SIZE))
		ldi ZH, High(TasksData + 2 + (@0 * TASK_DATA_SIZE))

		ldi R16, @2
		st Z+, R16

		ldi R16, Low(@1)
		st Z+, R16
		ldi R16, High(@1)
		st Z+, R16

		.if @3 == 0
			clr R16
			st Z+, R16
			st Z+, R16
		.else
			ldi R16, Low(@3)
			st Z+, R16
			ldi R16, High(@3)
			st Z+, R16
			clr R16
		.endif
		st Z+, R16
		st Z, R16
	.endif

.endm

.macro _SetTaskTimer	;TaskId, Timer value

	.if @0 >= TASK_COUNT
		.error "Invalid task number"
	.else
		.if @1 == 0
			clr R16
			sts (TasksData + 2 + (@0 * TASK_DATA_SIZE) + 5), R16
			sts (TasksData + 2 + (@0 * TASK_DATA_SIZE) + 6), R16
		.else
			ldi R16, Low(@1)
			sts (TasksData + 2 + (@0 * TASK_DATA_SIZE) + 5), R16
			ldi R16, High(@1)
			sts (TasksData + 2 + (@0 * TASK_DATA_SIZE) + 6), R16
		.endif
	.endif

.endm

.macro _ActivateTaskAndSetTimer	;TaskId, Timer value

	.if @0 >= TASK_COUNT
		.error "Invalid task number"
	.else
		lds R16, (TasksData + 2 + (@0 * TASK_DATA_SIZE))		; активируем задачу
		sbr R16, 1<<TASK_STATE_ACTIVE_BIT
		sts (TasksData + 2 + (@0 * TASK_DATA_SIZE)), R16

		; устанавливаем таймер
		.if @1 == 0
			clr R16
			sts (TasksData + 2 + (@0 * TASK_DATA_SIZE) + 5), R16
			sts (TasksData + 2 + (@0 * TASK_DATA_SIZE) + 6), R16

			; если таймер = 0 (задача должна выполниться немедленно), то устанавливаем флаг обновления очереди задач, чтобы пересканировать ее заново
			lds R16, TasksData
			sbr R16, 1<<TASKS_UPDATED_BIT
			sts TasksData, R16
		.else
			ldi R16, Low(@1)
			sts (TasksData + 2 + (@0 * TASK_DATA_SIZE) + 5), R16
			ldi R16, High(@1)
			sts (TasksData + 2 + (@0 * TASK_DATA_SIZE) + 6), R16
		.endif
	.endif

.endm

.macro _DelayTask	; Delay

	ldi R16, Low(@0)
	ldi R17, High(@0)
	call Delay

.endm

.macro _ActivateTask	;TaskId

	.if @0 >= TASK_COUNT
		.error "Invalid task number"
	.else
		lds R16, (TasksData + 2 + (@0 * TASK_DATA_SIZE))
		sbr R16, 1<<TASK_STATE_ACTIVE_BIT
		sts (TasksData + 2 + (@0 * TASK_DATA_SIZE)), R16
	.endif

.endm

.macro _DeactivateTask	;TaskId

	.if @0 >= TASK_COUNT
		.error "Invalid task number"
	.else
		lds R16, (TasksData + 2 + (@0 * TASK_DATA_SIZE))
		cbr R16, 1<<TASK_STATE_ACTIVE_BIT
		sts (TasksData + 2 + (@0 * TASK_DATA_SIZE)), R16
	.endif

.endm

.DSEG
	TasksData: .byte TASKS_DATA_SIZE
	.ifdef TIME_COUNTER
		.if TIME_COUNTER != 0
			TimeCounter: .byte TIME_COUNTER_SIZE
		.endif
	.endif

.CSEG
.ORG INT_VECTORS_SIZE

Start:
	_InitStack

;============================================= настройка периферии =======================================

	; настройка таймера 2
	; режим CTC, делитель 64, предел счета на 250 (на частоте 16 МГц), период срабатывания 1 мс
	ldi R16, (0<<COM2A1)|(0<<COM2A0)|(0<<COM2B1)|(0<<COM2B0)|(1<<WGM21)|(0<<WGM20)
	_out TCCR2A, R16
	ldi R16, (0<<WGM22)|(1<<CS22)|(0<<CS21)|(0<<CS20)
	_out TCCR2B, R16
	clr R16
	_out TCNT2, R16
	ldi R16, (F_CPU / 1000 / 64) - 1
	_out OCR2A, R16
	ldi R16, (1<<OCIE2A)|(0<<OCIE2B)|(0<<TOIE2)
	_out TIMSK2, R16

	; Инициализация сторожевого таймера
	; режим сброса по таймеру через 64 мс
	wdr
	_in R16, WDTCSR
	ori R16, (1<<WDCE)|(0<<WDIE)|(1<<WDE)
	_out WDTCSR, R16
	ldi R16, (0<<WDIE)|(1<<WDE)|(0<<WDP3)|(0<<WDP2)|(1<<WDP1)|(0<<WDP0)
	_out WDTCSR, R16

	; настройка режима энергосбережения
	; режим Extended Standby
	ldi R16, (1<<SM2)|(1<<SM1)|(1<<SM0)|(1<<SE)
	_out SMCR, R16

;=====================================  инициализация переменных ===================================

	_ClearMemory TasksData, TASKS_DATA_SIZE

	.ifdef TIME_COUNTER
		.if TIME_COUNTER != 0
			_ClearMemory TimeCounter, TIME_COUNTER_SIZE
		.endif
	.endif

	call Init

	sei

;=====================================  главный цикл программы =====================================

MainLoop:
	wdr

	ldi R20, TASK_COUNT		; количество задач
	tst R20
	breq MainLoop

	ldi YL, Low(TasksData + 2)
	ldi YH, High(TasksData + 2)

TaskLoop:
	ld R18, Y							; проверка состояния задачи
	ldi R19, (1<<TASK_STATE_ACTIVE_BIT)|(1<<TASK_STATE_DELAYED_BIT)
	and R19, R18						; задача неактивна и не отложена - переходим к следующей
	breq NextTask

	cli
	ldd R16, Y + 5						; проверка таймера задачи. Больше нуля - переход к следующей задаче
	ldd R17, Y + 6
	sei
	or R16, R17
	brne NextTask
	
	ldi R16, TASK_COUNT					; сохраним текущий TaskId
	sub R16, R20
	sts TasksData + 1, R16

	sbrc R18, TASK_STATE_CONT_BIT		; Проверка на непрерывность использования задачи
	rjmp RefreshTaskTimer				; если задача повторяемая, то восстанавливаем значение таймера

	cbr R18, 1<<TASK_STATE_ACTIVE_BIT	; иначе делаем задачу неактивной
	st Y, R18
	rjmp CheckDelayedTask

RefreshTaskTimer:
	cli
	ldd R16, Y + 3						; восстанавливаем значение таймера по умолчанию
	ldd R17, Y + 4
	std Y + 5, R16
	std Y + 6, R17
	sei
	sbr R18, 1<<TASK_STATE_ACTIVE_BIT	; восстанавливаем бит активности задачи
	st Y, R18

CheckDelayedTask:
	sbrc R18, TASK_STATE_DELAYED_BIT	; Проверка на непрерывность использования задачи
	rjmp EndDelay						; Если установлен бит TASK_STATE_DELAYED_BIT - выходим из главного цикла

CallTask:
	ldd ZL, Y + 1
	ldd ZH, Y + 2

	push R20
	push YL
	push YH
	icall								; вызов обрабочика задачи
	pop YH
	pop YL
	pop R20

NextTask:
	wdr

	adiw Y, TASK_DATA_SIZE

	dec R20
	brne TaskLoop

	lds R16, TasksData		; если в процессе обработки очереди установлен бит необходимости обновления, то пересканирем очередь еще раз (засыпать  не будем)
	cbr R16, 1<<TASKS_UPDATED_BIT
	sts TasksData, R16
	sbrs R16, TASKS_UPDATED_BIT

	sleep					; очередь обработана, спим
	rjmp MainLoop

Delay:
	; R16:R17 - значение задержки
	push R20
	push R21
	push R22
	push R23
	push R24
	push R25
	push XL
	push XH
	push YL
	push YH
	push ZL
	push ZH

	ldi YL, Low(TasksData + 1)
	ldi YH, High(TasksData + 1)
	ld R18, Y+			; в R18 - номер текущей задачи

	; получим смещение блока данных текущей задачи (номер задачи умножаем на 7)
	.ifdef ATMega
		ldi R19, TASK_DATA_SIZE
		mul R18, R19
		add YL, R0
		adc YH, R1
	.else					; в ATTiny нет операции умножения, реализуем ее через сдвиги и сложения
		.if TASK_DATA_SIZE != 7
			.error "Invalid logic. Task data size must be equal 7. Rewrite code."
		.endif
		clr R19
		mov R20, R18
		mov R21, R19
	 
		clc
		rol R20
		rol R21
		add R18, R20
		adc R19, R21

		rol R20
		rol R21
		add R18, R20
		add R19, R21

		add YL, R18
		adc YH, R19
	.endif

	cli
	ld R18, Y		; установим бит TASK_STATE_DELAYED_BIT в байте состояния задачи
	sbr R18, 1<<TASK_STATE_DELAYED_BIT
	st Y, R18

	std Y + 5, R16	; запишем в таймер значение задержки
	std Y + 6, R17
	sei

	push YL
	push YH
	rjmp MainLoop

EndDelay:
	pop YH
	pop YL

	ld R18, Y		; сбросим бит TASK_STATE_DELAYED_BIT в байте состояния задачи
	cbr R18, 1<<TASK_STATE_DELAYED_BIT
	st Y, R18

	pop ZH
	pop ZL
	pop YH
	pop YL
	pop XH
	pop XL
	pop R25
	pop R24
	pop R23
	pop R22
	pop R21
	pop R20

	ret

;============================================= ISR routines ========================================

Timer_ISR:

	push R20

	in R20, SREG
	push R20
	push XL
	push XH
	push YL
	push YH

	.ifdef TIME_COUNTER
		.if TIME_COUNTER != 0
			; Увеличиваем системный счетчик
			ldi XL, Low(TimeCounter + TIME_COUNTER_SIZE)
			ldi XH, High(TimeCounter + TIME_COUNTER_SIZE)

			ldi R20, TIME_COUNTER_SIZE
			clc

		Timer_ISR_TimeCounter_NextByte:
			ld R16, -X
			sbci R16, -1			; сложения с константой нет, поэтому через вычитание. Значение флага C при этом инвертировано
			st X, R16

			brcs Timer_ISR_TimeCounter_Exit		; в текущем байте нет переполнения - дальше продолжать нет смысла

			dec R20
			brne Timer_ISR_TimeCounter_NextByte

		Timer_ISR_TimeCounter_Exit:
		.endif
	.endif
	
	ldi R20, TASK_COUNT		; количество задач
	tst R20
	breq TimerISR_NoTasks

	ldi YL, Low(TasksData + 2)
	ldi YH, High(TasksData + 2)

TimerISR_Loop:
	ld XL, Y				; проверка состояния задачи - если у нее не установлены биты активности или отложенности - пропускаем
	andi XL, (1<<TASK_STATE_ACTIVE_BIT)|(1<<TASK_STATE_DELAYED_BIT)
	breq TimerISR_NextTimer

	ldd XL, Y + 5
	ldd XH, Y + 6
	sbiw X, 1
	brcs TimerISR_NextTimer	; таймер уже равен нулю - пропускаем

	std Y + 5, XL			; записываем новое состояние таймера
	std Y + 6, XH

TimerISR_NextTimer:
	adiw Y, TASK_DATA_SIZE	; переход к следующей задаче

	dec R20
	brne TimerISR_Loop

TimerISR_NoTasks:
	pop YH
	pop YL
	pop XH
	pop XL

	pop R20
	out SREG, R20

	pop R20

	reti


