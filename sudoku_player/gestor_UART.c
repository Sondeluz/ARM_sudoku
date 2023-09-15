#include "gestor_UART.h"
#include "eventos.h"
#include "cola.h"
#include <LPC210X.H>    // LPC21XX Peripheral Registers
#include <inttypes.h>
#include "Gestor_IO.h"


// Estructuras para escribir un string en la UART.
// No utilizadas al no usar la operación de escritura de buffer 
// y preferir realizar encuestas en las interrupciones, char por char

// Al compilar en O0, si no se incluyen la función y estructuras inutilizadas
// ocurrirá un prefetch abort a la salida de la segunda llamada a mostrarBuffer,
// que no hemos sido capaces de depurar por su comportamiento extraño.
//
// En O1, O2 y O3 -Otime no ocurre ningún fallo con o sin el código comentado.

/*#define MAX_CARACTERES 1000

static volatile char buffer[MAX_CARACTERES];
static volatile int tamanyoBuffer = 0;
static volatile int charsEscritos = 0;*/

void UART1_ISR (void) __irq;

void UART_init(void) {
    U1LCR = 0x82;                // carácteres de 7 bits y bit DLAB a 1 para poder indicar los baudios
    PINSEL0 |=  0x50000;         // Configura los pines con los bits 16 y 18 a 1, activando serial input / serial output             
    
    // Baudios = PCLK/(16*(256*0+U1DLL)), donde PCLK = 15 Mhz = 15000000 Hz
    // Por ejemplo, para 9600 baudios, U1DLL debe ser 97
    // Para obtener un ratio algo mayor para escribir más rapidamente, como 19200 baudios, U1DLL debe ser 48
    U1DLL = 48;                  
    U1LCR = 0x03;                // Pasa el bit DLAB a 0 para poder leer más tarde U1IER

    U1IER |= 0x01;               // Activa interrupciones por Receive data Available (entrada en UART)
                                	
    // Asigna la ISR de UART1 al VIC, que tratará eventos de lectura y escritura
    VICVectAddr4 = (unsigned long) UART1_ISR;
    VICVectCntl4 = 0x20 | 7;        // Habilita el slot del VIC con 0x20 (bit 5), y configura el número de interupción a tratar (7: UART1)
  

    VICIntEnable |= 0x00000080;     // Activa interrupciones de UART1 en el VIC
}

// Pre: --
// Post: Empieza a escribir un char en la UART,
//       poniendo en marcha el sistema de interrupciones
//       por pila de escritura vacía
void iniciarEscritura(char c) {
    // Empieza escribiendo un char, y habilita las interrupciones de THRE 
    // (cambios en la pila de salida) inmediatamente después
    U1THR = c;
    U1IER |= 0x02; // bit 1 a 1: activa interrupciones por THRE
}

// Pre: --
// Post: Función auxiliar que inhabilita interrupciones por THRE, para evitar realizar
//       más encuestas de nuevos chars al ver constantemente la pila de escritura vacía
void terminarEscritura(void) {
    U1IER &= 0xFFFFFFFD; // Máscara para desactivar el bit 1 (interrupciones por THRE)
}


// Pre: string es un string válido de C
// Post: Controla la escritura de un string de c en la UART.
//       
// No utilizada al preferir realizar encuestas en las 
// interrupciones, char por char
/*void escribirBuffer(char string[]) {
    int i = 0;
    
    while (string[i] != '\0' && i < MAX_CARACTERES) {
        buffer[i] = string[i];
        i++;
    }

    tamanyoBuffer = i;
    
    charsEscritos = 0;
    if (tamanyoBuffer > 0) {
        // Escribe el primer caracter en THR, para empezar a lanzar interrupciones y escribir el resto
        U1THR = buffer[charsEscritos++]; 
		U1IER |= 0x02; // Habilita las interrupciones una vez escrito
    }
}*/

// Rutina de servicio a interrupciones por lectura o escritura en UART, que debe
// distinguir entre ambos tipos de evento
void UART1_ISR (void) __irq {
    char c;
    uint32_t valor;
	uint32_t tipoInterrupcion = U1IIR;
		
	
    if (tipoInterrupcion == 0x4) { // Bit 0 a 0: interrupcion pendiente, bits 3_1 a 010: Receive data available (entrada)
        valor = U1RBR; // Se desactiva la flag de interrupción al leer de U1RBR
    
				U1THR = valor; // Escribimos el caracter en la UART para mostrar feedback
			
        cola_guardar_eventos(CARACTER_LEIDO, valor);
    } else if (tipoInterrupcion == 0x2) { // Bit 0 a 0: interrupcion pendiente, bits 3_1 a 001: interrupción de THRE (salida)
            // Obtiene el siguiente char del buffer en gestor_IO, si existe, para escribirlo. Lo tratamos como una comunicación 
            // interna entre este gestor y el de IO por eficiencia y porque creemos que si no expondría detalles de implementación 
            // al planificador.
            c = obtenerSiguienteChar();

            if (c != '\0') {
                U1THR = c; // Se desactiva la flag de interrupción al escribir en U1THR
            } else {
                terminarEscritura();
            }
    }
  
    VICVectAddr = 0; // Avisa al VIC de que se ha tratado la IRQ
}
