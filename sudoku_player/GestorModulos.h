#ifndef __GESTOR_MODULOS_H
#define __GESTOR_MODULOS_H

// Pre: --
// Post: Inicializa los módulos que ponen alarmas en marcha.
void inicializar_modulos_con_alarmas(void);

// Pre: Los módulos no están en marcha, ya que puede generar problemas en la cola de eventos.
// Post: Inicializa los módulos que no ponen alarmas en marcha.
void inicializar_modulos_sin_alarmas(void);

#endif  // __GESTOR_MODULOS_H
