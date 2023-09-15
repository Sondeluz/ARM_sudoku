#include "gestorSudoku.h"
#include "tableros.h"
#include "sudoku_2021.h"
#include "eventos.h"
#include "cola.h"
#include "temporizador.h"
#include "gestor_UART.h"
#include "Gestor_IO.h"

#define MAX_CARACTERES 10000
#define CR 0x0D

// Tiempo total de ejecución de candidatos_actualizar_C, en microsegundos
static uint32_t tiempoTotalMicrosegundos = 0;

/////////////////////////////////////////////////////////////////
// Funciones no exportadas
/////////////////////////////////////////////////////////////////

// Pre: --
// Post: Función de propagar candidatos original de la práctica 1.
void candidatos_propagar(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
	                     uint8_t fila, uint8_t columna){
    uint8_t j, i , init_i, init_j, end_i, end_j;
    /* puede ayudar esta "look up table" a mejorar el rendimiento */
    const uint8_t init_region[NUM_FILAS] = {0, 0, 0, 3, 3, 3, 6, 6, 6};

    /* valor que se propaga */
    uint8_t valor = celda_leer_valor(cuadricula[fila][columna]);

    /* recorrer fila descartando valor de listas candidatos */
    for (j=0;j<NUM_FILAS;j++)
	    celda_eliminar_candidato(&cuadricula[fila][j],valor);

    /* recorrer columna descartando valor de listas candidatos */
    for (i=0;i<NUM_FILAS;i++)
	    celda_eliminar_candidato(&cuadricula[i][columna],valor);
    
    /* determinar fronteras región */
    init_i = init_region[fila];
    init_j = init_region[columna];
    end_i = init_i + 3;
    end_j = init_j + 3;

    /* recorrer region descartando valor de listas candidatos */
    for (i=init_i; i<end_i; i++) {
      for(j=init_j; j<end_j; j++) {
	      celda_eliminar_candidato(&cuadricula[i][j],valor);
	    }
    }
}

