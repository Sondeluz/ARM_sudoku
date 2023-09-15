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
#endif // CELDA_H
