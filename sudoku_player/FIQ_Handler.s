    AREA codigo, CODE, READONLY 
		
    PRESERVE8 {TRUE}

    EXPORT     FIQ_HandlerNuestro

	IMPORT encolar_evento

FIQ_HandlerNuestro
    ; Handler de FIQ, único para todas las FIQ que se declaren

    STMFD        sp!, {r0-r7, lr} ; Apilamos solamente los registros r0-r7, ya que r8 a r12 son privados en FIQ y no tiene sentido preservarlos

    bl encolar_evento ; salta a la función que hacía como IRQ previamente

    LDMFD        sp!, {r0-r7, lr}

    subs pc, lr, #4 ; Corregimos LR al volver restándole 4, ya que debido al segmentado de ARM hay que corregirlo al saltar a FIQ

    END                               
