#ifndef DRIVER_TECLADO
#define DRIVER_TECLADO

#define TECLA_RIGHT 0
#define TECLA_UP 1
#define TECLA_DOWN 2
#define TECLA_LEFT 3
#define TECLA_SELECT 4
#define BOTON_A2 5

void teclado_init(int *vKey); //MODIFICACION PARA RECONOCER TECLA CON GALILEO

void teclado_loop();

void key_down_callback(void (*handler)(), int tecla);

void key_up_callback(void (*handler)(), int tecla);

#endif
