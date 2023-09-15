    AREA datos, DATA, READWRITE

NUM_FILAS       EQU     9
NUM_COLUMNAS    EQU     16
init_region     DCB     0, 0, 0, 3, 3, 3, 6, 6, 6 ; BYTES

	AREA codigo, CODE, READONLY
	PRESERVE8 {TRUE}
	
	ENTRY

	EXPORT candidatos_actualizar_arm
candidatos_actualizar_arm


; Registros para los dos primeros bucles
; Parámetros:
; r0: direccion de cuadricula
; Registros propios:
; r1: @cuadricula[NUM_FILAS][NUM_FILAS]
; r2: @cuadricula[0][NUM_FILAS]
; r3: NUM_COLUMNAS
; r4: @cuadricula[i][j]
; r5: *cuadricula[i][j]
; r6: celdas_vacías
; r7: BIT_CANDIDATOS + valor - 1 
; -------------------------------------
    ; Prólogo
    mov IP, SP                              ; salvaguardamos SP en IP
    stmdb SP!, {r4-r10, FP, IP, LR, PC}     ; salvaguardamos los registros a modificar incluyendo PC

   ; Inicialización
    mov r3, #NUM_COLUMNAS            ; r3 <- NUM_COLUMNAS
    mov r4,  r0                      ; r4 <- @cuadricula[0][0]
    add r1, r4, #0x120               ; r1 <- @cuadricula[NUM_FILAS][NUM_FILAS]
    add r2, r4, #18                  ; r2 <- @cuadricula[0][NUM_FILAS]
    
    ; Las comparaciones para salir del bucle se realizarán por desplazamientos:
    ; -> si la dirección efectiva es la misma que la del final de la cuadrícula se saldrá del bucle
    ; -> si la dirección efectiva es la misma que la de el final de la fila se avanzará a la primera columna de la siguiente fila 
    
    ; Primer doble bucle
buc1 
    ldrh r5, [r4]                    ; cargamos *celdaptr
    and r5, #0x7F                    ; r5 <- *celdaptr & 0x007F
    cmp r4, r2                       ; @cuadricula[i][j] == @cuadricula[i][NUM_FILAS]?	
    strhlt r5, [r4], #2              ; si es menor, almacenamos y con postindexado j++  
    blt buc1                         ; 
    strh r5, [r4], #16               ; si no, escribimos *celdaptr y con postindexado i++ 
    add r2, r2, #32                  ; r2 <- @cuadricula[i+1][NUM_FILAS]
    cmp r4, r1                       ; @cuadricula[i][j] == @cuadricula[NUM_FILAS][NUM_FILAS]?
    blt buc1  
    
    ; Segundo doble bucle, Inicialización
    mov r1, #0                       ; r1 <- i = 0
    mov r2, #0						 ; r2 <- j = 0
    mov r6, #0                       ; r6 <- celdas_vacias = 0
    mov r4, r0                       ; r4 <- @cuadricula[0][0]
    ldr r11, =init_region            ; r11 <- @init_region 

buc2
    ldrh r5, [r4]                    ; cargamos *@cuadricula[i][j]
    and r5, r5, #0x000F              ; r5 <- *celdaptr & 0x007F;
    add r7, r5, #6                   ; r7 <- BIT_CANDIDATOS + valor - 1 
    mov r8 , #1                      ; r8 <- 1
    lsl r7, r8, r7                   ; ahora r7 tiene el bit en la posicion del valor
    
    cmp r5, #0                       ; valor == 0?
    addeq r6, r6, #1                 ; if(celda_leer_valor(cuadricula[i][j])==0) celdas_vacias++
    beq vacio                        ; y no saltamos a candidatos_propagar

;-------------------------------------------------------
; candidatos_propagar_arm
;-------------------------------------------------------
   ; Registros para los dos primeros bucles
    ; Parámetros:
    ; r0: direccion de cuadricula
    ; Registros propios:
    ; r1: @cuadricula[NUM_FILAS][NUM_FILAS]
    ; r2: @cuadricula[0][NUM_FILAS]
    ; r3: NUM_COLUMNAS
    ; r4: @cuadricula[i][j]
    ; r5: *cuadricula[i][j]
    ; r6: celdas_vacías
    ; r7: BIT_CANDIDATOS + valor - 1 
    ; r8: @cuadricula[fila][NUM_FILAS]
    ; r9:  @cuadricula[fila][0]
    ; r10: @cuadricula[0][columna]
    ; r11: @init_region

    add r9, r0, r1, LSL #5          ; r9 <- @cuadricula[fila][0]
    add r10, r0, r2, LSL #1         ; r10 <- @cuadricula[0][columna]
    add r8, r9, #18                 ; r8 <- @cuadricula[fila][NUM_FILAS]
    
    ; Se ha unido for1 y for2 en un bucle único, utilizando dos desplazamientos:
    ; si la dirección efectiva es igual a la del final de la fila, en ese caso se saldrá del bucle
