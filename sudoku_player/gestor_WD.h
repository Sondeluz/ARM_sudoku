#ifndef __GESTOR_WD_H
#define __GESTOR_WD_H

/*
    gestor_WD.h define una interfaz de comunicación
    con el watchdog del chip para configurar un timeout
    y resetearlo con feeds.
*/

// Alimenta al watchdog
void WD_feed(void);

// Inicializa el watchdog con un timeout de sec segundos
void WD_init(int sec);


#endif  // __GESTOR_WD_H
