
// ��� ������������� �������� ���������� ���������� ��������� EEPROM_SIZE_GREATER_256
//	.equ EEPROM_SIZE_GREATER_256 = 1 - ��� �� � ����������� EEPROM ����� 256 ����
//	.equ EEPROM_SIZE_GREATER_256 = 0 - ��� �� � ����������� EEPROM ����� ��� ����� 256 ����
// � EEPROM ������������ ��� �������� � ������ ����� ������
// ��� ���������� ��� ����������� �� ��������� � ��� ������������ ������ �� ��� ������������� ��������������
// ������������ ���������� ������������ �������� ���(256, (EEPROM_SIZE - 8) div 3))

.ifndef EEPROM_SIZE_GREATER_256
	.error "Not defined const EEPROM_SIZE_GREATER_256"
.endif

.CSEG

//=============================================
; ��������� ������ ������ �������� � EEPROM
; � R16 - ����� ������������ ����������
; � R17 - ������������ ��������
WriteEEPROM:
	push XL
	.if EEPROM_SIZE_GREATER_256 != 0
		push XH
	.endif

	; ��������� ������ �� ������ ���������� �� �������: Addr1 = (X div 4) * 12 + 8; Addr2 = Addr1 + 4; Addr3 = Addr2 + 4
	mov XL, R16
	andi XL, 0b11111100
	.if EEPROM_SIZE_GREATER_256 != 0
		clr XH
		rol XL
		rol XH
		add XL, R16
		sbci XH, 0
		adiw X, 8
	.else
		lsl XL
		add XL, R16
		subi XL, -8
	.endif

	ldi R19, 3					; ���������� ����������� ��������

WriteEEPROM_Loop:
	cli
	sbis EECR, EEPE
	rjmp WriteEEPROM_Ready

	sei
	push R17
	push R19
	_DelayTask 1				; EEPROM �� ������, ������ ����� � 1��
	pop R19
	pop R17

	rjmp WriteEEPROM_Loop

WriteEEPROM_Ready:
	.if EEPROM_SIZE_GREATER_256 != 0
		_outW EEARL, XL, XH
	.else
		_out EEAR, XL
	.endif
	sbi EECR, EERE
	_in R18, EEDR				; ������� ��������� ������� ��������

	eor R18, R17				; ���� ��� ����� ������������� - ������ �� ������
	breq WriteEEPROM_Next

	cbi EECR, EEPM1				; ������������� �� ��������� ����� "�������� + ������"
	cbi EECR, EEPM0

	and R18, R17				; ����������, ���� �� ����, ���������� �� 0 � 1. ���� ���, �� �������� �������� ������ �� ����� - ������ ������
	breq WriteEEPROM_WriteMode
	
	cpi R17, 0xFF				; ���� ������������ �������� ����� 0xFF, �� ���������� ������ ������� ������ ��� ������������� ��������� ������
	brne WriteEEPROM_WriteData

	sbi EECR, EEPM0				; ����� "������ ��������"
	rjmp WriteEEPROM_WriteData

WriteEEPROM_WriteMode:
	sbi EECR, EEPM1				; ����� "������ ������"

WriteEEPROM_WriteData:
	_out EEDR, R17
	sbi EECR, EEMPE
	sbi EECR, EEPE

WriteEEPROM_Next:
	sei

	.if EEPROM_SIZE_GREATER_256 != 0
		adiw X, 4
	.else
		subi XL, -4
	.endif

	dec R19
	brne WriteEEPROM_Loop

	; ��� ���������� ����������� ����� ���������� � ������� ������ 0
	cli
	clr R17
	.if EEPROM_SIZE_GREATER_256 != 0
		_outW EEARL, R17, R17
	.else
		_out EEAR, R17
	.endif
	sei

	.if EEPROM_SIZE_GREATER_256 != 0
		pop XH
	.endif
	pop XL

	ret

//=============================================
; ������� ������ �������� �� EEPROM
; � R16 - ����� ����������� ����������
; ����������:
; � R17 - ��������� ��������
ReadEEPROM:

	push XL
	.if EEPROM_SIZE_GREATER_256 != 0
		push XH
	.endif

	; ��������� ������ �� ������ ���������� �� �������: Addr1 = (X div 4) * 12 + 8; Addr2 = Addr1 + 4; Addr3 = Addr2 + 4
	mov XL, R16
	andi XL, 0b11111100
	.if EEPROM_SIZE_GREATER_256 != 0
		clr XH
		rol XL
		rol XH
		add XL, R16
		sbci XH, 0
		adiw X, 8
	.else
		lsl XL
		add XL, R16
		subi XL, -8
	.endif

	ldi R19, 3				; ���������� ����������� ��������

ReadEEPROM_Loop:
	cli
	sbis EECR, EEPE
	rjmp ReadEEPROM_Ready

	sei
	push R19
	_DelayTask 1				; EEPROM �� ������, ������ ����� � 1��
	pop R19

	rjmp ReadEEPROM_Loop

ReadEEPROM_Ready:
	.if EEPROM_SIZE_GREATER_256 != 0
		_outW EEARL, XL, XH
	.else
		_out EEAR, XL
	.endif
	sbi EECR, EERE
	_in R16, EEDR

	sei
	
	push R16

	.if EEPROM_SIZE_GREATER_256 != 0
		adiw X, 4
	.else
		subi XL, -4
	.endif

	dec R19
	brne ReadEEPROM_Loop

	; ��� ��������� �������� � ��������� R16, R17, R18
	pop R18
	pop R17
	pop R16

	; ���������� ��������� ��������
	cp R16, R17
	brne ReadEEPROM_CheckR18

	cp R16, R18
	breq ReadEEPROM_Exit		; ��� ��� �������� ����������

	; ������ ��� ������ �������� �������� (� R18)
	.if EEPROM_SIZE_GREATER_256 != 0
		sbiw X, 4
	.else
		subi XL, 4
	.endif

	rjmp ReadEEPROM_Repair

ReadEEPROM_CheckR18:
	cp R16, R18
	breq ReadEEPROM_ErrorR17

	; ������ ��� ������ ������� �������� (� R16)
	.if EEPROM_SIZE_GREATER_256 != 0
		sbiw X, 12
	.else
		subi XL, 12
	.endif

	rjmp ReadEEPROM_Repair

ReadEEPROM_ErrorR17:
	; ������ ��� ������ ������� �������� (� R17)
	.if EEPROM_SIZE_GREATER_256 != 0
		sbiw X, 8
	.else
		subi XL, 8
	.endif

	mov R17, R16

ReadEEPROM_Repair:				; ���������� ������ ������ - ���������� ���������� ��������
	cli
	sbis EECR, EEPE
	rjmp ReadEEPROM_ReadyForWrite

	sei
	push R17
	_DelayTask 1				; EEPROM �� ������, ������ ����� � 1��
	pop R17

	rjmp ReadEEPROM_Repair

ReadEEPROM_ReadyForWrite:
	.if EEPROM_SIZE_GREATER_256 != 0
		_outW EEARL, XL, XH
	.else
		_out EEAR, XL
	.endif
	_out EEDR, R17

	sbi EECR, EEMPE
	sbi EECR, EEPE
	sei

ReadEEPROM_Exit:
	; ��� ���������� ����������� ����� ���������� � ������� ������ 0
	cli
	clr R16
	.if EEPROM_SIZE_GREATER_256 != 0
		_outW EEARL, R16, R16
	.else
		_out EEAR, R16
	.endif
	sei

	.if EEPROM_SIZE_GREATER_256 != 0
		pop XH
	.endif
	pop XL

	ret

