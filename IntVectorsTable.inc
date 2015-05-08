.CSEG

.ORG 0
	jmp Start

.ORG INT0addr	; External Interrupt Request 0
	reti

.ORG INT1addr	; External Interrupt Request 1
	reti

.ORG PCI0addr	; Pin Change Interrupt Request 0
	reti

.ORG PCI1addr	; Pin Change Interrupt Request 1
	reti

.ORG PCI2addr	; Pin Change Interrupt Request 2
	reti

.ORG WDTaddr	; Watchdog Time-out Interrupt
	reti

.ORG OC2Aaddr	; Timer/Counter2 Compare Match A
	jmp Timer_ISR

.ORG OC2Baddr	; Timer/Counter2 Compare Match B
	reti

.ORG OVF2addr	; Timer/Counter2 Overflow
	reti

.ORG ICP1addr	; Timer/Counter1 Capture Event
	reti

.ORG OC1Aaddr	; Timer/Counter1 Compare Match A
	reti

.ORG OC1Baddr	; Timer/Counter1 Compare Match B
	reti

.ORG OVF1addr	; Timer/Counter1 Overflow
	reti

.ORG OC0Aaddr	; TimerCounter0 Compare Match A
	reti

.ORG OC0Baddr	; TimerCounter0 Compare Match B
	reti

.ORG OVF0addr	; Timer/Couner0 Overflow
	reti

.ORG SPIaddr	; SPI Serial Transfer Complete
	reti

.ORG URXCaddr	; USART Rx Complete
	reti

.ORG UDREaddr	; USART, Data Register Empty
	reti

.ORG UTXCaddr	; USART Tx Complete
	reti

.ORG ADCCaddr	; ADC Conversion Complete
	reti

.ORG ERDYaddr	; EEPROM Ready
	reti

.ORG ACIaddr	; Analog Comparator
	reti

.ORG TWIaddr	; Two-wire Serial Interface
	reti

.ORG SPMRaddr	; Store Program Memory Read
	reti

