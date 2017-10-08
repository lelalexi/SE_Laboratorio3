#include "Arduino.h"
#include <Wire.h>

#define OBTENER_LUX 0
#define OBTENER_MIN 1
#define OBTENER_MAX  2
#define OBTENER_PROM 3
#define OBTENER_TODO 4
#define RESPONDER_LUX 5
#define RESPONDER_MIN 6
#define RESPONDER_MAX 7
#define RESPONDER_PROM 8
#define RESPONDER_TODO 9

int tabla[2][9]={{92,41,24,16,10,7,5,3,1},{0,1,3,6,10,15,35,80,100}};
float lux;
float luxVals[200];
float actualLux = 0.0;
int index = 0;
int cant = 0;
int indexMin = 0, indexMax = 0;
char received[32];
char reply[32];
volatile int count = 0;
volatile int timer = 1;
 
void setup()
{
  cli();          // disable global interrupts
  TCCR2A = 0;     // set entire TCCR1A register to 0
  TCCR2B = 0;     // same for TCCR1B
  TCNT2  = 0;
  // set compare match register to desired timer count:
  OCR2A = 155;
  //0.99
  // turn on CTC mode:
  TCCR2A |= (1 << WGM21);
   TCCR2B &= ~(1 << WGM22);
  // Set CS20 and CS21 and CS22 bits for 1024 prescaler:
  TCCR2B |= (1 << CS20);
  TCCR2B |= (1 << CS21);
  TCCR2B |= (1 << CS22);
  // enable timer compare interrupt:
  TIMSK2 |= (1 << OCIE2A);
  sei();          // enable global interrupts    
  //Serial.begin(9600);
  //Serial.println("hola");
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop()
{
  getLuxVals();
}

void requestEvent() 
{
  Wire.write(reply);
}

void receiveEvent(int howMany)
{
  int recvInProgress = 0;
  int i = 0;
  while(Wire.available())
  {
    char c = Wire.read(); // receive byte as a character
    if (recvInProgress)
    {
      if (c != ')') 
      {
        received[i] = c;
        i++;     
      }
      else
      {
        received[i] = '\0'; // terminate the string
        recvInProgress = 0;
        i = 0;
      }
    }
    else
      if (c == '(')
      {
        recvInProgress = 1;
      }
  }
  parseAndReply();
}

void parseAndReply()
{
  char delim[2] = "$";
  char *str = received;
  char *token;
  int tipo;
  int tam;
  char *payload;
  float prom;
  char todo[32];

  token = strtok(str, delim);
  tipo = atoi(token);
  //Serial.print(tipo);
  //token = strtok(NULL, delim);
  //tam = atoi(token);
  //token = strtok(NULL, delim);
  //payload = token;
  
  char parsedLux[10], parsedMin[10], parsedMax[10], parsedProm[10];
  String strLux, strMin, strMax, strProm;
  int sizeReply;
  switch(tipo)
  {
    case OBTENER_LUX:
      strLux = String(actualLux, 2);
      strLux.toCharArray(parsedLux, 10);
      sizeReply = strlen(parsedLux) + 7;
      sprintf(reply, "(%d$%d$%s)", RESPONDER_LUX, sizeReply, parsedLux);
      break;
    case OBTENER_MIN:
      minYMax();
      strMin = String(luxVals[indexMin], 2);
      strMin.toCharArray(parsedMin, 10);
      sizeReply = strlen(parsedMin) + 7;
      sprintf(reply, "(%d$%d$%s)", RESPONDER_MIN, sizeReply, parsedMin);
      break;
    case OBTENER_MAX:
      minYMax();
      strMax = String(luxVals[indexMax], 2);
      strMax.toCharArray(parsedMax, 10);
      sizeReply = strlen(parsedMax) + 7;
      sprintf(reply, "(%d$%d$%s)", RESPONDER_MAX, sizeReply, parsedMax);
      break;
    case OBTENER_PROM:
      prom = promedio();
      strProm = String(prom, 2);
      strProm.toCharArray(parsedProm, 10);
      sizeReply = strlen(parsedProm) + 7;
      sprintf(reply, "(%d$%d$%s)", RESPONDER_PROM, sizeReply, parsedProm);
      break;
    case OBTENER_TODO:
      minYMax();
      prom = promedio();
      strLux = String(actualLux, 2);
      strLux.toCharArray(parsedLux, 10);
      strMin = String(luxVals[indexMin], 2);
      strMin.toCharArray(parsedMin, 10);
      strMax = String(luxVals[indexMax], 2);
      strMax.toCharArray(parsedMax, 10);
      strProm = String(prom, 2);
      strProm.toCharArray(parsedProm, 10);
      sizeReply = strlen(parsedLux) + strlen(parsedMin) + strlen(parsedMax) + strlen(parsedProm) + 10;
      sprintf(todo, "%s$%s$%s$%s", parsedLux, parsedMin, parsedMax, parsedProm);
      sprintf(reply, "(%d$%d$%s)", RESPONDER_TODO, sizeReply, todo);
      break;
  }
}

float promedio()
{
  int i;
  float suma = 0;
  for(i = 0; i < cant; i++)
    suma += luxVals[i];
  return suma/cant;
}

void minYMax()
{
  int i;
  for(i = 0; i < cant; i++)
  {
    float lVal = luxVals[i];
    if(lVal < luxVals[indexMin])
      indexMin = i;
    if(lVal > luxVals[indexMax])
      indexMax = i;
  } 
}

void convertir(float resistencia){
  int termine=0;
  int i=0;
   while((!termine)&&(i<9)){
    if(tabla[0][i]<=resistencia)
        termine=1;
     else i++;      
   }
  float resultado=0.0;
   if(i==0)
       resultado=tabla[1][0];
    else { if(i==9)
              resultado=tabla[1][8];
           else {
                int anterior=tabla[0][i];
                int luxanterior=tabla[1][i];
                int siguiente=tabla[0][i-1];
                int luxsiguiente=tabla[1][i-1];   
                resultado=map(resistencia*100,anterior*100,siguiente*100,luxanterior*100,luxsiguiente*100);
                resultado=resultado/100;
           }
    }
    
   lux=resultado;
}

void getLuxVals()
{
 if(timer){
  int valorLeido = analogRead(A1);
  float voltaje=(5/1024.0)*valorLeido;
  float R1=((5.0-voltaje)*10.0)/voltaje;
  convertir(R1);
  
  cant++;
  cant = min(cant, 200);
  actualLux = lux;
  luxVals[index] = lux;
  index = (index + 1) % 200;
  timer = 0;
  }
}
ISR(TIMER2_COMPA_vect)
{
  count++;
  if(count == 6)
  {
    timer = 1;
    count = 0;
  }
}
