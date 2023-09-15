#include "stdint.h"
#include <lpc210x.h>

#include "temporizador.h"
#include "cola.h"
#include "eventos.h"

#define PERIODO 0xFFFFFFFF // Valor más alto posible, para generar el mínimo número de interrupciones (una)

// Contador de veces que timer1 ha pasado por 0xFFFFFFFF microsegundos
volatile static unsigned int contador_timer_1 = 0;

void aumentar_contador_ISR(void) __irq;

// Pre: --
// Post: Configura el timer1 como un temporizador de un microsegundo de precisión, que genera el mínimo número de interrupciones
// para dicha precisión.
void temporizador_iniciar(void) {
		// Resetea el contador
    T1TCR = 2; // Activa el bit de Reset
    // Se sacará del reset al ejecutar temporizador_empezar()
    
		// Configuración del timer
    T1MR0 = PERIODO; // Una interrupción cada 0xFFFFFFFF ticks
		T1PR = 14; // 1 tick cada 15 ciclos, para hacer que pase a ser 1 tick / microsegundo, al ir el reloj (PCLK) a 15Mhz 
		T1MCR = 3; // Activa los bits de interrupción al llegar al valor de TC (bit 0) y el de reset al llegar también (bit 1)
	
    // Configuración del VIC
		VICVectAddr0 = (unsigned long)aumentar_contador_ISR; // Almacenamos la dirección de la rutina de servicio a ejecutar cada vez que se haga un tick

    // Se escribe el valor 5 en los bits 0 a 4 de VICVectCntl0, para indicar el número de interrupción de timer 1
    // A su vez, ponemos el bit 5 (0x20) a 1 para activar el slot de IRQ en el VIC
    VICVectCntl0 = 0x20 | 5;                   
    
    // Activa la IRQ 5 (la IRQ de timer1 en el VIC)
    VICIntEnable |= 0x00000020;
}

// Pre: --
// Post: Devuelve el número de veces que timer1 ha pasado por 0xFFFFFFFF microsegundos
unsigned int leer_cuenta(void) {
	return contador_timer_1;
}

// Pre: --
// Post: IRQ que aumenta el número de veces que timer1 ha pasado por 0xFFFFFFFF microsegundos
void aumentar_contador_ISR (void) __irq {
    contador_timer_1 += 1;
    T1IR = 1; // Limpia la flag de interrupciones pendientes
    VICVectAddr = 0; // Avisa al VIC de que se ha tratado la IRQ
}

// Pre: --
// Post: Inicia la cuenta del temporizador 1
void temporizador_empezar(void) {
		T1TCR = 1; // Activa el bit Counter Enable, y por tanto activa TC Y PC
}

// Pre: --
// Post: Para la cuenta del temporizador 1, y devuelve su cuenta de tiempo, en microseundos
long temporizador_parar(void) {
    T1TCR = 0; // Desactiva el bit Counter Enable, desactiva TC Y PC
    return clock_gettime();
}


// Pre: --
// Post: IRQ que encola un evento de comprobación del temporizador periódico. Ejecutada por el timer 0.
void encolar_evento(void) {
    cola_guardar_eventos(EVENTO_TEMPORIZADOR_PERIODICO, 0); // Encola un evento de comprobar el temporizador periódico
    T0IR = 1; // Limpia la flag de interrupciones pendientes
    VICVectAddr = 0; // Avisa al VIC de que se ha tratado la IRQ
}

// Pre: --
// Post: Configura el timer0 como un temporizador periódico. El periodo se indica en milisegundos.
void temporizador_periodico(int periodo) { // El periodo a usar en el juego será de 1 milisegundo
    // Resetea el contador
    T0TCR = 2; // Activa el bit de Reset
    // Se pondrá en marcha tras configurarse
    
	  // Configuración del timer
    // Multiplicamos el periodo por 1000, porque sabemos que los ticks van a ser por microsegundo
    T0MR0 = periodo*1000; // 1 tick / periodo (en s) ciclos, que luego pasa a ser cada periodo*15 ciclos con el prescale register
    T0PR = 14; // 1 tick cada 15 ciclos, para hacer que pase a ser 1 tick / microsegundo, al ir el reloj (PCLK) a 15Mhz 
    T0MCR = 3; // Activa los bits de interrupción al llegar al valor de TC (bit 0) y el de reset al llegar también (bit 1)
	
		VICIntSelect |= 0x10; // Se escribe un 1 en el bit 4 para indicar que las interrupciones de timer 0 se traten como FIQs en vez de IRQs
	
    // Configuración del VIC
		VICVectAddr1 = (unsigned long)encolar_evento; // Se almacena la dirección de la rutina de servicio a ejecutar cada vez que se haga un tick

    // Se escribe el valor 4 en los bits 0 a 4 de VICVectCntl0, para indicar el número de interrupción de timer 0
    // A su vez, ponemos el bit 5 (0x20) a 1 para activar el slot de IRQ en el VIC
    VICVectCntl1 = 0x20 | 4;                   
	
    // Se activa la IRQ 4 (la IRQ de timer0 en el VIC)
    VICIntEnable = VICIntEnable | 0x00000010;

    T0TCR = 1; // Activa el bit Counter Enable, y por tanto activa TC Y PC
}


// Pre:  --
// Post: Devuelve el valor del temporizador periódico en milisegundos
long temporizador_leer_periodico(void) {
    return T0TC / 1000;
}

// Pre: --
// Post: Devuelve la cuenta de tiempo del temporizador 1, en microsegundos
uint32_t __swi(2) clock_gettime(void);

// Función SWI que devuelve la cuenta del tiempo de timer1 en microsegundos
uint32_t __SWI_2 (void) { 
   return T1TC + contador_timer_1*PERIODO;
}

// Pre: --
// Post: Alternativa a clock_gettime para su uso en FIQ, ya que puede 
//       dar problemas ejecutar SWI desde FIQ debido a los cambios de modo
uint32_t temporizador_leer_tiempo (void) { 
   return T1TC + contador_timer_1*PERIODO;
}
