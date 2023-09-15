#include "GestorEnergia.h"
#include <LPC210x.H> 
#include "GPIO.h"
#include "eventos.h"
#include "GestorAlarmas.h"

// Pre: --
// Post: Activa el modo idle del procesador
void modo_idle (void) {
    PCON |= 0x01; // Activamos el modo idle (bit 0 a 1)
}

// Pre: --
// Post: Activa el modo power-down del procesador, asegurando que se podrá despertar
// presionando los dos botones (eint1, eint2), y que se restaura la frecuencia original
// del procesador
void modo_power_down (void){	
  	EXTWAKE = 0x06; // Indicamos que se despierte con EINT1 y EINT2 (bits 2 y 1 a 1)
	PCON = 0x02; // Activamos el modo power-down (bit 1 a 1)

	// Al volver, restauramos la frecuencia del procesador inicial, ya que el PLL se ha apagado tambiémn
	Reinicializar_PLL(); // Función externa que apunta a código de Startup.s y reinicializa la frecuencia original del procesador.
}

// Pre: --
// Post: Inicia la alarma de latido en modo idle, que será periódica y solo se desactivará al entrar en
// modo power-down
void iniciar_alarma_latido_modo_idle(void) {
    uint32_t alarma = LATIDO_MODO_IDLE << 24; // Shifteamos 24 bits a la derecha para que el ID_evento 
                             		   // esté en los 8 bits de mayor peso
    alarma = alarma | 0x800000; // El bit 23 indica que la alarma es periodica
    alarma = alarma | 0xFA;     // Las alarmas tienen 250 ms de retardo
    set_alarma(alarma);
}

// Pre: --
// Post: Inicia la alarma de modo power-down, que solicitará al planificador terminar en 15 segundos, a no ser que 
// llame a resetear_alarma_power_down()
void iniciar_alarma_power_down(void) {
	uint32_t alarma = DORMIR_PROCESADOR << 24; // Shifteamos 24 bits a la derecha para que el ID_evento 
                                               // esté en los 8 bits de mayor peso
    alarma = alarma | 0x800000; // El bit 23 indica que la alarma es periodica
    alarma = alarma | 0x3A98;   // Las alarmas tienen 15000ms de retardo
    set_alarma(alarma);
}

// Pre: --
// Post: Resetea la alarma. 
void resetear_alarma_power_down(void) {
    // En realidad solo tiene que solicitar a GestorAlarmas que la sobreescriba, poniendo así
    // el nuevo tiempo (15 segundos) en ella.
	iniciar_alarma_power_down();
}