// Pre: --
// Post: Función de candidatos_actualizar original de la práctica 1.
int candidatos_actualizar(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{
    // Medimos el tiempo en el momento de entrar a la función
    uint8_t i, j;

    int celdas_vacias = 0;
    
    long tiempoAntes = clock_gettime();
    long tiempoDespues;
    // "Borrar" todos los candidatos, insertando todos para todas las celdas
    for (i = 0; i< NUM_FILAS; i++) {
        for (j = 0; j < NUM_FILAS; j++) {
            celda_inicializar(&cuadricula[i][j]);
        }
    }

    // Propagar los candidatos
    for (i = 0; i< NUM_FILAS; i++){ // Para cada fila
        for(j = 0; j < NUM_FILAS; j++){ // Para cada columna

            if (celda_leer_valor(cuadricula[i][j]) != 0){ // Es una celda con valor inicial, la propagamos borrándola como candidata para su fila, columna y región
                candidatos_propagar(cuadricula, i, j);
            }
            else{
                celdas_vacias++;
            }
        }
    }
    
    // Calculamos el tiempo en este momento y acumulamos la diferencia en el tiempo total de ejecución de la función
    tiempoDespues = clock_gettime();
    tiempoTotalMicrosegundos += tiempoDespues - tiempoAntes;
   
    return celdas_vacias;
}

// Pre: --
// Post: Añade una fila del sudoku sin símbolos al buffer a partir de indice
//       y devuelve el siguiente a escribir
int anyadir_fila(char buffer[], int indice) {
    int i;
    for (i = indice; i < indice + 55;){
        buffer[i++] = '+';
        buffer[i++] = ' ';
        buffer[i++] = '-';
        buffer[i++] = '-';
        buffer[i++] = '-';
        buffer[i++] = ' ';
        
        if (i == indice + 18 || i == indice + 38){
            buffer[i++] = '+';
            buffer[i++] = ' ';
        }
    }  
    buffer[i++] = '+';
    buffer[i++] = '\n';
    return i;
}


// Pre: --
// Post: Añade una fila del sudoku con símbolos para la fila dada al buffer 
//       a partir de indice y devuelve el siguiente a escribir
int anyadir_fila_con_elementos(char buffer[], int indice, int fila) {
    int columna;
    int i = indice;
    buffer[i++] = '|';

    for (columna = 0; columna < 9; columna++) { 
        buffer[i++] = ' ';
        if (esError(&cuadricula[fila][columna])){
            buffer[i++] = celda_leer_valor(cuadricula[fila][columna]) + '0';
            buffer[i++] = ' ';
            buffer[i++] = 'E';
            desmarcar_error(&cuadricula[fila][columna]);
        }
        else if (esPista(&cuadricula[fila][columna])){
            buffer[i++] = celda_leer_valor(cuadricula[fila][columna]) + '0';
            buffer[i++] = ' ';
            buffer[i++] = 'P';
        }
        else if (celda_leer_valor(cuadricula[fila][columna]) != 0){
            buffer[i++] = ' ';
            buffer[i++] = celda_leer_valor(cuadricula[fila][columna]) + '0';
            buffer[i++] = ' ';
        }
        else{
            buffer[i++] = ' ';
            buffer[i++] = ' ';
            buffer[i++] = ' ';
        }
        buffer[i++] = ' ';
        buffer[i++] = '|';
        
        if (columna == 2 || columna == 5){
            buffer[i++] = ' ';
            buffer[i++] = '|';
        }
    }
    buffer[i++] = '\n';
    return i;
}

// Pre: valor está codificado en one-hot encoding
// Post: Convierte un valor codificado en one-hot encoding a char
//       (Usada para la máscara de candidatos)
char one_hot_a_char(uint32_t valor) {
       
    switch (valor) { 
        case 1:
            return '1';
        case 2:
            return '2';
        case 4:
            return '3';
        case 8:
            return '4';
        case 16:
            return '5';
        case 32:
            return '6';
        case 64:
            return '7';
        case 128:
            return '8';
        case 256:
            return '9';
        default:
            return '0';
    }
    
}

// Pre: --
// Post: Dibuja la lista de candidatos para cada fila y columna a
//       a partir de indice y devuelve el siguiente a escribir
int dibujar_candidatos(char buffer[], int indice) {
    int fila;
    int columna; 
    int i = indice;  
    int j = 0; 
    char candidato;
    uint32_t candidatos;

    buffer[i++] = 'F'; buffer[i++] = ' '; buffer[i++] = 'C';
    buffer[i++] = '\t'; buffer[i++] = 'C'; buffer[i++] = 'a';
    buffer[i++] = 'n'; buffer[i++] = 'd'; buffer[i++] = 'i';
    buffer[i++] = 'd'; buffer[i++] = 'a'; buffer[i++] = 't';
    buffer[i++] = 'o'; buffer[i++] = 's'; buffer[i++] = '\n';
    
    for (fila = 0; fila < 9; fila++) {
        for (columna = 0; columna < 9; columna++) {
            if (!esPista(&cuadricula[fila][columna])) {
                candidatos = celda_leer_candidatos(cuadricula[fila][columna]);
                if (candidatos > 0) { // Tiene candidatos
                    // Para cada uno de los 7 bits
                    buffer[i++] = fila + '0';    
                    buffer[i++] = ' ';
                    buffer[i++] = columna + '0'; 
                    buffer[i++] = '\t'; 
                    for (j = 0; j < 9; j ++) {
                        candidato = one_hot_a_char(candidatos & (1<<j));
                        if (candidato != '0') {
                            buffer[i++] = candidato;
                            buffer[i++] = ',';
                            buffer[i++] = ' ';
                        }
                    }
                    i -= 2; // Elimina ", " del último candidato
                    buffer[i++] = '\n';
                }
            }
        }
        buffer[i++] = '\n';
    }
    buffer[i++] = '\n';
    return i;
}


/////////////////////////////////////////////////////////////////
// Funciones exportadas
/////////////////////////////////////////////////////////////////

// Pre: --
// Post: Escribe en un valor en la cuadrícula y fila/columna indicadas. 
// Si es una escritura out-of-bounds o sobre una pista, se genera un error para el planificador. 
void escribir_valor(uint8_t fila, uint8_t columna, uint8_t valor) {
    if (fila < NUM_FILAS && columna < NUM_FILAS) {
        if (esPista(&cuadricula[fila][columna]) == 0) { // Si la celda no es una pista
            // Si el valor a introducir no es 0 y no está entre los posibles candidatos
            if (valor != 0 && ! valor_en_candidatos(cuadricula[fila][columna], valor)){ 
                marcar_error(&cuadricula[fila][columna]); //Ponemos el bit de error de la casilla
                cola_guardar_eventos(ERROR, ERROR_ESCRIBIR_NO_CANDIDATO); 
            }
            else {
                desmarcar_error(&cuadricula[fila][columna]); // Quitamos el bit de error de la casilla
                celda_poner_valor(&cuadricula[fila][columna], valor);
                cola_guardar_eventos(JUGADA_VALIDA, 0);
                candidatos_actualizar(cuadricula);
            }
        } 
        else { // error por ser una pista
            if (valor != 0){
                cola_guardar_eventos(ERROR, ERROR_ESCRIBIR_PISTA_INICIAL); 
            }
            else {
                cola_guardar_eventos(ERROR, ERROR_BORRAR_PISTA_INICIAL);
            }

            marcar_error(&cuadricula[fila][columna]); // Marca el bit de error de la casilla
        }
    } else {
        cola_guardar_eventos(ERROR, ERROR_ESCRIBIR_FUERA_DE_TABLERO); 
    }
}

// Pre: --
// Post: Reinicia los valores de las celdas del tablero que no fueran pistas.
void reiniciar_tablero(void) {
    int i; int j;

    for (i = 0; i< NUM_FILAS; i++){ // Para cada fila
        for(j = 0; j < NUM_FILAS; j++){ // Para cada columna
             if (esPista(&cuadricula[i][j]) == 0){ // No es una pista, borramos su valor
                celda_poner_valor(&cuadricula[i][j], 0);
            }
        }
    }

    // En candidatos actualizar se borrarán y reinicializarán todos los candidatos
    candidatos_actualizar(cuadricula);
}

// Pre: --
// Post: Crea una representación en texto del tablero del juego y la lista
//       de candidatos, y se la envía a gestor_IO para que la trate
void dibujar_tablero(void) {
    char buffer[MAX_CARACTERES];
    int fila;   
    int indice = 0;
    indice = anyadir_fila(buffer, indice); // Se añade una fila de separación
    
    for (fila = 0; fila < 9; fila ++) { 
        indice = anyadir_fila_con_elementos(buffer, indice, fila); // Se añade una fila de numeros
        indice = anyadir_fila(buffer, indice);                     // Se añade una fila de separación
        if (fila == 2 || fila == 5){ // Si es la fila 2 o 5 se pone una separación de región
            indice = anyadir_fila(buffer, indice);                  // Se añade una fila de separación extra
        }
    }
    
    indice = dibujar_candidatos(buffer, indice); // Dibujamos los candidatos
    
    buffer[indice++] = '\n'; buffer[indice++] = '\n'; buffer[indice++] = '\n'; // Añadimos saltos de linea 
    buffer[indice] = '\0';   // Final de buffer
    mostrarBuffer(buffer);
}

// Pre: --
// Post: Crea una representación en texto del tutorial del juego
//       y se la envía a gestor_IO para que lo trate
void dibujar_tutorial(void) {
    char buffer[] = "---------------------------------------------------- TUTORIAL---------------------------------------------------\n \
Para iniciar una nueva partida: #NEW! \n \
Para reiniciar la partida actual: #RST! \n \
Para introducir una jugada en el tablero: #FCVS! \n \
    F -> Fila a modificar \n \
    C -> Columna a modificar \n \
    V -> Valor a introducir \n \
    S -> Checksum = (F+C+V)%8 \n \
\n \
Al introducir una jugada, el led parpadeara lentamente y se tendran 3 segundos para confirmar o cancelarla con el boton.\n \
\n \
Al confirmar una jugada, el led parpadeara y se mostrara el tablero de juego actualizado: \n \
    Si el led parpadea una vez     -> Se ha escrito el valor correctamente \n \
    Si el led parpadea rapidamente -> Se ha cometido un error (escritura en pista o de un valor no candidato) \n \
\n \
Leyenda de simbolos del tablero: \n \
    [1-9] -> Valor de una casilla \n \
        P -> La casilla es una pista \n \
        E -> La casilla contiene un error (escritura en pista o de un valor no candidato) \n \
    Si la casilla no contiene P o E, es una casilla a modificar que no contiene pistas. \n \n \
Para confirmar la jugada una vez terminado el comando pulse el pin 15 del gpio (boton derecho) \n \
Si en su lugar prefieres cancelar el comando pulse el pin 16 (boton izquierdo)\n \
\n \
Debajo del tablero encontraras los candidatos de posibles valores para cada una de las casillas a rellenar, \n \
indicando a la izquierda los indices de la fila y columna respectivamente. \n \
\n \
Disfruta del juego! >:)\n \
Copyright 2021 Laura Gonzalez y Samuel Garcia. Todos los derechos reservados.\n\n\n\0";

    mostrarBuffer(buffer);
}

// Pre: auxData contiene fila, columna y valor codificados como enteros sin signo
//      de 8 bits, del MSB al LSB en dicho orden
// Post: Trata una jugada de escritura de valor
void tratar_jugada(uint32_t auxData){
    uint8_t fila = (auxData & 0xFF0000) >> 16;
    uint8_t columna = (auxData & 0xFF00) >> 8;
    uint8_t valor = auxData & 0xFF;
    // Si al escribir un dato se ha seleccionado valor 0 en (0,0), se termina el juego
    if (fila == 0 && columna == 0 && valor == 0){
        cola_guardar_eventos(FIN_JUEGO, 0);
    }
    else{ // Si no, se escribe el valor en el tablero y se muestra el tablero por la UART
        escribir_valor(fila, columna, valor);
        dibujar_tablero();
    }   
}

// Invierte un string
void reverse(char str[], int start, int length)
{
    char intermedio;
    int end = length -1;
    while (start < end)
    {
        intermedio = *(str+start);
        *(str+start) = *(str+end);
        *(str+end) = intermedio;

        start++;
        end--;
    }
}
 
// Implementacion de (u)itoa()
// Adaptado de https://tutorialspoint.dev/language/c/implement-itoa
int itoa(uint32_t num, char* str, int base, int indice) {
    int i = indice;
 
    /* Handle 0 explicitly, otherwise empty string is printed for 0 */
    if (num == 0){
        str[i++] = '0';
        return i;
    }
 
    // Process individual digits
    while (num != 0){
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
 
    // Reverse the string
    reverse(str, indice, i);
 
    return i;
}

// Pre: --
// Post: Crea una representación en texto del tiempo de juego
//       y procesado de candidatos_actualizar, y se la envía a 
//       gestor_IO para que lo trate
void dibujar_tiempo(uint8_t minutos, uint8_t segundos) {
    int i = 0;
    char buffer[MAX_CARACTERES];
    // El mensaje que se muestra es:
    // Tiempo de partida: {minutos} min, {segundos} seg
    // Tiempo de candidatos actualizar {tiempoTotalMicrosegundos} useg
    // Razon de fin de partida: terminar partida manualmente

    buffer[i++] = 'T';buffer[i++] = 'i';buffer[i++] = 'e';
    buffer[i++] = 'm';buffer[i++] = 'p';buffer[i++] = 'o';
    buffer[i++] = ' ';buffer[i++] = 'd';buffer[i++] = 'e';
    buffer[i++] = ' ';buffer[i++] = 'p';buffer[i++] = 'a';
    buffer[i++] = 'r'; buffer[i++] = 't';buffer[i++] = 'i';
    buffer[i++] = 'd';buffer[i++] = 'a';buffer[i++] = ':';
    buffer[i++] = ' ';
    i = itoa(minutos, buffer, 10, i);
    buffer[i++] = 'm';buffer[i++] = 'i';buffer[i++] = 'n';
    buffer[i++] = ' ';
    i = itoa(segundos, buffer, 10, i);
    buffer[i++] = 's';buffer[i++] = 'e';buffer[i++] = 'g';
    buffer[i++] = '.';
    buffer[i++] = '\n';

    buffer[i++] = 'T';buffer[i++] = 'i';buffer[i++] = 'e';
    buffer[i++] = 'm';buffer[i++] = 'p';buffer[i++] = 'o';
    buffer[i++] = ' ';buffer[i++] = 'd';buffer[i++] = 'e';
    buffer[i++] = ' ';buffer[i++] = 'c';buffer[i++] = 'a';
    buffer[i++] = 'n';buffer[i++] = 'd';buffer[i++] = 'i';
    buffer[i++] = 'd';buffer[i++] = 'a';buffer[i++] = 't';
    buffer[i++] = 'o';buffer[i++] = 's';buffer[i++] = ' ';
    buffer[i++] = 'a';buffer[i++] = 'c';buffer[i++] = 't';
    buffer[i++] = 'u';buffer[i++] = 'a';buffer[i++] = 'l';
    buffer[i++] = 'i';buffer[i++] = 'z';buffer[i++] = 'a';
    buffer[i++] = 'r';buffer[i++] = ':';buffer[i++] = ' ';
    i = itoa(tiempoTotalMicrosegundos, buffer, 10, i);
    buffer[i++] = 'u';buffer[i++] = 's';buffer[i++] = 'e';
    buffer[i++] = 'g';buffer[i++] = '.';buffer[i++] = '\n';
    
    buffer[i++] = 'R'; buffer[i++] = 'a'; buffer[i++] = 'z'; 
    buffer[i++] = 'o'; buffer[i++] = 'n'; buffer[i++] = ' ';
    buffer[i++] = 'd'; buffer[i++] = 'e'; buffer[i++] = ' '; 
    buffer[i++] = 'f'; buffer[i++] = 'i'; buffer[i++] = 'n';
    buffer[i++] = ' '; buffer[i++] = 'd'; buffer[i++] = 'e'; 
    buffer[i++] = 'p'; buffer[i++] = 'a'; buffer[i++] = 'r'; 
    buffer[i++] = 't'; buffer[i++] = 'i'; buffer[i++] = 'd'; 
    buffer[i++] = 'a'; buffer[i++] = ':'; buffer[i++] = ' ';

    buffer[i++] = 'T'; buffer[i++] = 'e'; buffer[i++] = 'r'; 
    buffer[i++] = 'm'; buffer[i++] = 'i'; buffer[i++] = 'n'; 
    buffer[i++] = 'a'; buffer[i++] = 'r'; buffer[i++] = ' '; 
    buffer[i++] = 'p'; buffer[i++] = 'a'; buffer[i++] = 'r';
    buffer[i++] = 't'; buffer[i++] = 'i'; buffer[i++] = 'd'; 
    buffer[i++] = 'a'; buffer[i++] = ' '; buffer[i++] = 'm'; 
    buffer[i++] = 'a'; buffer[i++] = 'n'; buffer[i++] = 'u'; 
    buffer[i++] = 'a'; buffer[i++] = 'l'; buffer[i++] = 'm';
    buffer[i++] = 'e'; buffer[i++] = 'n'; buffer[i++] = 't'; 
    buffer[i++] = 'e';
    buffer[i++] = '\n';buffer[i++] = '\n';buffer[i++] = '\n'; // Se añade una separación
    
    buffer[i++] = '\0'; // Fin de buffer
    mostrarBuffer(buffer);
}

