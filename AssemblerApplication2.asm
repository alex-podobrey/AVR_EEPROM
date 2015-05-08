/*
 * AssemblerApplication2.asm
 *
 *  Created: 13.03.2015 0:22:13
 *   Author: ???????
 */ 
.equ F_CPU = 16000000
.equ ATMega = 1

.equ TASK_COUNT = 1

.equ TASKID_EEPROM = 0

.equ LED_PORT = PORTB
.equ LED_PIN = 5

.equ KEY_PINS = 0b00000001
.equ KEY_PIN = 0
.equ KEY_PORT = PINB
.equ KEY_INVERSE = 1

;=============================================== Макросы ===========================================

.include "macros.inc"

;============================================ CODE Segment ========================================

.CSEG

.include "IntVectorsTable.inc"

.include "TaskQueue.inc"

Init:
;========================================== настройка периферии ====================================

	; DDR = 0 - вход, 1 - выход
	; PORT - значение на выходе при DDR = 1 или наличие подтяжки при DDR = 0 (0 - режим Hi-Z, 1 - подтяжка)

	sbi DDRB, LED_PIN		; установка режима порта на выход для мигания светодиодом

;======================================  инициализация переменных ===================================


;======================================  инициализация задач ========================================

	_AddTask TASKID_EEPROM, TaskEEPROM, (1<<TASK_STATE_ACTIVE_BIT)|(0<<TASK_STATE_CONT_BIT), 0

	ret

;============================================= Task routines =======================================

.equ EEPROM_SIZE_GREATER_256 = 1
.include "eeprom.inc"

TaskEEPROM:

	ldi R16, 1
	ldi R17, 0xff
	rcall WriteEEPROM

	ldi R16, 0
	ldi R17, 0x23
	rcall WriteEEPROM

	ldi R16, 1
	ldi R17, 0
	rcall WriteEEPROM

	ldi R16, 2
	ldi R17, 0xFF
	rcall WriteEEPROM

	ldi R16, 0
	call ReadEEPROM
	ldi R20, 1
	cpi R17, 0x23
	brne TaskEEPROM_Error

	ldi R16, 1
	call ReadEEPROM
	ldi R20, 2
	cpi R17, 0
	brne TaskEEPROM_Error

	ldi R16, 2
	call ReadEEPROM
	ldi R20, 3
	cpi R17, 0xff
	brne TaskEEPROM_Error
	///////////////////

	ldi R16, 0
	ldi R17, 0x34
	rcall WriteEEPROM

	ldi R16, 1
	ldi R17, 7
	rcall WriteEEPROM

	ldi R16, 2
	ldi R17, 0x10
	rcall WriteEEPROM


	ldi R16, 0
	call ReadEEPROM
	ldi R20, 4
	cpi R17, 0x34
	brne TaskEEPROM_Error

	ldi R16, 1
	call ReadEEPROM
	ldi R20, 5
	cpi R17, 7
	brne TaskEEPROM_Error

	ldi R16, 2
	call ReadEEPROM
	ldi R20, 6
	cpi R17, 0x10
	brne TaskEEPROM_Error

	rjmp TaskEEPROM_OK

TaskEEPROM_Error:
	_DelayTask 1000

T1_Loop:
	sbi LED_PORT, LED_PIN
	_DelayTask 500
	cbi LED_PORT, LED_PIN
	_DelayTask 1000

	dec R20
	brne T1_Loop

TaskEEPROM_Exit:
	ret

TaskEEPROM_OK:
	_DelayTask 1000
	sbi LED_PORT, LED_PIN
	_DelayTask 100
	cbi LED_PORT, LED_PIN
	_DelayTask 200
	sbi LED_PORT, LED_PIN
	_DelayTask 100
	cbi LED_PORT, LED_PIN
	_DelayTask 200
	sbi LED_PORT, LED_PIN
	_DelayTask 100
	cbi LED_PORT, LED_PIN
	rjmp TaskEEPROM_Exit


;============================================ DATA Segment ========================================

.DSEG
