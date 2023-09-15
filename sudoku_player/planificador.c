#include "planificador.h"
#include "GestorAlarmas.h"
#include "eventos.h"
#include "Botones_eint1_eint2.h"
#include "GestorEnergia.h"
#include "Gestor_IO.h"
#include "cola.h"
#include "gestorSudoku.h"
#include "GestorModulos.h"
#include "comandos.h"
#include "gestor_WD.h"
#include "gestor_RTC.h"

void reiniciar_partida() {
	// Nos aseguramos de limpiar interrupciones externas pendientes,
	// ya que han perdido al sentido al haber reiniciado el juego
	button_clear_nueva_pulsacion_1();
	button_clear_nueva_pulsacion_2();
	habilitar_interrupciones_1();
	habilitar_interrupciones_2();
	
	// Alimenta al watchdog y reinicia la alarma de powerdown como si
	// se hubieran reiniciado dichos módulos
	WD_feed();
	resetear_alarma_power_down();
	
	// Reinicia el conteo del tiempo de juego
	RTC_reset();

	// Cuando se procesen todos los demás eventos, si hubiera, se iniciará un nuevo juego
	cola_guardar_eventos(INICIAR_JUEGO, 0);
}

// Pre: Todos los módulos están inicializados correctamente.
// Post: Atiende a todos los eventos generados durante la ejecución del juego por el
// resto de módulos, y actúa acordemente coordinando entre los módulos.
void ejecutar_planificador(void) {
	uint8_t vuelta_de_power_down = 0; 	// Indicador de que hemos vuelto del estado de power_down
	uint8_t nueva_partida = 1; 			// Indicador de que es una nueva partida (al encender el chip)

	EventoCola evento; 	
	
	// Empieza la máquina de estados con un evento de INICIAR_JUEGO
	cola_guardar_eventos(INICIAR_JUEGO, 0);
	
    while(1) {
		if(nuevos_eventos()) {
			WD_feed(); // Alimentamos al watchdog cada vez que se comprueba un evento a la cola
			cola_leer_evento(&evento); // Lee un nuevo evento de la cola y actúa acordemente según el ID de evento
			
			switch(evento.id_evento) {
				// Inicio del juego desde 0, realimentado desde el planificador al iniciar o reiniciar la partida
				case INICIAR_JUEGO:
					reiniciar_tablero();
					dibujar_tutorial();
					nueva_partida = 1;
					break;

				// Se ha leído un carácter de la línea serie
				case CARACTER_LEIDO:
					tratarEventoIO(evento); // Se redirige a Gestor_IO para que lo trate
					break;

				// Evento de latido de modo idle
				case LATIDO_MODO_IDLE:
					tratarEventoIO(evento);
					break;

				// Entra en modo power-down directamente, sabemos que no hemos conseguido aplazar la alarma si llega aquí
				case DORMIR_PROCESADOR: 
					vuelta_de_power_down = 1;
					modo_power_down();
					break;

				// Cambia el timestamp de todas las alarmas de GestorAlarmas, y las dispara, cancela o reprograma si aplica
				case EVENTO_TEMPORIZADOR_PERIODICO:
					disminuir_y_lanzar_alarmas(); 
					break;

				// El usuario ha introducido un nuevo comando de juego, válido o no
				case NUEVO_COMANDO:
					if (! nueva_partida) { // Solo muestra comandos si no es una nueva partida (y está en el tutorial);
						mostrar_comando();
						cola_guardar_eventos(JUGADA_PENDIENTE, 0); // Se retroalimenta con un evento de comando o jugada pendiente
					}
					
					break;

				// Se ha solicitado prohibir la aplicación de nuevos comandos (por timeout de respuesta de confirmación de un nuevo comando)
				case PROHIBIR_APLICAR_COMANDOS:
					prohibirAplicarComandos();
					cancelarComando();
					break;

				// Evento de eint1_ISR para confirmar un comando introducido
				case CONFIRMAR_COMANDO:
					resetear_alarma_power_down(); // Se resetea la alarma de power-down al haber realizado una acción

					if (!vuelta_de_power_down && !nueva_partida) {
						if (comandosAplicables()) { // Si no ha pasado el timeout de confirmar o cancelar un nuevo comando
							tratar_comando(); // Manda al gestor de comandos leer el comando de Gestor_IO para comprobar su validez y acción a realizar
							indicarComandoLeido();
						}	
					} 
					else if (nueva_partida) { // Si acaba de empezar el juego, y solamente se quiere empezar la partida
						reiniciar_tablero();
						dibujar_tablero();
						nueva_partida = 0;

						// Prohibe aplicar comandos si ya había leído alguno, esto hace que al escribir 
						// el comando de nueva jugada se acepte, y que si se introduce algún otro comando 
						// válido o no, no tenga efectos
						prohibirAplicarComandos();
						cancelarComando();
					}
					else { // No se hace nada, y limpia la flag de vuelta de power-down
						vuelta_de_power_down = 0;
					}
					break;

				// Evento de eint2_ISR para cancelar la entrada de un comando
				case CANCELAR_COMANDO:
					resetear_alarma_power_down(); // Se resetea la alarma de power-down al haber realizado una acción

					if (!vuelta_de_power_down && !nueva_partida) {
						if (comandosAplicables()) { // Si no ha pasado el timeout de confirmar o cancelar un nuevo comando
							cancelarComando();
						}
					} 
					else if (nueva_partida) { // Si acaba de empezar el juego, y solamente se quiere empezar la partida
						reiniciar_tablero();
						dibujar_tablero();
						nueva_partida = 0;

						// Prohibe aplicar comandos si ya había leído alguno, esto hace que al escribir 
						// el comando de nueva jugada se acepte, y que si se introduce algún otro comando 
						// válido o no, no tenga efectos
						prohibirAplicarComandos();
						cancelarComando();
					}
					else { // No se hace nada, y limpia la flag de vuelta de power-down
						vuelta_de_power_down = 0;
					}
					break;

				// El jugador ha realizado una nueva jugada válida tras confirmar un comando
				case NUEVA_JUGADA:
					resetear_alarma_power_down(); // Se resetea la alarma de power-down al haber realizado una acción
					tratar_jugada(evento.auxData); // Indica al gestor de sudoku que trate la jugada
					break;

				// Hay que mostrar que hay una jugada pendiente
				case JUGADA_PENDIENTE:
					tratarEventoIO(evento);
					break;

				// Hay que hacer parpadear la salida de que hay una jugada pendiente
				case PARPADEAR_SALIDA_JUGADA_PENDIENTE:
					tratarEventoIO(evento);
					break;

				// Evento de activación de la salida de jugada válida
				case JUGADA_VALIDA:
					tratarEventoIO(evento);
					break;

				// Evento de desactivación de la salida de jugada válida
				case DESACTIVAR_SALIDA_JUGADA_VALIDA:
					tratarEventoIO(evento);
					break;

				// Hay que mostrar que ha habido un error al realizar alguna acción en el juego
				case ERROR:
					tratarEventoIO(evento); // Se indica al gestor IO que ha ocurrido un error para que lo muestre
					break;

				// Hay que hacer parpadear la salida de que se ha cometido un error en el juego
				case PARPADEAR_SALIDA_ERROR:
					tratarEventoIO(evento);
					break;	

				// Ha habido un overflow en un módulo
				case OVERFLOW:
					// Indica Gestor_IO que avise al usuario
					tratarEventoIO(evento);
					
					// Y entra en un bucle infinito, que provocará que el Watchdog se active
					while (1) {} 
					//break;

				// Evento de reinicio de partida
				// Actualmente generado tras introducir el comando #NEW! o introducir el valor 0 en (0,0) del tablero del Sudoku
				case REINICIAR_PARTIDA:
					vuelta_de_power_down = 1;
					nueva_partida = 0;
					reiniciar_partida();
					break;

				// Evento de terminación de partida
				// Actualmente generado tras introducir el comando #RST!
				case FIN_JUEGO:
					dibujar_tiempo(RTC_leer_minutos(), RTC_leer_segundos());
					RTC_reset();
					break;

				// Evento de comprobación de si el botón de eint1 sigue activo     
                case EVENTO_COMPROBAR_PULSACION_1:
					if (!button_nueva_pulsacion_1()) { // Si el botón no está pulsado, entonces habilitamos eint1
						habilitar_interrupciones_1();
					} else {
						button_clear_nueva_pulsacion_1(); // Limpiamos la flag de interrupciones de eint1, y comprobamos en el siguiente evento si sigue activa
					}
                    break;

				// Evento de comprobación de si el botón de eint2 sigue activo      
                case EVENTO_COMPROBAR_PULSACION_2:
					if (!button_nueva_pulsacion_2()) { // Si el botón no está pulsado, entonces habilitamos eint2
						habilitar_interrupciones_2();
					} else{
						button_clear_nueva_pulsacion_2(); // Limpiamos la flag de interrupciones de eint2, y comprobamos en el siguiente evento si sigue activa
					}
                    break;
			}
		} 
		else { // Si no hay nuevos eventos, se pasa el procesador a modo idle hasta que llegue cualquier interrupción
			modo_idle();
		}
	}	
}
