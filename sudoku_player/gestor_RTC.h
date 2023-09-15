#ifndef __GESTOR_RTC_H
#define __GESTOR_RTC_H

#include <inttypes.h>

/*
    gestor_RTC.h define una interfaz de gestión del RTC del chip,
    permitiendo ponerlo en marcha acorde con la frecuencia del
    procesador y leer minutos y segundos transcurridos desde el inicio
    o reset.
*/

// Pone en marcha el RTC
void RTC_init(void);

// Resetea la cuenta interna del RTC, así como los
// contadores de tiempo transcurrido utilizados
void RTC_reset(void);

// Devuelve los minutos transcurridos desde el último
// inicio o reseteo del RTC
uint8_t RTC_leer_minutos(void);

// Devuelve los segundos transcurridos desde el último
// minuto tras el inicio o reseteo del RTC
uint8_t RTC_leer_segundos(void);

#endif  // __GESTOR_RTC_H
