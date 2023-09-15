#ifndef __GESTOR_ENERGIA_H
#define __GESTOR_ENERGIA_H

// Pre: --
// Post: Activa el modo idle del procesador
void modo_idle (void);

// Pre: --
// Post: Activa el modo power-down del procesador, asegurando que se podrá despertar
// presionando los dos botones (eint1, eint2), y que se restaura la frecuencia original
// del procesador
void modo_power_down (void); 

// Pre: --
// Post: Inicia la alarma de latido en modo idle, que será periódica y solo se desactivará al entrar en
// modo power-down
void iniciar_alarma_latido_modo_idle(void);

// Pre: --
// Post: Inicia la alarma de modo power-down, que solicitará al planificador terminar en 15 segundos, a no ser que 
// llame a resetear_alarma_power_down()
void iniciar_alarma_power_down(void);

// Pre: --
// Post: Inicia la alarma de modo power-down, que solicitará al planificador terminar en 15 segundos, a no ser que 
// llame a resetear_alarma_power_down()
void resetear_alarma_power_down(void);

// Pre: --
// Post: Función externa que apunta a código de Startup.s y reinicializa la frecuencia original del procesador.
extern void Reinicializar_PLL(void);

#endif 
