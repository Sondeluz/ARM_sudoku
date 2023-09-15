#ifndef __GESTOR_UART_H
#define __GESTOR_UART_H

#include <inttypes.h>

/*
    gestor_UART.h define la interfaz de comunicación con
    la UART para tratar sus interrupciones y escribir 
    caracteres en ella.
*/

// Inicia la UART
void UART_init(void);

// Pre: string es un string válido de C
// Post: Controla la escritura de un string de c en la UART.
//       
// No utilizada al preferir realizar encuestas en las 
// interrupciones, char por char
//void escribirBuffer(char string[]);

// Empieza a escribir un char en la UART,
// poniendo en marcha el sistema de interrupciones
// por pila de escritura vacía
void iniciarEscritura(char c);


#endif  // __GESTOR_UART_H
