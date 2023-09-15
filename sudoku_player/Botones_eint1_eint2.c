#include <LPC210X.H>    // LPC21XX Peripheral Registers
#include "Botones_eint1_eint2.h"
#include "cola.h"
#include "eventos.h"

void eint1_ISR (void) __irq {
	VICIntEnClr = 0x00008000;    // Deshabilitamos eint1, se podrá habilitar más tarde bajo demanda
	
	EXTINT =  EXTINT | 2;        // Limpiamos la flag de interrupciones eint1 presentes
	VICVectAddr = 0;             // Y reseteamos VICVectAddr (marcamos la interrupción como tratada)

	cola_guardar_eventos(CONFIRMAR_COMANDO, 0); // Encolando el evento de interrupción 1 al planificador
}

void eint2_ISR (void) __irq {
	VICIntEnClr = 0x00010000;    // Deshabilitamos eint2, se podrá habilitar más tarde bajo demanda
	
	EXTINT =  EXTINT | 4;        // Limpiamos la flag de interrupciones eint2 presentes    
	VICVectAddr = 0;             // Y reseteamos VICVectAddr (marcamos la interrupción como tratada)

	cola_guardar_eventos(CANCELAR_COMANDO, 0); // Encolando el evento de interrupción 2 al planificador
};

// Devuelve 1 si el botón 1 está siendo presionado, 0 en caso contrario
unsigned int button_nueva_pulsacion_1(){
	return ((EXTINT & 2) >> 1); 		// Devolvemos el valor de la flag EXTINT para eint1. Si sigue activa, entonces sabemos que se sigue presionando el botón.
};

// Devuelve 1 si el botón 2 está siendo presionado, 0 en caso contrario
unsigned int button_nueva_pulsacion_2(){
	return ((EXTINT & 4) >> 2);		// Devolvemos el valor de la flag EXTINT para eint2. Si sigue activa, entonces sabemos que se sigue presionando el botón.
};

// Limpia las interrupciones pendientes para el botón 1
void button_clear_nueva_pulsacion_1(){
	EXTINT =  EXTINT | 2;         // Limpiamos la flag de interrupciones eint1 presentes
	VICVectAddr = 0;  			  // Y reseteamos VICVectAddr (marcamos la interrupción como tratada, si acaso hubiera alguna pendiente)
};

// Limpia las interrupciones pendientes para el botón 2
void button_clear_nueva_pulsacion_2(){
	EXTINT =  EXTINT | 4;        // Limpiamos la flag de interrupciones eint2 presentes    
	VICVectAddr = 0;             // Y reseteamos VICVectAddr (marcamos la interrupción como tratada, si acaso hubiera alguna pendiente)
};

// Inicializa las interrupciones para el botón 1
void eint1_init (void) {
	EXTINT =  EXTINT | 2;        // Limpiamos la flag de interrupciones eint1 presentes   	
	
	// Asignamos la ISR de eint1 al VIC
	VICVectAddr2 = (unsigned long)eint1_ISR;

	// Modificamos el registro PINSEL para configurar eint1 en su pin del GPIO (15), en vez de un pin normal de GPIO
    PINSEL0 = PINSEL0 & 0xCFFFFFFF;     // Aplicamos una máscara para poner los bits 29:28 a 0
	PINSEL0 = PINSEL0 | 0x20000000;		// Habilitamos eint1 (bits 29:28 a 10)
    
    VICVectCntl2 = 0x20 | 15; // Habilitamos el slot del VIC con 0x20 (bit 5), y configuramos el número de interupción a tratar (15: eint1)                  
}

// Inicializa las interrupciones para el botón 2
void eint2_init (void) {
	EXTINT =  EXTINT | 4;        // Limpiamos la flag de interrupciones eint2 presentes	
	
	// Asignamos la ISR de eint2 al VIC
	VICVectAddr3 = (unsigned long)eint2_ISR;

	// Modificamos el registro PINSEL para configurar eint2 en su pin del GPIO (16), en vez de un pin normal de GPIO
    PINSEL0 = PINSEL0 & 0x3FFFFFFF;     // Aplicamos una máscara para poner los bits 21:30 a 0
	PINSEL0 = PINSEL0 | 0x80000000;		// Habilitamos eint2 (bits 31:30 a 10)

    VICVectCntl3 = 0x20 | 16; // Habilitamos el slot del VIC con 0x20 (bit 5), y configuramos el número de interupción a tratar (16: eint2)   
}


void habilitar_interrupciones_1(void) {
	VICIntEnable = VICIntEnable | 0x00008000; 
}

void habilitar_interrupciones_2(void) {
	VICIntEnable = VICIntEnable | 0x00010000;
}
