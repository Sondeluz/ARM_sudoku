#ifndef __EVENTOS_H
#define __EVENTOS_H

#include "stdint.h" // uints

typedef uint8_t ID_evento;

enum eventos {
    // ID de evento que no se utiliza y tomará valor 0, para que no genere problemas al encolarse en el gestor de alarmas
    EVENTO_DUMMY, 
    
    // Evento de la alarma periódica de visualización de valor y candidatos de la celda seleccionada
    ACTUALIZAR_VISUALIZACION_CELDA, 
    
    // Evento de dormir al procesador 
    DORMIR_PROCESADOR, 
    
    // Evento del temporizador periódico (genérico, el planificador actuará acordemente al recibirlo)
    EVENTO_TEMPORIZADOR_PERIODICO,
    
    // Eventos de confirmación (eint1) y cancelación (eint2) de un comando
    CONFIRMAR_COMANDO,
    CANCELAR_COMANDO,

    // Evento de activación y desactivación de la salida de jugada válida
    JUGADA_VALIDA,
    DESACTIVAR_SALIDA_JUGADA_VALIDA,

    // Eventos de cambios en las entradas, enviados por Gestor_IO
    CAMBIO_FILA_SELECCIONADA,
    CAMBIO_COLUMNA_SELECCIONADA,
    CAMBIO_VALOR_A_INTRODUCIR,
    
    // Eventos de overflow, error y desactivación de la salida de error al haber sido tratado
    OVERFLOW,
    ERROR,
    DESACTIVAR_LED_ERROR,

    // Eventos de comprobación de la pulsación de los botones (eint1 y eint2)
    EVENTO_COMPROBAR_PULSACION_1,
    EVENTO_COMPROBAR_PULSACION_2,

    // Evento de latido en modo idle, tratado tanto por el planificador como por Gestor_IO
    LATIDO_MODO_IDLE,

    // Evento de caracter leído por la UART
    CARACTER_LEIDO,

    // Evento de nuevo comando (válido o inválido) leído por gestor_IO
    NUEVO_COMANDO,

    // Eventos de comandos confirmados
    FIN_JUEGO,              
    NUEVA_JUGADA,
    REINICIAR_PARTIDA,

    // Evento de prohibición de aplicar comandos
    PROHIBIR_APLICAR_COMANDOS,

    // Evento de hacer parpadear de las salidas de error y de jugada pendiente
    PARPADEAR_SALIDA_ERROR,
    PARPADEAR_SALIDA_JUGADA_PENDIENTE,

    // Evento de existencia de una jugada pendiente por ser aceptada
    JUGADA_PENDIENTE,

    // Evento de señalar el inicio del juego
    INICIAR_JUEGO,
};

// Definición de posibles errores del juego
// Se ha optado por introducirlos en el campo auxiliar de datos de un evento de la cola de
// eventos, permitiendo que sea tratado acordemente si se decidiera realizar algún tipo de
// corrección, o visualizarlo de diferentes formas.
enum errores {
    ERROR_BORRAR_PISTA_INICIAL,             // Intentar borrar el valor de una pista inicial
    ERROR_ESCRIBIR_PISTA_INICIAL,           // Intentar escribir en el valor de una pista inicial
    ERROR_ESCRIBIR_FUERA_DE_TABLERO,        // Intentar escribir o borrar fuera del tablero
    ERROR_ESCRIBIR_NO_CANDIDATO,            // Intentar escribir un valor no candidato en una celda del sudoku (específico para el juego)
    ERROR_LEER_FUERA_DE_TABLERO,            // Intentar leer fuera del tablero
    ERROR_COMANDO_INVALIDO,                 // Intentar escibir un comando mas grande que el tamaño del buffer
};


#endif  // __EVENTOS_H
