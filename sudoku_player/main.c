#include "planificador.h"
#include "GestorModulos.h"
#include "Botones_eint1_eint2.h"

// Para probar RTC y WD
/*
	#include "gestor_WD.h"
	#include "gestor_RTC.h"
*/

int main (void) {
	// Inicializa todos los módulos	
	inicializar_modulos_sin_alarmas();
	inicializar_modulos_con_alarmas();

	// Habilita los botones (interrupciones externas) a partir de este momento
	habilitar_interrupciones_1();
	habilitar_interrupciones_2();
	
	// Pasa el control al planificador
	ejecutar_planificador();


	// Para probar RTC y WD, comentar el resto del código y descomentar los includes
	/*
		RTC_init();
		WD_init(10); // WD iniciado y alimentado a partir de este punto

		while (RTC_leer_segundos() < 5) { // Espera 5 segundos
		}

		while (1) {} // En 5 segundos saltará el WD al no ser alimentado
	*/
}
