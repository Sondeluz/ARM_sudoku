#ifndef __GESTOR_IO_H
#define __GESTOR_IO_H

#include "cola.h"
#include "comandos.h"

/*
    Gestor_IO.h define operaciones de trata de eventos de gestión
    de salida y entrada con diferentes dispositivos, como el GPIO o
    la UART, así como una máquina de estados de lectura de comandos 
    carácter por carácter independientemente de su contenido interno, 
    que deberá ser tratado por otro módulo para darle significado una
    vez detectado (comandos.h).

    Adicionalmente, expone una interfaz de obtención de nuevos caracteres
    a escribir por dispositivos de línea serie.
*/

// Pre:--
// Post: Inicializa el gestor, configurando los dispositivos
// de entrada y salida que vaya a tratar y las alarmas
// que vaya a configurar para el planificador
void gestor_IO_inicializar (void);

// Pre: --
// Post: Recibe un evento del planificador, y actúa acordemente
// según el identificador de evento y datos auxiliares indicados.
// Por simplicidad, y debido a que se tienen que enviar los
// mismos tipos de datos, recibe un evento de la cola del
// planificador
void tratarEventoIO (EventoCola evento);


// Pre: --
// Post: Reinicia la máquina de estados de reconocimiento de comandos
void cancelarComando(void);

// Pre: --
// Post: Obtiene el comando reconocido por la máquina de estados hasta el momento.
//       No está garantizado que el comando sea válido.
void obtenerComando(Comando *comando);

// Pre: --
// Post: Indica a la máquina de estados que puede empezar a reconocer un nuevo comando
void indicarComandoLeido(void);

// Pre: --
// Post: Devuelve un char a escribir por dispositivos de línea serie, o '\0' si no
//       existe contenido por escribir
char obtenerSiguienteChar(void);

// Pre: string es un string válido de C, terminado en '\0'
// Post: Gestiona la escritura del string indicado en el dispositivo de línea serie
void mostrarBuffer(char string[]);

#endif  // __GESTOR_IO_H
