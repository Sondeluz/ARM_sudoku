#ifndef __TEMPORIZADOR_H
#define __TEMPORIZADOR_H


#include "stdint.h"

// Pre: --
// Post: Configura el timer1 como un temporizador de un microsegundo de precisión, que genera el mínimo número de interrupciones
// para dicha precisión.
void temporizador_iniciar(void);

// Pre: --
// Post: Devuelve el número de veces que timer1 ha pasado por 0xFFFFFFFF microsegundos
unsigned int leer_cuenta(void);

// Pre: --
// Post: Inicia la cuenta del temporizador 1
void temporizador_empezar(void);


// Pre: --
// Post: Para la cuenta del temporizador 1, y devuelve su cuenta de tiempo, en microseundos
long temporizador_parar(void);

// Pre: --
// Post: IRQ que encola un evento de comprobación del temporizador periódico. Ejecutada por el timer 0.
void encolar_evento (void);

// Pre: --
// Post: Configura el timer0 como un temporizador periódico. El periodo se indica en milisegundos.
void temporizador_periodico(int periodo);

// Pre:  --
// Post: Devuelve el valor del temporizador periódico en milisegundos
long temporizador_leer_periodico(void);

// Pre: --
// Post: Devuelve la cuenta de tiempo del temporizador 1, en microsegundos
uint32_t __swi(2) clock_gettime(void);

// Función SWI que devuelve la cuenta del tiempo de timer1 en microsegundos
uint32_t __SWI_2 (void);

// Pre: --
// Post: Alternativa a clock_gettime para su uso en FIQ, ya que puede 
//       dar problemas ejecutar SWI desde FIQ debido a los cambios de modo
uint32_t temporizador_leer_tiempo (void);


#endif  // __TEMPORIZADOR_H
