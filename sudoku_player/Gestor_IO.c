#include "Gestor_IO.h"
#include "eventos.h"
#include "cola.h"
#include "GestorAlarmas.h"
#include "GPIO.h"
#include "comandos.h"
#include "cola.h"
#include "gestor_UART.h"

/* Parpadeos:
    - Si ocurre un error: 12 parpadeos cada 250ms (3000ms de parpadeos rápidos)
    
    - Si hay una jugada pendiente: 6 parpadeos cada 500 ms (3000ms de parpadeos lentos)
    
    - Si se ha validado una jugada: Un parpadeo durante un segundo
*/

#define PARPADEOS_LED_ERROR 12
#define PARPADEOS_LED_JUGADA_PENDIENTE 6

#define RETARDO_ALARMA_LED_JUGADA_VALIDA 0x3E8      // 1000ms
#define RETARDO_ALARMA_LED_ERROR 0xFA               // 250ms
#define RETARDO_ALARMA_LED_JUGADA_PENDIENTE 0x1F4   // 500 ms

// Buffer de escritura de salida a línea serie estático,
// con control de tamaño y caracteres escritos hasta un momento dado
#define MAX_CARACTERES 10000
static char bufferEscritura[MAX_CARACTERES];
static int tamanyoBuffer = 0;
static int charsEscritos = 0;

// Buffer de parseo de un comando, con control de tamaño
// y flag de bloqueo según si se ha leído o no
static char bufferComando[TAM_MAX_COMANDO];
static uint8_t indiceBufferComando = 0;  
static int bufferComandoLeido = 1;

// Indicador de cuántas veces se ha parpadeado una salida 
// de error o jugada pendiente
static int vecesParpadeado = 0;


// Pre:--
// Post: Inicializa el gestor, configurando los dispositivos
// de entrada y salida que vaya a tratar y las alarmas
// que vaya a configurar para el planificador
void gestor_IO_inicializar (void) {
    // Configuramos la GPIO por completo
    GPIO_iniciar();
    
    // Visualización y gestión del tablero
    GPIO_marcar_salida(13, 1); // Led de validar una entrada, confirmar acciones o mostrar errores,
                               // que parpadeará de forma diferente según la razón.
    
    // Los pines de EINT1 y EINT2 están configurados al inicializar las interrupciones externas

    // Indicadores
    GPIO_marcar_salida(30, 1); // Indicador de overflow
    GPIO_marcar_salida(31, 1); // Latido <3 modo idle
}

