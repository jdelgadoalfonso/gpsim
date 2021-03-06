
   ;;  18F452 tests
   ;;
   ;; This regression test exercises the 18f452's I/O ports.
   ;; 
   ;;  Tests performed:
   ;;
   ;;     


	list    p=18f452                ; list directive to define processor
	include <p18f452.inc>           ; processor specific variable definitions
        include <coff.inc>              ; Grab some useful macros

;----------------------------------------------------------------------
;----------------------------------------------------------------------
GPR_DATA                UDATA
failures        RES     1


;----------------------------------------------------------------------
;   ******************* MAIN CODE START LOCATION  ******************
;----------------------------------------------------------------------
MAIN    CODE

	CLRF	failures	;Assume success

	CLRF	BSR

	MOVLW	0xff

	CLRF	TRISA		;Port A is an output
	MOVWF	TRISB		;Port B is an input

	MOVLW	0x0f

a_to_b_loop:
	MOVWF	LATA		;Port A and Port B are externally
	XORWF	PORTB,W		;connected. So we should see the
	bnz	FAILED		;same thing on each port.

	DECFSZ	PORTB,W
	 goto	a_to_b_loop

   ; Now let's write from PORTB to PORTA.
   ; With the configuration bit setting we have, all of PORTA I/O lines
   ; should be able to serve as inputs

	COMF	TRISA,F		;Port A is now an input port
	COMF	TRISB,F		;Port B is now an output port

	CLRW

b_to_a_loop:
	MOVWF	PORTB		;Port A and Port B are externally
	XORWF	PORTA,W		;connected. So we should see the
	andlw   0x1f
	bnz	FAILED		;same thing on each port.

	DECFSZ	PORTB,W
	 bra	b_to_a_loop

  .assert  "\"*** PASSED 18F452 port test\""
	bra	$

FAILED:
  .assert  "\"*** FAILED 18F452 port test\""
	bra	$

  end
