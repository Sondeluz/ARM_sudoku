#include "cola.h"
#include <stdio.h>
#include "eventos.h"
#include "Gestor_IO.h"
#include "temporizador.h"
#include <LPC210x.H>

#define MAX_DATOS 32

// Funciones de desactivar y activar interrupciones IRQ y FIQ escritas en ensamblador, 
// para facilitar la exclusión mútua en las operaciones de lectura y escritura
// frente a interrupciones que puedan cortarlas.
//
// Se encuentran implementadas como SWI, en SWI_Handler.s para sus respectivos IDs.
void __swi(0xFF) enable_isr (void);
uint32_t __swi(0xFE) disable_isr (void); 
 
void __swi(0xFD) enable_isr_fiq (void); 
uint32_t __swi(0xFC) disable_isr_fiq (void);

// Función auxiliar implementada en ensamblador (es_modo_fiq.s), que 
// devuelve 1 si el procesador se encuentra en modo ejecución de FIQ
// o 0 en caso contrario, leyendo el CPSR.
int es_modo_fiq(void);

// Struct de Cola circular de eventos, con un array de tamaño fijo e índices
struct Cola {
    EventoCola cola[MAX_DATOS];
    uint8_t tamanyo, numProcesados, indiceProcesados, siguienteIndice;
};

// Instancia local de la cola de eventos
static volatile struct Cola colaLocal;

// Inicia la cola a valores seguros
void inicializar_cola(void) {
    colaLocal.tamanyo = 0;    
    colaLocal.numProcesados = 0;
    colaLocal.siguienteIndice = 0;
    colaLocal.indiceProcesados = 0;
}

// Función auxiliar que devuelve 1 si el procesador se encuentra en modo 
// ejecución de FIQ o 0 en caso contrario
//
// Se utiliza para cambiar el comportamiento de cola_guardar_eventos
// según el llamador (user, IRQ o FIQ)
int ejecucionEnFIQ(void) {
    return es_modo_fiq();
}

// Pre: --
// Post: Guarda un evento en la cola. 
// Si hay espacio en la cola, se inserta ordenadamente.
//
// Si la cola se encuentra llena, pero hay eventos ya procesados, 
// se inserta ordenadamente sobreescribiendo el primero de ellos. 
//
// Si la cola se encuentra llena y no hay eventos procesados, se
// entra en estado de overflow con un bucle infinito.
void cola_guardar_eventos(ID_evento ID_evento, uint32_t auxData) { 
    EventoCola eventoEnvio; // Evento a rellenar en caso de overflow

    // Valor de los bits de I y F del CPSR del procesador, para saber
    // si las interrupciones estaban deshabilitadas antes de entrar a
    // la función.
    // 
    // Si estaban a 0 (interrupciones activas), se activarán al terminar.
    uint32_t valorI; uint32_t valorF;

	int ejecutandoEnFIQ = ejecucionEnFIQ();
	
    // Si el procesador no se encuentra en modo FIQ, se 
    // desactivan las interrupciones incondicionalmente a partir de
    // este momento
    if (! ejecutandoEnFIQ) {
        valorI = disable_isr();
        valorF = disable_isr_fiq();
    }
    
    if (colaLocal.tamanyo < MAX_DATOS){ // Hay espacio en la cola
        EventoCola evento; // Se crea el evento con los datos indicados
        evento.id_evento = ID_evento;
        evento.auxData = auxData;
			
        // Si el procesador se encuentra en modo FIQ, se rellena el timestamp
        // con una función no SWI, para evitar problemas de corrupción de
        // registros al pasar a modo SVC y poder ser interrumpidos por otra FIQ    
        if (! ejecutandoEnFIQ) {
            evento.timestamp = clock_gettime();
        } else {
            evento.timestamp = temporizador_leer_tiempo();
        }
					
        colaLocal.cola[colaLocal.tamanyo] = evento;
        colaLocal.tamanyo += 1;
        colaLocal.siguienteIndice = (colaLocal.siguienteIndice + 1) % MAX_DATOS;     
    } 
    else { // La cola está llena
        if (colaLocal.numProcesados > 0) { // Pero sabemos que hay eventos ya procesados que podemos sobreescribir
            colaLocal.cola[colaLocal.siguienteIndice].id_evento = ID_evento;
            colaLocal.cola[colaLocal.siguienteIndice].auxData = auxData;
            
            if (! ejecutandoEnFIQ) {
                colaLocal.cola[colaLocal.siguienteIndice].timestamp = clock_gettime();
            } else {
                colaLocal.cola[colaLocal.siguienteIndice].timestamp = temporizador_leer_tiempo();
            }
            
            colaLocal.siguienteIndice = (colaLocal.siguienteIndice + 1) % MAX_DATOS; 
        }
        else { // Y no hay eventos procesados, por lo que es un overflow

            // Debido a que no nos podemos comunicar con el planificador 
            // mediante la cola sin sobreescribir eventos no procesados,
            // entramos aquí en el estado de overflow, y dejamos que el
            // Watchdog salte.

            // Indicamos a Gestor_IO que active la salida de overflow
            eventoEnvio.id_evento = OVERFLOW;
            tratarEventoIO(eventoEnvio);
            
            // Y entramos en un bucle infinito
            while (1) {} 
        } 
    }

    // Si había eventos procesados, disminuimos el contador en 1
    if (colaLocal.numProcesados){
        colaLocal.numProcesados = (colaLocal.numProcesados - 1);
    }

    if (! ejecutandoEnFIQ) {    
        // Si el bit de IRQ en CPSR estaba antes a 0 (interrupciones activas), 
        // las volvemos a activar.
        // Esto evita causar problemas en IRQs, que de por sí escriben en el bit I 
        // y desactivan las interrupciones el empezar, y podrían encadenarse múltiples
        // al volver a activar las interrupciones.
        if (valorI == 0) enable_isr();
        if (valorF == 0) enable_isr_fiq();
    }
}

