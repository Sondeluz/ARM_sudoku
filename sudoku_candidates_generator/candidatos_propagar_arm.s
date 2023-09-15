	AREA datos, DATA, READWRITE

NUM_FILAS       EQU     9
NUM_COLUMNAS    EQU     16
init_region     DCB     0, 0, 0, 3, 3, 3, 6, 6, 6 ; BYTES


; DISTRIBUCION DE REGISTROS BUCLES
; r0  <- @cuadricula
; r1  <- fila
; r2  <- columna
; r3  <- valor para la máscara | 1
;--------------------
; r4  <- NUM_FILAS 
; r5  <- NUM_COLUMNAS
; r6  <- bit en la posicion de valor
; -------------------
; r7  <- i | j | init_i
; r8  <- desplazamiento | fin_i
; r9  <- *celdaptr a modificar

	AREA codigo, CODE, READONLY

	ENTRY

	EXPORT candidatos_propagar_arm
candidatos_propagar_arm

    ; Prólogo
	mov IP, SP 							    ; salvaguardamos SP en IP
	stmdb SP!, {r4-r10, FP, IP, LR, PC}     ; salvaguardamos los registros a modificar incl. PC
	
    ; Inicialización
    mov r4, #NUM_FILAS                      ; r4 <- NUM_FILAS
    mov r5, #NUM_COLUMNAS                   ; r5 <- NUM_COLUMNAS
             
	mla r8, r5, r1, r2                      ; desplazamiento <- fila*NUM_COLUMNAS + columna
	lsl r8, r8, #1                          ; desplazamiento con shift para direccionar halfwords
    ldrh r3, [r0, r8]                       ; r3 <- *cuadricula[fila][columna] 
	and r3, r3, #0x000F					    ; r3 <- valor (se aplica una mascara para elegir solo los bits de valor)
	
    mov r6, #7                              ; r6 <- BIT_CANDIDATOS
    add r6, r6, r3                          ; r6 <- BIT_CANDIDATOS + valor
    sub r6, r6, #1                          ; r6 <- BIT_CANDIDATOS + valor - 1                                      
    mov r3 , #1                             ; r3 <- 1
	lsl r6, r3, r6                          ; ahora r6 tiene el bit en la posicion del valor

    mov r7, #0                              ; r7 (j) <- 0      
    mla r8, r1, r5, r7                      ; r8 (desplazamiento) <- fila*NUM_COLUMNAS + j
    lsl r8, r8, #1                          ; desplazamiento con shift para direccionar halfwords
    add r8, r8, r0                          ; desplazamiento <- cuadricula + desplazamiento


for1 
    ldrh r9, [r8]                           ; cargamos *celdaptr
    orr r9, r9, r6					        ; aplicamos la máscara con un or logico
    strh r9, [r8], #2                       ; almacenamos la celda modificada, con postindexado
    add r7, r7, #1                          ; j++
    cmp r7, #NUM_COLUMNAS - 7               ; j == NUM_FILAS?
    blt for1                                ; seguir en for1 si es menor estricto

    mov r7, #0                              ; r7 <- i = 0
    mla r8, r7, r5, r2                      ; r8 <- desplazamiento <- i*NUM_COLUMNAS + columna
    lsl r8, r8, #1                          ; r8 <- shift para direccionar halfwords
    add r8, r8, r0                          ; r8 <- base+desplazamiento 

for2
    ldrh r9, [r8]                           ; cargamos *celdaptr
    orr r9, r9, r6				            ; aplicamos la máscara con un or logico
	strh r9, [r8], #32                      ; y sobreescribimos *celdaptr
    add r7, r7, #1                          ; i++             
    cmp r7, r4                              ; i == NUM_FILAS? 
    blt for2                                ; seguir en for1 si es menor estricto


   
; DISTRIBUCION DE REGISTROS BUCLE ANDIDADOS
; r0  <- @cuadricula
; r1  <- fila
; r2  <- columna
; r3  <- init_region
;--------------------
; r4  <- scratch
; r5  <- NUM_COLUMNAS | base + desplazamiento
; r6  <- bit en la posicion de valor
; -------------------
; r7  <- init_i
; r8  <- fin_i
; r9  <- init_j
; r10 <- fin_j
; r11 <- *celdaptr

     ; inicializacion del doble anidado
    ldr r3, =init_region            ; r3 <- @init_region 
    ldrb r7, [r3, r1]               ; r7 <- init_i, con r1 (fila) como offset
    ldrb r9, [r3, r2]               ; r9 <- init_j, con r2 (columna) como offset
    MLA r4, r7, r5, r9              ; scratch <- init_i(fila)*NUM_COLUMNAS + init_j
    lsl r4, r4, #1                  ; scratch con shift para direccionar halfwords
    add r5, r4, r0                  ; r5 <- base+desplazamiento
    add r8, r7, #3                  ; r8 <- fin_i
    add r10, r9, #3                 ; r10 <- fin_j
    mov r3, r9                      ; guardamos init_j en un registro scratch para evitar cargarlo de nuevo
    
    ; evitamos la comparación inicial, sabemos que va a haber 3x3 iteraciones                       
ini_i
    ldrh r11, [r5]                   ; cargamos *celdaptr
    orr r11, r11, r6                 ; contenido a escribir en la celda <- máscara con un or logico
    
    add r9, r9, #1                   ; init_j++
    cmp r9, r10                      ; init_j == fin_j?
	strhlt r11, [r5], #2             ; si es menor, almacenamos la celda modificada y sigue avanzando en la región con postindexado
    blt ini_i
    strh  r11, [r5], #28             ; si no, almacenamos la celda modificada y saltamos a la siguiente fila (pero 2 casillas antes para acabar en la 1ra posicion de la fila de la región) con postindexado, para evitar así calcular la nueva base+desplazamiento
    mov r9, r3                       ; r9 <- init_j
    add r7, r7, #1                   ; init_i++
    cmp r7, r8                       ; init_i == fin_i? 
    blt ini_i                        ; si no, sigue iterando
fin_i
    ; Epílogo
    sub FP, IP, #4 				    ; FP está un registro por debajo de donde estaba SP originalmente
	ldmdb FP, {r4-r10, FP, SP, PC}	; carga desde FP, que es el SP original antes de apilar -4, descendiendo antes, los registros, donde LR ahora va a PC
	END