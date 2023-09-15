#ifndef __GPIO_H
#define __GPIO_H

#include "stdint.h" // uints
#include "cola.h"
#include "temporizador.h"

// Pre: --
// Post: Inicia el GPIO a valores seguros (todos los pines como salida)
void GPIO_iniciar(void);

// Pre: --
// Post: Devuelve num_bits valores del GPIO desde bit_inicial
uint32_t GPIO_leer(uint32_t bit_inicial, uint32_t num_bits);

// Pre: --
// Post: Escribe valor en los num_bits valores del GPIO desde bit_inicial. Si valor 
// no puede representarse en los bits indicados se escribirá los num_bits menos significativos 
// a partir del inicial
void GPIO_escribir(uint32_t bit_inicial, uint32_t num_bits, uint32_t valor);

// Pre: --
// Post: Marca num_bits como entrada desde bit_inicial
void GPIO_marcar_entrada(uint32_t bit_inicial, uint32_t num_bits);

// Pre: --
// Post: Marca num_bits como salida desde bit_inicial
void GPIO_marcar_salida(uint32_t bit_inicial, uint32_t num_bits);

#endif
