#ifndef __GESTOR_SUDOKU_H
#define __GESTOR_SUDOKU_H

#include "celda.h"
#include "sudoku_2021.h"

// Pre: --
// Post: Escribe en un valor en la cuadrícula y fila/columna indicadas. 
// Si es una escritura out-of-bounds o sobre una pista, se genera un error para el planificador. 
void escribir_valor(uint8_t fila, uint8_t columna, uint8_t valor);


// Pre: --
// Post: Reinicia los valores de las celdas del tablero que no fueran pistas.
void reiniciar_tablero(void);

// Pre: --
// Post: Crea una representación en texto del tablero del juego y la lista
//       de candidatos, y se la envía a gestor_IO para que la trate
void dibujar_tablero(void);

// Pre: auxData contiene fila, columna y valor codificados como enteros sin signo
//      de 8 bits, del MSB al LSB en dicho orden
// Post: Trata una jugada de escritura de valor
void tratar_jugada(uint32_t auxData);

// Pre: --
// Post: Crea una representación en texto del tutorial del juego
//       y se la envía a gestor_IO para que lo trate
void dibujar_tutorial(void);

// Pre: --
// Post: Crea una representación en texto del tiempo de juego
//       y procesado de candidatos_actualizar, y se la envía a 
//       gestor_IO para que lo trate
void dibujar_tiempo(uint8_t minutos, uint8_t segundos);

#endif  // __GESTOR_SUDOKU_H
