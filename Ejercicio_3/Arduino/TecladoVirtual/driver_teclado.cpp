#include "driver_teclado.h"
#include "driver_adc.h"
#include "Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int adc_key_val[5] ={30, 170, 360, 535, 760 };
int NUM_KEYS = 5;
int key = -1, oldKey = -1;
void (*keyup_handler[6])(void);
void (*keydown_handler[6])(void);
int count = 0;
volatile int boton = 0;
int *virtualKey;
miestruct cfg;

void key_down_callback(void (*handler)(), int tecla)
{
  keydown_handler[tecla] = handler;
}

void key_up_callback(void (*handler)(), int tecla)
{
  keyup_handler[tecla] = handler;
}

int get_key(int input)
{
  int k;
  for (k = 0; k < NUM_KEYS; k++)
    if (input < adc_key_val[k])
      return k;

  if (k >= NUM_KEYS)
    k = -1;     // No valid key pressed
  
  return k;
}

void Callback()
{
  key = get_key(cfg.analogValue);
  count = (count + 1) % 2;
  delay(50);
  if(count)
    return;
  if(key!=oldKey)
  {
    if(key >= 0)
      keydown_handler[key]();
    else
      keyup_handler[oldKey]();
    
    oldKey = key;
  }
  //PARA SIMULAR EL APRETADO Y SOLTADO
  if(*virtualKey >= 0)
  {
    keydown_handler[*virtualKey]();
    delay(1000);
    keyup_handler[*virtualKey](); 
    *virtualKey = -1;
  }
}

void teclado_init(int *vKey)
{
  virtualKey = vKey;
  
  cli(); //DESHABILITO LAS INTERRUPCIONES EN LA CONFIGURACION DEL PINCHANGE INTERRUPT
  DDRC &= ~(1 << DDC2);      // PC2 (PCINT10 pin) is now an input
  PCICR |= (1 << PCIE1);     // set PCIE1 to enable PCMSK1 scan
  PCMSK1 |= (1 << PCINT10);   // set PCINT10 to trigger an interrupt on state change
  sei();
  //-----------------------
  cfg.canal = 0;
  cfg.callback = Callback;
  adc_init(&cfg);
  
}

void teclado_loop()
{
  adc_loop();
  if(boton)
  {
    if(PINC & (1 << DDC2))
      keydown_handler[BOTON_A2]();
    else
      keyup_handler[BOTON_A2]();
    boton = 0;
  }
}


ISR (PCINT1_vect)
{
  boton = 1;
}



