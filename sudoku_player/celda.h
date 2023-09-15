/* guarda para evitar inclusiones múltiples (include guard) */
#ifndef CELDA_H
#define CELDA_H

#include <inttypes.h>

/* Cada celda se codifica en 16 bits
 * bits [15,7]: los 9 bits más significativos representan el vector de candidatos,
 * si el bit 7 + valor - 1 está a 0, valor es candidato, 1 en caso contrario
 * bit 6: no empleado
 * bit 5: error
 * bit 4: pista
 * bits [3,0]: valor de la celda
 */

enum { BIT_CANDIDATOS = 7 };

typedef uint16_t CELDA;

/* *****************************************************************************
 * elimina el candidato del valor almacenado en la celda indicada */
__inline static void celda_eliminar_candidato(CELDA *celdaptr, uint8_t valor)
{
    *celdaptr = *celdaptr | (0x0001 << (BIT_CANDIDATOS + valor - 1));
}

/* *****************************************************************************
 * inserta el candidato del valor almacenado en la celda indicada */
__inline static void celda_insertar_candidato(CELDA *celdaptr, uint8_t valor)
{
    *celdaptr = *celdaptr | (0x0000 << (BIT_CANDIDATOS + valor - 1));
}

/* *****************************************************************************
 * Marca todos los valores posibles (1-9) como candidatos de la celda indicada */
__inline static void celda_inicializar(CELDA *celdaptr)
{
    //*celdaptr = *celdaptr & 0b0000 0000 0111 1111;
    *celdaptr = *celdaptr & 0x007F; // Aplicamos una máscara
}

/* *****************************************************************************
 * modifica el valor almacenado en la celda indicada */
__inline static void
celda_poner_valor(CELDA *celdaptr, uint8_t val)
{
    *celdaptr = (*celdaptr & 0xFFF0) | (val & 0x000F);
}

/* *****************************************************************************
 * extrae el valor almacenado en los 16 bits de una celda 
   si el valor retornado es 0, la celda está vacía, si no,
   es un valor del 1 al 9 */
__inline static uint8_t
celda_leer_valor(CELDA celda)
{
    return (celda & 0x000F);
}

/* *****************************************************************************
 * Devuelve una máscara de candidatos activos para la celda, en one-hot encoding, indexando desde el valor 1  */
__inline static uint32_t
celda_leer_candidatos(CELDA celda)
{
    return ~(celda & 0xFF80) >> 7; // Leemos bits del 15 al 7 incluídos negándolos, y los rotamos 7 posiciones a la derecha
}

/* *****************************************************************************
Pre: valor > 0
Devuelve valor mayor que 1 si y solo si el valor indicado está dentro de sus posibles candidatos, 0
en caso contrario */
__inline static uint32_t
valor_en_candidatos(CELDA celda, uint8_t valor){
	// Obtiene una máscara con los bits de candidatos, rotados 7 posiciones a la derecha y negados
	uint32_t mascara_candidatos = ~((celda & 0xFF80) >> 7);
	
	 // Aplica una máscara de un 1 sobre el dígito correspondiente al valor en la máscara de candidatos
    return ( mascara_candidatos& (1 << (valor-1)));
}

/* *****************************************************************************
 * Devuelve el bit de pista inicial  */
__inline static uint8_t
es_pista_inicial(CELDA celda)
{
    return (celda & 0x10); // Leemos bits del 15 al 7 incluídos
}

/* *****************************************************************************
 * Marca el bit de error de una celda  */
__inline static void
marcar_error(CELDA *celdaptr)
{
    *celdaptr |= 0x20; // Marca el bit 5
}

/* *****************************************************************************
 * Desmarca el bit de error de una celda  */
__inline static void
desmarcar_error(CELDA *celdaptr)
{
    *celdaptr &= 0xFFFFFFDF; // Desmarca el bit 5
}


/* *****************************************************************************
 * Devuelve el bit de pista de la celda  */
__inline static uint8_t
esPista(CELDA *celdaptr)
{
    return (*celdaptr & 0x10)>>4; // Leemos el bit de pista
}

/* *****************************************************************************
 * Devuelve el bit de error de la celda  */
__inline static uint8_t
esError(CELDA *celdaptr)
{
    return (*celdaptr & 0x20)>>5; // Leemos el bit de error
}

#endif // CELDA_H
