#ifndef __COLA_H
#define __COLA_H

#include "eventos.h"
#include "stdint.h"

/*
    cola.h define operaciones sobre una cola ordenada de eventos.
*/

// Tipo de datos de un evento a tratar por el planificador
typedef struct EventoCola {
    ID_evento id_evento;
    uint32_t  auxData; 
    uint32_t timestamp; // Utilizado para debugging en caso de error
} EventoCola;

// Inicia la cola a valores seguros
void inicializar_cola(void);

// Pre: --
// Post: Guarda un evento en la cola. 
// Si hay espacio en la cola, se inserta ordenadamente.
//
// Si la cola se encuentra llena, pero hay eventos ya procesados, 
// se inserta ordenadamente sobreescribiendo el primero de ellos. 
//
// Si la cola se encuentra llena y no hay eventos procesados, se
// entra en estado de overflow con un bucle infinito.
void cola_guardar_eventos(ID_evento ID_evento, uint32_t auxData);

// Pre: -- 
// Post: Lee el primer evento de la cola no procesado, escribiéndolo 
// en la dirección indicada y devolviendo 1.
// 
// Si no hay eventos por procesar, no escribe nada y devuelve 0.
int cola_leer_evento(EventoCola *evento);

// Devuelve 1 si hay nuevos eventos por leer, 0 en caso contrario.
int nuevos_eventos(void);

#endif  // __COLA_H
