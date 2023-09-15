#ifndef __COMANDOS_H
#define __COMANDOS_H

#define TAM_MAX_COMANDO 10
#define DELIMITADOR_INICIAL_COMANDO '#'
#define DELIMITADOR_FINAL_COMANDO '!'

/*
    comandos.h ofrece deficiones y operaciones sobre comandos 
    de un juego, y controla y expone una máquina de estados para
    indicar si se pueden aplicar comandos o no en un momento dado.

    El permiso de aplicar comandos se controlará internamente.
*/

// Estructura de un comando compuesto por
// caracteres, con una longitud dada
typedef struct Comando {
    int longitud;
    char colaCaracteres[TAM_MAX_COMANDO];
} Comando;

// Pre: --
// Post: Trata un comando obtenido de Gestor_IO y encola un evento apropiado asociado a él.
//       Si el comando es inválido, no se realiza ninguna acción.
void tratar_comando(void);

// Pre: --
// Post: Muestra el contenido de un comando obtenido de Gestor_IO y su significado
//       por la salida ofrecida por gestor_IO, preguntando al usuario
//       si quiere ejecutarlo. Si el comando es inválido, lo indica
//       acordemente.
//
//       Durante los 3 segundos posteriores el permiso de aplicar comandos estará activo.
void mostrar_comando(void);

// Pre: --
// Post: Devuelve 1 si se pueden aplicar comandos, 0 en caso contrario.
int comandosAplicables(void);

// Pre: --
// Post: Almacena en la máquina de estados que no se puede aplicar ningún comando.
void prohibirAplicarComandos(void);

#endif  // __COMANDOS_H
