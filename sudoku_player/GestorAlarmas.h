#ifndef __GESTOR_ALARMAS_H
#define __GESTOR_ALARMAS_H

#include "stdint.h" 

// Pre: --
// Post: Introduce una alarma nueva en el gestor. 
// Si se encuentra, se reprograma y resetea con el tiempo indicado.
// Si no se encuentra, se introduce en el mismo, o se genera un evento de oveflow si no queda espacio.
void set_alarma(uint32_t datos_alarma);


// Pre: --
// Post: Función auxiliar a set_alarma
void introducir_alarma(uint32_t datos_alarma);

// Pre: --
// Post: Elimina todas las alarmas del gestor de forma segura.
void limpiar_cola_alarmas(void);

// Pre: --
// Post: Disminuye en uno los milisegundos de todas las alarmas del gestor, y las dispara 
// si sus cuentas han alcanzado su fin. Si no eran periódicas y se han disparado, se
// eliminan también del gestor.
void disminuir_y_lanzar_alarmas(void);

#endif  // __GESTOR_ALARMAS_H
