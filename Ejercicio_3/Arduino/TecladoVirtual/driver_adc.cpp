#include "driver_adc.h"
#include "Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile int readFlag;
volatile int analogValue;
miestruct* me;

int adc_init(miestruct *cfg)
{
  if(cfg->canal >= 8 || cfg->canal < 0)
    return 0;
  
  me = cfg;   

  cli();
	// clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  ADMUX &= ~(1<<ADLAR);
  
  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  ADMUX |= (1<<REFS0);
  
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog
  // input
  ADMUX &= ~((1<<MUX0) | (1<<MUX1) | (1<<MUX2) | (1<<MUX3));
  
  // Set MUX3..0 in ADMUX (0x7C) to read from AD8 (Internal temp)
  // Do not set above 15! You will overrun other parts of ADMUX. A full
  // list of possible inputs is available in Table 24-4 of the ATMega328
  // datasheet

  ADMUX |= cfg->canal;
  // ADMUX |= B00001000; // Binary equivalent
 
  // Set ADEN in ADCSRA (0x7A) to enable the ADC.
  // Note, this instruction takes 12 ADC clocks to execute
  ADCSRA |= (1<<ADEN);
 
  // Set ADATE in ADCSRA (0x7A) to enable auto-triggering.
  ADCSRA |= (1<<ADATE);

  // Set the Prescaler to 128 (16000KHz/128 = 125KHz)
  // Above 200KHz 10-bit results are not reliable.
  ADCSRA |= ((1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2));

  // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  ADCSRA |= (1<<ADIE);
  
  // Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running.
  // This means that as soon as an ADC has finished, the next will be
  // immediately started.
  ADCSRB &= ~((1<<ADTS0) | (1<<ADTS1) | (1<<ADTS2));
 
  // Enable global interrupts
  // AVR macro included in <avr/interrupts.h>, which the Arduino IDE
  // supplies by default.
  sei();
   
  // Kick off the first ADC
  readFlag = 0;
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |= (1<<ADSC);

  sei();
  return 1;
}

void adc_loop()
{
  // Check to see if the value has been updated
  if (readFlag == 1)
  {
    me->analogValue = analogValue;
    me->callback();
   
    readFlag = 0;
  }
}

ISR(ADC_vect)
{

  // Done reading
  readFlag = 1;
 
  // Must read low first
  analogValue = ADCL | (ADCH << 8);
 
  // Not needed because free-running mode is enabled.
  // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
  // ADCSRA |= B01000000;
}
