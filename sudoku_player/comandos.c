#include "comandos.h"
#include "eventos.h"
#include "cola.h"
#include "gestor_IO.h"
#include "GestorAlarmas.h"

// Variable estática que indica si se pueden (1) o no (0)
// aplicar comandos
static int permisoAplicarComandos = 0;

/////////////////////////////////////////////////////////////////
// Funciones no exportadas
/////////////////////////////////////////////////////////////////

// Devuelve 1 si c es un dígito según la tabla ASCII
int esDigito(char c) {
    return (c >= '0' && c <= '9');
}

// Convierte un char en ASCII a int
int atoi(char c) {
    return c - '0';
}

// Devuelve 1 si el checksum de un comando de jugada ((fila+columna+valor) % 8) es válido
int checksumValido(Comando *comando) {
    return (((atoi(comando->colaCaracteres[0]) + atoi(comando->colaCaracteres[1]) + 
            atoi(comando->colaCaracteres[2]))%8 - atoi(comando->colaCaracteres[3]))==0);
}

// Habilita el permiso de aplicar comandos, y programa una alarma para cancelar el permiso
// en 3 segundos.
void setAlarmaTimeoutComando(void) {
    uint32_t alarma = PROHIBIR_APLICAR_COMANDOS << 24; // esté en los 8 bits de mayor peso
    alarma = alarma | 0xBB8;    // Con 3000 ms de retardo
    set_alarma(alarma);

    permisoAplicarComandos = 1;
}


/////////////////////////////////////////////////////////////////
// Funciones exportadas
/////////////////////////////////////////////////////////////////


// Pre: --
// Post: Trata un comando obtenido de  Gestor_IO y encola un evento apropiado asociado a él.
//       Si el comando es inválido, no se realiza ninguna acción.
void tratar_comando() {
	Comando comando;
	obtenerComando(&comando); // Obtiene el comando leído hasta el momento por gestor_IO, sin saber su contenido o validez
	
    // Comando NEW!
    if (comando.longitud == 4 && comando.colaCaracteres[0] == 'N' &&      
        comando.colaCaracteres[1] == 'E' && comando.colaCaracteres[2] == 'W'
        && comando.colaCaracteres[3] == '!' ) {

        cola_guardar_eventos(REINICIAR_PARTIDA,0);   // Se encola el evento de reiniciar partida
    } // Comando RST!
    else if (comando.longitud == 4 && comando.colaCaracteres[0] == 'R' && 
             comando.colaCaracteres[1] == 'S' && comando.colaCaracteres[2] == 'T'
             && comando.colaCaracteres[3] == '!' ) {

        cola_guardar_eventos(FIN_JUEGO,0);           // Se encola el evento de fin de juego
    }// Comando FCVS!
    else if (comando.longitud == 5 && esDigito(comando.colaCaracteres[0]) && 
             esDigito(comando.colaCaracteres[1]) && esDigito(comando.colaCaracteres[2]) &&  // Se comprueba que el checksum es valido
             esDigito(comando.colaCaracteres[3]) && comando.colaCaracteres[4] == '!' && checksumValido(&comando) ){ 
        
        // La codificación de auxData es la siguiente:
        uint32_t auxData = atoi(comando.colaCaracteres[0]) << 16 | // Del bit 16 al 24 -> fila 
                           atoi(comando.colaCaracteres[1]) << 8  | // Del bit 8 al 15  -> columna
                           atoi(comando.colaCaracteres[2]);        // Del bit 0 al 7   -> valor
        
        cola_guardar_eventos(NUEVA_JUGADA, auxData); // Se encola el evento de nueva jugada
    }
}

