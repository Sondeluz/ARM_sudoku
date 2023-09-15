#include "gestor_RTC.h"
#include <LPC210X.H>    // LPC21XX Peripheral Registers

// Pone en marcha el RTC
void RTC_init(void) {
    PCONP |= 0x200; // Escribe un 1 en el bit 9 de PCONP, para que se active el RTC
    
    CCR = 0x02;  // Activa el bit de reset
    CCR = 0x01;  // Activa el bit de CLKEN, deshabilitando a la vez el bit de reset, para habilitar la cuenta

    // Configuramos el RTC (p. 218) 
    PREINT = (15000000 / 32768) - 1;
    PREFRAC = 15000000 - ((PREINT + 1) * 32768);
}

// Resetea la cuenta interna del RTC, así como los
// contadores de tiempo transcurrido utilizados
void RTC_reset(void) {
    CCR = 0x02;  // Activa el bit de reset

    // Reinicia los contadores de tiempo utilizados
    SEC = 0;
    MIN = 0;

    CCR = 0x01; // Activa el bit de CLKEN, deshabilitando a la vez el bit de reset, para habilitar la cuenta
}

// Devuelve los minutos transcurridos desde el último
// inicio o reseteo del RTC
uint8_t RTC_leer_minutos(void){
    return MIN;
}

// Devuelve los segundos transcurridos desde el último
// minuto tras el inicio o reseteo del RTC
uint8_t RTC_leer_segundos(void){
    return SEC;
}

