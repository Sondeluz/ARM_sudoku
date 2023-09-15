#include "Gestor_pulsacion.h"
#include "GestorAlarmas.h"
#include "temporizador.h"
#include "eventos.h"


// Pre: --
// Post: Configura alarmas de comprobación de pulsaciones de los botones (eint1 y eint2)
void inicializar_gestor_pulsacion(void) {
    uint32_t alarma1 = EVENTO_COMPROBAR_PULSACION_1 << 24; // Shifteamos 24 bits a la derecha para que el ID_evento 
    uint32_t alarma2 = EVENTO_COMPROBAR_PULSACION_2 << 24; // esté en los 8 bits de mayor peso
    
    alarma1 = alarma1 | 0x800000; // El bit 23 indica que la alarma es periodica
    alarma2 = alarma2 | 0x800000;

    alarma1 = alarma1 | 0x64;   // Las alarmas tienen 100 ms de retardo
    alarma2 = alarma2 | 0x64;

    set_alarma(alarma1);
    set_alarma(alarma2); 
}