// Pre: --
// Post: Recibe un evento del planificador, y actúa acordemente
// según el identificador de evento y datos auxiliares indicados.
// Por simplicidad, y debido a que se tienen que enviar los
// mismos tipos de datos, recibe un evento de la cola del
// planificador
void tratarEventoIO (EventoCola evento) {
    uint32_t alarma;
	uint32_t valor; 
    
    switch (evento.id_evento) {
        // Se ha leído un caracter proveniente de la línea serie,
        // se avanza el autómata
        case CARACTER_LEIDO:
            valor = evento.auxData;

            if (bufferComandoLeido) {
                if (valor == DELIMITADOR_INICIAL_COMANDO) { // Resetea la cola de caracteres al leer el caracter inicial
                    indiceBufferComando = 0; 
                }
                else if (valor == DELIMITADOR_FINAL_COMANDO && indiceBufferComando < TAM_MAX_COMANDO) { // Caracter final
                    bufferComando[indiceBufferComando] = valor;
                    indiceBufferComando += 1;
                    cola_guardar_eventos(NUEVO_COMANDO, 0);
                    bufferComandoLeido = 0;
                }
                else if (indiceBufferComando < TAM_MAX_COMANDO) { // Caracter intermedio (sin significado para el gestor)
                    bufferComando[indiceBufferComando] = valor;
                    indiceBufferComando += 1;
                }
                else { // Se ha sobrepasado el límite de caracteres para un comando, se reinicia el índice para prevenir overflows
                    indiceBufferComando = 0;
                }
            }
            
            break;

        // Se ha realizado una jugada válida en el juego, y hay que indicárselo al jugador
        case JUGADA_VALIDA:
            // Escribe el indicador y configura una alarma para desactivarlo más tarde
            GPIO_escribir(13, 1, 1);

            // Encola una alarma para desactivar el led del GPIO dentro de 1 segundo
            alarma = DESACTIVAR_SALIDA_JUGADA_VALIDA << 24;  // Shifteamos 24 bits a la derecha para que el ID_evento 
                                                          // esté en los 8 bits de mayor peso
            alarma = alarma | RETARDO_ALARMA_LED_JUGADA_VALIDA;
            set_alarma(alarma);
            break;

        case DESACTIVAR_SALIDA_JUGADA_VALIDA:
            GPIO_escribir(13, 1, 0); // Desactiva el led de jugada válida
            break;

        case LATIDO_MODO_IDLE:
            // Invierte la salida del latido de modo idle (para que parpadee, y no haya que distinguir entre dos eventos)
            GPIO_escribir(31, 1, ~GPIO_leer(31,1));
            break;

        // Hay una jugada pendiente y hay que indicárselo al jugador
        case JUGADA_PENDIENTE:
            vecesParpadeado = 1;

            GPIO_escribir(13, 1, 1);

            // Encola una alarma para volver a parpadear la salida 
            alarma = PARPADEAR_SALIDA_JUGADA_PENDIENTE << 24;   // Shifteamos 24 bits a la derecha para que el ID_evento 
                                                                // esté en los 8 bits de mayor peso
            alarma = alarma | RETARDO_ALARMA_LED_JUGADA_PENDIENTE;
            set_alarma(alarma);

            break;

        // Hay que seguir parpadeando la salida de jugada pendiente
        case PARPADEAR_SALIDA_JUGADA_PENDIENTE:
            if (vecesParpadeado == PARPADEOS_LED_JUGADA_PENDIENTE) { // Deja de pedir volver a parpadearlo
                vecesParpadeado = 0;
                GPIO_escribir(13, 1, 0);
            } else {
                // Escribe el indicador y configura una alarma para desactivarlo más tarde
                GPIO_escribir(13, 1, ~GPIO_leer(13,1));

                // Vuelve a encolar la alarma
                alarma = PARPADEAR_SALIDA_JUGADA_PENDIENTE << 24;   // Shifteamos 24 bits a la derecha para que el ID_evento 
                                                                    // esté en los 8 bits de mayor peso
                alarma = alarma | RETARDO_ALARMA_LED_JUGADA_PENDIENTE;
                set_alarma(alarma);

                vecesParpadeado++;
            }

            break;    

        // Ha habido un error y hay que indicárselo al usuario
        case ERROR:
            vecesParpadeado = 1;

            GPIO_escribir(13, 1, 1);

            // Encola una alarma para desactivar el led del GPIO dentro de 1 segundo
            alarma = PARPADEAR_SALIDA_ERROR << 24;  // Shifteamos 24 bits a la derecha para que el ID_evento 
                                                    // esté en los 8 bits de mayor peso
            alarma = alarma | 0xFA;   // La alarma tiene 250 ms de retardo
            set_alarma(alarma);

        
        case PARPADEAR_SALIDA_ERROR:
            if (vecesParpadeado == PARPADEOS_LED_ERROR) { // Deja de pedir volver a parpadearlo
                vecesParpadeado = 0;
                GPIO_escribir(13, 1, 0);
            } else {
                // Escribe el indicador y configura una alarma para desactivarlo más tarde
                GPIO_escribir(13, 1, ~GPIO_leer(13,1));

                // Encola una alarma para desactivar el led del GPIO dentro de 1 segundo
                alarma = PARPADEAR_SALIDA_ERROR << 24;          // Shifteamos 24 bits a la derecha para que el ID_evento 
                                                                // esté en los 8 bits de mayor peso
                alarma = alarma | RETARDO_ALARMA_LED_ERROR;     // La alarma tiene 250 ms de retardo
                set_alarma(alarma);

                vecesParpadeado++;
            }
            break;

        case OVERFLOW:
            // Activa el led de overflow incondicionalmente
            GPIO_escribir(30, 1, 1);
            break;
    }
}

// Pre: --
// Post: Devuelve un char a escribir por dispositivos de línea serie, o '\0' si no
//       existe contenido por escribir
char obtenerSiguienteChar(void) {
    if (charsEscritos < tamanyoBuffer) {
        return bufferEscritura[charsEscritos++];
    } else {
        return '\0'; // Si por algún casual la UART nos pide un char cuando ya se ha terminado de escribir
                     // devuelve el fin de string de C
    }
}

// Pre: string es un string válido de C, terminado en '\0'
// Post: Gestiona la escritura del string indicado en el dispositivo de línea serie
void mostrarBuffer(char string[]) {
    int i = 0;
    
    // Lee hasta encontrar el fin del string o no poder seguir
    // para evitar overflow
    while (string[i] != '\0' && i < MAX_CARACTERES) {
        bufferEscritura[i] = string[i];
        i++;
    }

    tamanyoBuffer = i;
     charsEscritos = 0;
    
    if (tamanyoBuffer > 0) {
        // Inicia la escritura del buffer en la UART, que después
        // irá pidiendo nuevos caracteres a escribir asíncronamente
        iniciarEscritura(bufferEscritura[charsEscritos++]);
    }
}

// Pre: --
// Post: Reinicia la máquina de estados de reconocimiento de comandos
void cancelarComando(void) {
    bufferComandoLeido = 1;
    indiceBufferComando = 0;
}

// Pre: --
// Post: Obtiene el comando reconocido por la máquina de estados hasta el momento.
//       No está garantizado que el comando sea válido.
void obtenerComando(Comando *comando) {
	int i;
	
    for (i = 0; i < indiceBufferComando; i++) {
        comando->colaCaracteres[i] = bufferComando[i];
    }

    comando->longitud = indiceBufferComando;
}

// Pre: --
// Post: Indica a la máquina de estados que puede empezar a reconocer un nuevo comando
void indicarComandoLeido(void) {
    // Mismo comportamiento que cancelar un comando
    cancelarComando();
}