// Pre: --
// Post: Muestra el contenido de un comando obtenido de Gestor_IO y su significado
//       por la salida ofrecida por gestor_IO, preguntando al usuario
//       si quiere ejecutarlo. Si el comando es inválido, lo indica
//       acordemente.
//
//       Durante los 3 segundos posteriores el permiso de aplicar comandos estará activo.
void mostrar_comando(){
	Comando comando;
    char buffer[450];
    int i = 0;
    int j;
	
	obtenerComando(&comando); // Obtiene el comando leído hasta el momento por gestor_IO, sin saber su contenido o validez
	
    // El mensaje que se muestra es:
    // Estas segur@ de que quieres escribir: {comando.colaCaracteres[0:comando.longitud]}
    // Que significa:
    // Si comando es NEW!  -> Nueva partida
    // Si comando es RST!  -> Reiniciar partida
    // Si comando es FCVS! -> // Introducir {comando.colaCaracteres[2]->valor} en {comando.colaCaracteres[0]->fila} fila y en columna {comando.colaCaracteres[1]->columna}
    // Cualquier otro caso -> Comando invalido
    //Dispones de tres segundos para aceptar o cancelar accion
    buffer[i++] = 'E'; buffer[i++] = 's'; buffer[i++] = 't'; buffer[i++] = 'a'; buffer[i++] = 's'; buffer[i++] = ' ';
    buffer[i++] = 's'; buffer[i++] = 'e'; buffer[i++] = 'g'; buffer[i++] = 'u'; buffer[i++] = 'r'; buffer[i++] = '@';
    buffer[i++] = ' '; buffer[i++] = 'd'; buffer[i++] = 'e'; buffer[i++] = ' '; buffer[i++] = 'q'; buffer[i++] = 'u';
    buffer[i++] = 'e'; buffer[i++] = ' '; buffer[i++] = 'q'; buffer[i++] = 'u'; buffer[i++] = 'i'; buffer[i++] = 'e'; 
    buffer[i++] = 'r'; buffer[i++] = 'e'; buffer[i++] = 's'; buffer[i++] = ' '; buffer[i++] = 'e'; buffer[i++] = 's';
    buffer[i++] = 'c'; buffer[i++] = 'r'; buffer[i++] = 'i'; buffer[i++] = 'b'; buffer[i++] = 'i'; buffer[i++] = 'r';
    buffer[i++] = ':'; buffer[i++] = ' ';

    for (j = 0; j< comando.longitud; j++){
        buffer[i++] = comando.colaCaracteres[j];
    }
    buffer[i++] = '\n';
    buffer[i++] = 'Q'; buffer[i++] = 'u'; buffer[i++] = 'e'; buffer[i++] = ' '; buffer[i++] = 's'; buffer[i++] = 'i';
    buffer[i++] = 'g'; buffer[i++] = 'n'; buffer[i++] = 'i'; buffer[i++] = 'f'; buffer[i++] = 'i'; buffer[i++] = 'c';
    buffer[i++] = 'a'; buffer[i++] = ':'; buffer[i++] = ' '; 
    
    if (comando.longitud == 4 && comando.colaCaracteres[0] == 'N' &&      // Comando NEW!
        comando.colaCaracteres[1] == 'E' && comando.colaCaracteres[2] == 'W'
        && comando.colaCaracteres[3] == '!' ) {
            buffer[i++] = 'N'; buffer[i++] = 'u'; buffer[i++] = 'e'; buffer[i++] = 'v'; buffer[i++] = 'a'; buffer[i++] = ' '; 
            buffer[i++] = 'p'; buffer[i++] = 'a'; buffer[i++] = 'r'; buffer[i++] = 't'; buffer[i++] = 'i'; buffer[i++] = 'd'; 
            buffer[i++] = 'a';
    }
    else if (comando.longitud == 4 && comando.colaCaracteres[0] == 'R' && // Comando RST!
             comando.colaCaracteres[1] == 'S' && comando.colaCaracteres[2] == 'T'
             && comando.colaCaracteres[3] == '!' ) {
            buffer[i++] = 'R'; buffer[i++] = 'e'; buffer[i++] = 'i'; buffer[i++] = 'n'; buffer[i++] = 'i'; buffer[i++] = 'c'; 
            buffer[i++] = 'i'; buffer[i++] = 'a'; buffer[i++] = 'r'; buffer[i++] = ' '; buffer[i++] = 'p'; buffer[i++] = 'a'; 
            buffer[i++] = 'r'; buffer[i++] = 't'; buffer[i++] = 'i'; buffer[i++] = 'd'; buffer[i++] = 'a'; 
    }
    else if (comando.longitud == 5 && esDigito(comando.colaCaracteres[0]) && // Comando FCVS!
             esDigito(comando.colaCaracteres[1]) && esDigito(comando.colaCaracteres[2]) && 
             esDigito(comando.colaCaracteres[3]) && checksumValido(&comando) && comando.colaCaracteres[4] == '!'){ 
            buffer[i++] = 'I'; buffer[i++] = 'n'; buffer[i++] = 't'; buffer[i++] = 'r'; buffer[i++] = 'o'; buffer[i++] = 'd'; 
            buffer[i++] = 'u'; buffer[i++] = 'c'; buffer[i++] = 'i'; buffer[i++] = 'r'; 
            
            buffer[i++] = ' ';buffer[i++] = comando.colaCaracteres[2]; buffer[i++] = ' ';

            buffer[i++] = 'e'; buffer[i++] = 'n'; buffer[i++] = ' '; buffer[i++] = 'f'; buffer[i++] = 'i';buffer[i++] = 'l'; 
            buffer[i++] = 'a';  
            
            buffer[i++] = ' ';buffer[i++] = comando.colaCaracteres[0]; buffer[i++] = ' ';

            buffer[i++] = 'y'; buffer[i++] = ' '; buffer[i++] = 'e'; buffer[i++] = 'n'; buffer[i++] = ' '; buffer[i++] = 'c'; 
            buffer[i++] = 'o'; buffer[i++] = 'l'; buffer[i++] = 'u'; buffer[i++] = 'm'; buffer[i++] = 'n'; buffer[i++] = 'a'; 
           
            buffer[i++] = ' ';buffer[i++] = comando.colaCaracteres[1];
    }
    else{
        buffer[i++] = 'C'; buffer[i++] = 'o'; buffer[i++] = 'm'; buffer[i++] = 'a'; buffer[i++] = 'n'; buffer[i++] = 'd';
        buffer[i++] = 'o'; buffer[i++] = ' '; buffer[i++] = 'i'; buffer[i++] = 'n'; buffer[i++] = 'v'; buffer[i++] = 'a';
        buffer[i++] = 'l'; buffer[i++] = 'i'; buffer[i++] = 'd'; buffer[i++] = 'o';
    } 
    buffer[i++] = '\n';
    buffer[i++] = 'D'; buffer[i++] = 'i'; buffer[i++] = 's'; buffer[i++] = 'p'; buffer[i++] = 'o'; buffer[i++] = 'n'; 
    buffer[i++] = 'e'; buffer[i++] = 's'; buffer[i++] = ' '; buffer[i++] = 'd'; buffer[i++] = 'e'; buffer[i++] = ' '; 
    buffer[i++] = 't'; buffer[i++] = 'r'; buffer[i++] = 'e'; buffer[i++] = 's'; buffer[i++] = ' '; buffer[i++] = 's';
    buffer[i++] = 'e'; buffer[i++] = 'g'; buffer[i++] = 'u'; buffer[i++] = 'n'; buffer[i++] = 'd'; buffer[i++] = 'o'; 
    buffer[i++] = 's'; buffer[i++] = ' '; buffer[i++] = 'p'; buffer[i++] = 'a'; buffer[i++] = 'r'; buffer[i++] = 'a'; 
    buffer[i++] = ' '; buffer[i++] = 'a'; buffer[i++] = 'c'; buffer[i++] = 'e'; buffer[i++] = 'p'; buffer[i++] = 't';
    buffer[i++] = 'a'; buffer[i++] = 'r'; buffer[i++] = ' '; buffer[i++] = 'o'; buffer[i++] = ' '; buffer[i++] = 'c'; 
    buffer[i++] = 'a'; buffer[i++] = 'n'; buffer[i++] = 'c'; buffer[i++] = 'e'; buffer[i++] = 'l'; buffer[i++] = 'a'; 
    buffer[i++] = 'r'; buffer[i++] = ' '; buffer[i++] = 'l'; buffer[i++] = 'a'; buffer[i++] = ' '; buffer[i++] = 'a'; 
    buffer[i++] = 'c'; buffer[i++] = 'c'; buffer[i++] = 'i'; buffer[i++] = 'o'; buffer[i++] = 'n'; buffer[i++] = '\n'; 
    buffer[i++] = '\n';buffer[i++] = '\n'; buffer[i++] = '\0';

    mostrarBuffer(buffer);
    setAlarmaTimeoutComando();
}

// Pre: --
// Post: Devuelve 1 si se pueden aplicar comandos, 0 en caso contrario.
int comandosAplicables(void) {
    return permisoAplicarComandos;
}

// Pre: --
// Post: Almacena en la máquina de estados que no se puede aplicar ningún comando.
void prohibirAplicarComandos(void) {
    permisoAplicarComandos = 0;
}
