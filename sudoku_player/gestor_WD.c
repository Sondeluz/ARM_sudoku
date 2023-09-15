#include "gestor_WD.h"
#include <LPC210X.H>    // LPC21XX Peripheral Registers
#include <inttypes.h>


// Funciones de desactivar y activar interrupciones IRQ y FIQ escritas en ensamblador, 
// para facilitar la exclusión mútua en la alimentación al watchdog, que no se puede interrumpir,
// frente a interrupciones que puedan cortar la ejecución.
//
// Se encuentran implementadas como SWI, en SWI_Handler.s para sus respectivos IDs.
void __swi(0xFF) enable_isr (void); 
uint32_t __swi(0xFE) disable_isr (void); 
 
void __swi(0xFD) enable_isr_fiq (void); 
uint32_t __swi(0xFC) disable_isr_fiq (void);

// Inicializa el watchdog con un timeout de sec segundos
void WD_init(int sec){
    // Configura la constante de tiempo del watchdog para que sea un segundo * sec
    WDTC = 3750000*sec;
    // Habilita el modo de reset del procesador si hay underflow del temporizador del watchdog, no se puede modificar a partir de ahora
    WDMOD = 0x03;

    // Se alimenta para ponerlo en marcha
    WD_feed();
}

// Alimenta al watchdog
void WD_feed(void){
    uint32_t valorI = disable_isr();
    uint32_t valorF = disable_isr_fiq();
    
    WDFEED = 0xAA; 
    WDFEED = 0x55;

    // Si el bit de IRQ en CPSR estaba antes a 0 (interrupciones activas), 
    // las volvemos a activar.
    // Esto evita causar problemas en IRQs, que de por sí escriben en el bit I 
    // y desactivan las interrupciones el empezar, y podrían encadenarse múltiples
    // al volver a activar las interrupciones.
    if (valorI == 0) enable_isr();
    if (valorF == 0) enable_isr_fiq();
}
