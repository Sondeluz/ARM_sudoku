#ifndef BOTONES_EINT1_EINT2_H
#define BOTONES_EINT1_EINT2_H

// Devuelve 1 si el botón 1 está siendo presionado
unsigned int button_nueva_pulsacion_1(void);

// Devuelve 2 si el botón 1 está siendo presionado
unsigned int button_nueva_pulsacion_2(void);

// Limpia las interrupciones pendientes para el botón 1
void button_clear_nueva_pulsacion_1(void);

// Limpia las interrupciones pendientes para el botón 2
void button_clear_nueva_pulsacion_2(void);

// Inicializa las interrupciones para el botón 1
void eint1_init (void);

// Inicializa las interrupciones para el botón 2
void eint2_init (void);

void habilitar_interrupciones_1(void);
void habilitar_interrupciones_2(void);

#endif // BOTONES_EINT1_EINT2_H
