    AREA TopLevelSwi, CODE, READONLY 
		
	IMPORT 	   __SWI_2
	
    EXPORT     SWI_HandlerNuestro

SWI_HandlerNuestro
    ; Handler de SWIs que ejecuta SWIs de lectura y escritura en el CPSR y del temporizador

    STMFD      sp!,{r4-r12,lr}        ; Store registers.

    LDR        r0,[lr,#-4]            ; Calculate address of SWI instruction and load it into r0.

    BIC        r0,r0,#0xff000000      ; Mask off top 8 bits of instruction to give SWI number.
	
	cmp r0, #2
	beq SWInum2

	cmp r0, #0xFF
	beq SWInum0xFF
	
	cmp r0, #0xFE
	beq SWInum0xFE
	
	cmp r0, #0xFD
	beq SWInum0xFD

	cmp r0, #0xFC
	beq SWInum0xFC
	
	b EndofSWI



SWIJumpTable

	DCD    SWInum2         ; SWI de temporizador

	DCD	   SWInum0xFF      ; Activar IRQs

	DCD	   SWInum0xFE      ; Desactivar IRQs

	DCD	   SWInum0xFD      ; Activar FIQs

	DCD	   SWInum0xFC      ; Desactivar FIQs


SWInum2
	
	BL __SWI_2
	
    ; En r0 estara el valor del tmeporizador
	b EndofSWI
	
SWInum0xFF
	;BL __SWInum0xFF

	; Activar IRQs
	mrs r0, spsr 
	and r0, r0, #0xFFFFFF7F ; Se ponen todos los bits excepto I a 1, si lo estaban
	MSR SPSR_cxsf, r0 ; Modifica el SPSR, para que al cambiar de modo se vuelque en CPSR

    B    EndofSWI

SWInum0xFE
	;BL __SWInum0xFE

	mrs r1, spsr
	and r1, r1, #0x80
	lsr r1, r1, #7

	; Desactivar IRQs
	mrs r0, spsr 
	orr r0, r0, #0x80 ; Se pone el bit de I a 1
	MSR SPSR_cxsf, r0 ; Modificam el SPSR, para que al cambiar de modo se vuelque en CPSR

	mov r0, r1

    B    EndofSWI

SWInum0xFD
	;BL __SWInum0xFD

	; Activar FIQs
	mrs r0, spsr 
	and r0, r0, #0xFFFFFFBF ; Se ponen todos los bits excepto F a 1, si lo estaban
	MSR SPSR_cxsf, r0 ; Modifica el SPSR, para que al cambiar de modo se vuelque en CPSR

	B    EndofSWI

SWInum0xFC
	;BL __SWInum0xFC

	mrs r1, spsr
	and r1, r1, #0x40
	lsr r1, r1, #6

	; Desactivar FIQs
	mrs r0, spsr 
	orr r0, r0, #0x40 ; Se pone el bit de F a 1
	MSR SPSR_cxsf, r0 ; Modificam el SPSR, para que al cambiar de modo se vuelque en CPSR

	mov r0, r1
	
	B    EndofSWI

EndofSWI

    ; Restaura el contexto

    LDMFD        sp!, {r4-r12,pc}^
    END                               
