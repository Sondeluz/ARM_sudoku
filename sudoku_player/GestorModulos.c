#include "GestorModulos.h"
#include "temporizador.h"
#include "GestorAlarmas.h"
#include "Botones_eint1_eint2.h"
#include "Gestor_pulsacion.h"
#include "GestorEnergia.h"
#include "Gestor_IO.h"
#include "gestor_UART.h"
#include "gestor_RTC.h"
#include "gestor_WD.h"

// Pre: --
// Post: Inicializa los módulos que ponen alarmas en marcha.
void inicializar_modulos_con_alarmas(void) {
	// Inicio de Gestor Pulsacion
	inicializar_gestor_pulsacion(); // Pone dos alarmas cada 100ms para monitorizar pulsaciones

	// Inicio del GPIO y del gestor IO
	gestor_IO_inicializar(); // Pone una alarma cada 200ms de actualizar visualización de la celda
	
	// Inicio de la alarma de power down a 15 segundos, que se reseteará al detectar cambios en las entradas
	iniciar_alarma_power_down();
    iniciar_alarma_latido_modo_idle(); // Cada 250ms
}

// Pre: Los módulos no están en marcha, ya que puede generar problemas en la cola de eventos.
// Post: Inicializa los módulos que no ponen alarmas en marcha.
void inicializar_modulos_sin_alarmas(void) {
	// Inicio de temporizadores
	temporizador_iniciar();
	temporizador_empezar();
	temporizador_periodico(1); // Lo ponemos a 1
	inicializar_cola();

	// Inicio de interrupciones externas
	eint1_init();
	eint2_init();

	// Inicia la línea serie
	UART_init(); 

	// Inicia el reloj en tiempo real
	RTC_init();

	// Inicia el watchdog con 20 segundos de espera
	WD_init(20); 
}
