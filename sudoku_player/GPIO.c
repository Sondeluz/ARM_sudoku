#include "GPIO.h"
#include <LPC210x.H>                       /* LPC210x definitions */


// Pre: --
// Post: Inicia el GPIO a valores seguros (todos los pines como salida)
void GPIO_iniciar(void){
    // Marca todos los pines como salidas
    IODIR 		= 0xFFFFFFFF;
    
    // Y limpia sus valores
	IOCLR 		= 0xFFFFFFFF;
} 

// Pre: --
// Post: Devuelve num_bits valores del GPIO desde bit_inicial
uint32_t GPIO_leer(uint32_t bit_inicial, uint32_t num_bits){
    int i;

    // Crea una máscara de 1s sobre los bits a leer 
	uint32_t mascara = 0x00000000; 
    for (i = 0; i < num_bits; i++) mascara |= (1 << (bit_inicial + i));

    // Hacemos un LSR de la máscara con IOPIN bit_inicial veces para devolver el 
    // valor comprendido entre los bits [bit_inicial, num_bits-1], desde el bit 0
    return (IOPIN & mascara) >> bit_inicial; 
}

// Pre: --
// Post: Escribe valor en los num_bits valores del GPIO desde bit_inicial. Si valor 
// no puede representarse en los bits indicados se escribirá los num_bits menos significativos 
// a partir del inicial
void GPIO_escribir(uint32_t bit_inicial, uint32_t num_bits, uint32_t valor) {
    int i;

    // Crea una máscara de tantos 1s como se bits vayan a escribir, independientemente del
    // valor indicado. Esto evitará overflows.
    uint32_t mascara = 0x00000000;
    for (i = 0; i < num_bits; i++) {
        mascara |= (1 << i);
    }
    
    // Se aplica la máscara a valor para coger solo los bits menos significativos hasta num_bit,
    // y se mueve a la posicion donde se quiere escribir en el GPIO con un shift a la izquierda 
    // bit_inicial veces
    valor = (valor & mascara) << bit_inicial;

    // Escribir un valor 0 en IOSET no hace efecto, así si el valor es 0 hay que escribirlo en IOCLR
    if (valor != 0) {
        IOCLR |= (mascara << bit_inicial); // Se borran los bits que hubiera escritos en la posición anteriormente        
        IOSET |= valor; // Y se escriben utilizando IOSET
    } else {	
        IOCLR |= (mascara << bit_inicial); // Se borran los bits que hubiera escritos en la posición anteriormente    
    }
}

// Pre: --
// Post: Marca num_bits como entrada desde bit_inicial
void GPIO_marcar_entrada(uint32_t bit_inicial, uint32_t num_bits){
	int i;
    // Se crea una máscara de 1s en las posiciones de los bits a marcar como entrada
    uint32_t mascara = 0x00000000;
    for (i = 0; i < num_bits; i++) mascara |= (1 << (bit_inicial + i)); // Uno de los num_bits ya es el bit inicial

    IODIR &= ~mascara; // Se escribe en IODIR, negando la máscara con un AND
}

// Pre: --
// Post: Marca num_bits como salida desde bit_inicial
void GPIO_marcar_salida(uint32_t bit_inicial, uint32_t num_bits){
    int i;
    // Se crea una máscara de 1s en las posiciones de los bits a marcar como entrada
    uint32_t mascara = 0x00000000;
    for (i = 0; i < num_bits; i++) mascara |= (1 << (bit_inicial + i)); // Uno de los num_bits ya es el bit inicial
    
    IODIR |= mascara; // Se escribe en IODIR la máscara directamente con un OR
}
