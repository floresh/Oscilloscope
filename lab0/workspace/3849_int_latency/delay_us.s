; Delay function that accepts an argument in microseconds.
; The delay is tuned to a 50 MHz clock.
    	.thumb
		.text
		.global delay_us
delay_us .asmfunc
; void delay_us(unsigned long us);
; r0 = number of microseconds to delay
		cmp	 r0, #0		; 1 cycle
		beq	 done		; 1
		mov  r1, #12	; 1 // number of iterations in the first inner loop
		nop				; 1
loop	subs r1, #1		; 3 cycles per iteration
		bne	 loop
		nop				; 1
		nop				; 1
		mov	 r1, #15	; 1 // number of iterations in the inner loop
		subs r0, #1		; 1
		bne	 loop		; 2
done	bx	 lr			; 3
    	.endasmfunc