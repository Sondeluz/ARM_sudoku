    AREA datos, DATA, READWRITE

NUM_FILAS       EQU     9
NUM_COLUMNAS    EQU     16

	AREA codigo, CODE, READONLY
	PRESERVE8 {TRUE}
	
	ENTRY

	IMPORT candidatos_propagar_c

	EXPORT candidatos_actualizar_arm_c
candidatos_actualizar_arm_c


; parámetros
; r0: direccion de cuadricula

; registros propios
; r1: i
; r2: j
; r3: NUM_FILAS
; r4: NUM_COLUMNAS
; r5: desplazamiento sobre la cuadricula
; r6: *celdaptr, contenido leido de cuadricula 
; r7: celdas_vacias
; r8:
; r9:

    ; Prólogo
	mov IP, SP 							    ; salvaguardamos SP en IP
	stmdb SP!, {r4-r7, FP, IP, LR, PC}      ; salvaguardamos los registros a modificar incl. PC


    ; primer doble bucle

    ; inicialización
    mov r1, #0						 ; i = 0
	mov r2, #0						 ; j = 0
    mov r3, #NUM_FILAS
    mov r4, #NUM_COLUMNAS
    mla r5, r4, r1, r2               ; desplazamiento <- i*NUM_COLUMNAS + j
	lsl r5, r5, #1                   ; hacemos shift a desplazamiento para direccionar halfwords
    add r5, r5, r0                   ; desplazamiento <- desplazamiento + dirección de cuadricula
	
    ; Evitamos hacer la comparación inicial, sabemos que va a entrar al bucle
    ;cmp r1, r3                       ; i == NUM_FILAS?
    ;bge fin_i1

ini_i1

    ldrh r6, [r5]                    ; cargamos *celdaptr
    and r6, #0x7F                    ; r6 <- *celdaptr & 0x007F;
    add r2, r2, #1                   ; j++
    cmp r2, r3                       ; j == NUM_FILAS?	
	strhlt r6, [r5], #2              ; si es menor, almacenamos la celda modificada y seguimos avanzando en la fila con postindexado
    blt ini_i1
    strheq r6, [r5], #16             ; si no, almacenamos la celda modificada y saltamos a la siguiente fila con postindexado, para evitar así calcular la nueva base+desplazamiento
    mov r2, #0                       ; si j = NUM_FILAS -> j = 0  
    add r1, r1, #1                   ; si j = NUM_FILAS -> i++
    cmp r1, r3                       ; i == NUM_FILAS? 
    blt ini_i1                       ; seguimos en el bucle si es menor
fin_i1



    ; segundo doble bucle
    ; inicialización
    mov r1, #0                      ; i=0
    mov r2, #0                      ; j=0
    mov r7, #0                      ; celdas_vacias = 0
    mla r5, r4, r1, r2              ; desplazamiento <- i*NUM_COLUMNAS + j
    lsl r5, r5, #1                  ; hacemos shift a desplazamiento para direccionar halfwords
	add r5, r5, r0                  ; desplazamiento <- desplazamiento + dirección de cuadricula 

    ; comparación inicial de for2
    cmp r1, r3                      ; i == NUM_FILAS?
    bge fin_i2
ini_i2
    ldrh r6, [r5]                   ; cargamos *celdaptr
    and r6, r6, #0x000F				; r6 <- valor (se aplica una mascara para elegir solo los bits de valor)
	
    cmp r6, #0
    addeq r7, r7, #1                ; if (cuadricula[i][j]) == 0) celdas_vacias++
    beq vacio                       ; y no saltamos a candidatos_propagar
    push {r0-r3, IP}                ; preservamos los registros de argumento, porque según el ATPCS la subrutina no tiene por qué conservarlos
    bl candidatos_propagar_c        ; if (cuadricula[i][j]) != 0) candidatos_propagar_c(cuadricula, i, j)
    pop {r0-r3, IP}
    
vacio  
    add r2, r2, #1                  ; j++
    cmp r2, r3                      ; j == NUM_FILAS?
    addlt r5, r5, #2                ; si seguimos en la fila, aumentamos el desplazamiento en 2
    blt ini_i2                      ; y seguimos iterando
    add r5, r5, #16                 ; si pasamos a una nueva fila, aumentamos el desplazamiento en 16
    mov r2, #0                      ; volvemos a j=0
    add r1, r1, #1                  ; i++
    cmp r1, r3                      ; i == NUM_FILAS? 
    blt ini_i2                      ; y seguimos iterando si no
fin_i2

    ; pasamos a r0 celdas_vacias para devolverlo como resultado siguiendo el ATPCS
    mov r0, r7

    ; Epílogo
    sub FP, IP, #4 				    ; FP está un registro por debajo de donde estaba SP originalmente
	ldmdb FP, {r4-r7, FP, SP, PC}	; carga desde FP, que es el SP original antes de apilar -4, descendiendo antes, los registros, donde LR ahora va a PC
    END
