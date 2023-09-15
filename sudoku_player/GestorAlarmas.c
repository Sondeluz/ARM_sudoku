#include "GestorAlarmas.h"
#include "stdint.h" 
#include "cola.h"
#include "temporizador.h"

#define MAX_ALARMAS 8

// Tipo de datos alarma, con sus datos (ID de vento, periodicidad y tiempo) 
// y un temporizador asociado a modificar y tratar por el gestor
typedef struct Alarma {
    uint32_t datos_alarma;
    uint32_t temporizador;
} Alarma;

// Array de alarmas, no circular.
static Alarma alarmas[MAX_ALARMAS];

// Pre: --
// Post: Introduce una alarma nueva en el gestor. 
// Si se encuentra, se reprograma y resetea con el tiempo indicado.
// Si no se encuentra, se introduce en el mismo, o se genera un evento de oveflow si no queda espacio.
void set_alarma(uint32_t datos_alarma) {
    int i, encontrada = 0;
    // Comprobamos si la alarma está ya almacenada, en cuyo caso la reprogramamos
    for (i = 0; i < MAX_ALARMAS && !encontrada; i++){
        // Si los IDs de evento son iguales (aplicando una máscara sobre los bits de ID_evento)
        if ((alarmas[i].datos_alarma & 0xFF000000) == (datos_alarma & 0xFF000000)) { 				
            // Sobreescribimos la alarma, con su nuevo tiempo que tenga asignado, si lo tuviera
			alarmas[i].datos_alarma = datos_alarma;              
            // Reseteamos también el temporizador con el nuevo tiempo que tenga asignado, si lo tuviera (con una máscara sobre los bits de tiempo)
            alarmas[i].temporizador = datos_alarma & 0x007FFFFF; 
            encontrada = 1;
        }
    }

    if (!encontrada){
        // Si no se encuentra, se guarda en una nueva posición del array si es posible
        introducir_alarma(datos_alarma); 
    }
}

// Pre: --
// Post: Función auxiliar a set_alarma
void introducir_alarma(uint32_t datos_alarma) {
    int i, escrito = 0;
    for (i = 0; i < MAX_ALARMAS && !escrito; i++){
        if (alarmas[i].datos_alarma == 0){
            // Hemos encontrado una posición vacía, introducimos la alarma y el nuevo temporizador a modificar
            alarmas[i].datos_alarma = datos_alarma;
            alarmas[i].temporizador = datos_alarma & 0x007FFFFF; // Máscara sobre los bits de tiempo
            escrito = 1;
        }
    }

    // Si no se ha escrito, entonces ha habido un overflow, y tenemos que parar la ejecución
    if (!escrito) {
        cola_guardar_eventos(OVERFLOW, 1); // Indicamos un overflow al planificador
    }
}

// Pre: --
// Post: Disminuye en uno los milisegundos de todas las alarmas del gestor, y las dispara 
// si sus cuentas han alcanzado su fin. Si no eran periódicas y se han disparado, se
// eliminan también del gestor.
void disminuir_y_lanzar_alarmas(void) {
	int i;
    for (i = 0; i < MAX_ALARMAS; i++) {
        // Si la posición tiene una alarma
        if (alarmas[i].datos_alarma != 0) { 
            // Comprobamos si el valor del temporizador disminuido en uno termina la cuenta,
            // con una máscara sobre los bits de tiempo
            uint32_t nuevo_timestamp = (alarmas[i].temporizador & 0x007FFFFF) - 1;      
                                                                                        
            if (nuevo_timestamp == 0) { // Disparamos la alarma
                uint32_t id_evento = (alarmas[i].datos_alarma & 0xFF000000) >> 24; // Máscara sobre los bits de ID_evento

                cola_guardar_eventos(id_evento, 0); // Encolamos la alarma como evento con su id, y con auxData vacío al no tenerlo

                if (!(alarmas[i].datos_alarma & 0x00800000)) { // Si la alarma no es periódica, la cancelamos (Máscara sobre el bit de periodicidad)
                    alarmas[i].datos_alarma = 0;                
                    alarmas[i].temporizador = 0;
                } else { // Si no, reseteamos el temporizador al valor que tenía datos_alarma
                    alarmas[i].temporizador = alarmas[i].datos_alarma & 0x007FFFFF; 
                }
            } else { // Aún queda tiempo para que salte, sobreescribimos su valor con el nuevo, ya reducido
                alarmas[i].temporizador = nuevo_timestamp;
            } 
        }
    }
}

// Pre: --
// Post: Elimina todas las alarmas del gestor de forma segura.
void limpiar_cola_alarmas(void){
    int i;
    for (i = 0; i < MAX_ALARMAS; i++){
        alarmas[i].datos_alarma = 0;
        alarmas[i].temporizador = 0;
    }
}