for1_2
    ldrh r5, [r9]                   ; cargamos *cuadricula[fila][j]
    orr r5, r5, r7                  ; aplicamos la máscara con un or logico
    strh r5, [r9], #2               ; sobreescribimos *celdaptr y con postindexado j++
    
    ldrh r5, [r10]                  ; cargamos *cuadricula[j][columna]
    orr r5, r5, r7                  ; aplicamos la máscara con un or logico
    strh r5, [r10], #32             ; sobreescribimos *celdaptr y con postindexado i++, j=0
    
    cmp r9, r8                      ; @cuadricula[fila][j] == @cuadricula[fila][NUM_FILAS]?
    blt for1_2                      ; seguir en for1_2 si es menor

    ; Último bucle (propagar en región)
    ; Parámetros:
    ; r0: direccion de cuadricula
    ; Registros propios:
    ; r1: @cuadricula[NUM_FILAS][NUM_FILAS]
    ; r2: @cuadricula[0][NUM_FILAS]
    ; r3: NUM_COLUMNAS
    ; r4: @cuadricula[i][j]
    ; r5: *cuadricula[init_i][k]
    ; r6: celdas_vacías
    ; r7: BIT_CANDIDATOS + valor - 1 
    ; r8: init_i
    ; r9: @cuadricula[fin_i][fin_j]
    ; r10: @cuadricula[init_i][init_j]
    ; r11: @init_region
    ldrb r8, [r11, r1]              ; init_i
    ldrb r10, [r11, r2]             ; init_j, k
    
    mla r9, r8, r3, r10             ; scratch <- init_i*NUM_COLUMNAS + init_j
    add r10, r0, r9, LSL #1         ; r10 <- @cuadricula[init_i][init_j]
    add r9, r10, #96                ; r9 <- @cuadricula[fin_i][fin_j] 
    ; (96 = (2+2+28)*3), @ del final de la región

    ; Se ha desenrrollado parcialmente el bucle de propagar en la región para evitar 
    ; recalcular los índices con la tabla de look-up, de tal forma que init_j es un 
    ; desplazamiento dentro del bucle.
buc_region
    ldrh r5, [r10]                  ; cargamos *cuadricula[init_i][k]
    orr r5, r5, r7                  ; contenido a escribir en la celda <- máscara con un or logico
    strh r5, [r10], #2              ; almacenamos la celda modificada, y con postindexado k++

    ldrh r5, [r10]                  ; cargamos *cuadricula[init_i][k]
    orr r5, r5, r7                  ; contenido a escribir en la celda <- máscara con un or logico
    strh r5, [r10], #2              ; almacenamos la celda modificada, y con postindexado k++
    
    ldrh r5, [r10]                  ; cargamos *cuadricula[init_i][k]
    orr r5, r5, r7                  ; contenido a escribir en la celda <- máscara con un or logico
    strh r5, [r10], #28             ; almacenamos la celda modificada, con postindex init_i++ y k=init_j
    cmp r10 , r9                    ; @cuadricula[init_i][fin_j] == @cuadricula[fin_i][fin_j]? 
    blt buc_region 

;-------------------------------------------------------
; fin de candidatos_propagar_arm                        
;-------------------------------------------------------

vacio  
    add r2, r2, #1                  ; j++
    cmp r2, #NUM_FILAS              ; j == NUM_FILAS?
    addlt r4, r4, #2                ; si es menor, r4 <- @cuadricula[i][j+1]
    blt buc2                        ; y seguimos iterando
    
    add r4, r4, #16                 ; si no, r4 <- @cuadricula[i+1][0]
    mov r2, #0                      ; j = 0,
    add r1, r1, #1                  ; i++
    cmp r1, #NUM_FILAS              ; i == NUM_FILAS? 
    blt buc2                        ; y seguimos iterando si no

fin_i2
    mov r0, r6                      ; r0 <- celdas_vacias para devolverlo como resultado
    
    ; Epílogo
    sub FP, IP, #4 				    ; FP está un registro por debajo de donde estaba SP originalmente
	ldmdb FP, {r4-r10, FP, SP, PC}	; carga desde FP, que es el SP original antes de apilar -4, descendiendo antes, los registros, donde LR ahora va a PC
    END