// Pre: -- 
// Post: Lee el primer evento de la cola no procesado, escribiéndolo 
// en la dirección indicada y devolviendo 1.
// 
// Si no hay eventos por procesar, no escribe nada y devuelve 0.
int cola_leer_evento(EventoCola *evento) {
    // Valor de los bits de I y F del CPSR del procesador antes de haber
    // desactivado las interrupciones IRQ y FIQ, para saber
    // si las interrupciones estaban deshabilitadas antes de entrar a
    // la función.
    // 
    // Si estaban a 0 (interrupciones activas), se activarán al terminar.
    uint32_t valorI = disable_isr();
    uint32_t valorF = disable_isr_fiq();
    
    // Si hay eventos por leer
    if (nuevos_eventos()) { 
        *evento = colaLocal.cola[colaLocal.indiceProcesados];
        colaLocal.numProcesados = colaLocal.numProcesados + 1; // El evento queda contabilizado como procesado
        colaLocal.indiceProcesados = (colaLocal.indiceProcesados + 1) % MAX_DATOS; // Y el índice al siguiente evento se mueve
        
        // Si el bit de IRQ en CPSR estaba antes a 0 (interrupciones activas), 
        // las volvemos a activar.
        // Esto evita causar problemas en IRQs, que de por sí escriben en el bit I 
        // y desactivan las interrupciones el empezar, y podrían encadenarse múltiples
        // al volver a activar las interrupciones.
        if (valorI == 0) enable_isr();
        if (valorF == 0) enable_isr_fiq();
        
        return 1;
    }

    if (valorI == 0) enable_isr();
    if (valorF == 0) enable_isr_fiq();
    
    // No hay eventos para leer
    return 0;
}

// Devuelve 1 si hay nuevos eventos por leer, 0 en caso contrario.
int nuevos_eventos(void) {
	uint8_t tamanyo;
	uint8_t numProcesados;
	
	// Deshabilita y habilita las interrupciones incondicionalmente, porque
	// sabemos que se llama únicamente desde el planificador en modo usuario
	disable_isr();
    disable_isr_fiq();
		
	tamanyo = colaLocal.tamanyo;
	numProcesados = colaLocal.numProcesados;
		
	enable_isr();
    enable_isr_fiq();
	
    // Devuelve 1 si y solo si hay algun elemento en la cola y no se han 
    // procesado todos los datos.
    return (tamanyo > 0 && numProcesados < MAX_DATOS);
}
