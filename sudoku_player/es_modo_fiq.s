	AREA codigo, CODE, READONLY
	ENTRY
	EXPORT es_modo_fiq
es_modo_fiq
	
	STMFD      sp!, {lr} 

	; Carga CPSR en r0, y le aplica una máscara sobre los bits de modo
	mrs r0, cpsr 
	and r0,r0, #0x1F

	; Compara con el estado correspondiente a FIQ (0x11)
	cmp r0, #0x11

	moveq r0, #1
	movne r0, #0

	LDMFD      sp!, {pc}
	
	END
