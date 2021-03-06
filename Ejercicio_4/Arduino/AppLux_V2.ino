#include "driver_teclado.h"
#include "Arduino.h"
#include "driver_adc.h"
#include <LiquidCrystal.h>
#include <Wire.h>

//DEFINO TIPOS DE MENSAJES
#define OBTENER_LUX 6
#define OBTENER_MIN 7
#define OBTENER_MAX  8
#define OBTENER_PROM 9
#define OBTENER_TODO 10
#define RESPONDER_LUX 11
#define RESPONDER_MIN 12
#define RESPONDER_MAX 13
#define RESPONDER_PROM 14
#define RESPONDER_TODO 15

const int numRows = 2;
const int numCols = 16;
int vKey;
float lux;
float luxVals[100];
float actualLux = 0.0;
char recibido[32];
char reply[32];
int indexMin = 0, indexMax = 0;
int cant;
int index = 0;
int tabla[2][9]={{92,41,24,16,10,7,5,3,1},{0,1,3,6,10,15,35,80,100}};
volatile int cuenta = 0;
volatile int timer = 1;
miestruct cfg_ADC;
int ini_ADC;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup()
{
  Serial.begin(9600);
  
  //INICIALIZO SHIELD LCD 1602------------------------------
  lcd.begin(numCols,numRows);
  analogWrite(10, 100); //Controla intensidad backlight
  lcd.setCursor(0, 0);
  lcd.print("Presione una");
  lcd.setCursor(0, 1);
  lcd.print("tecla");
  //--------------------------------------------------------

  //INICIALIZO STRUCT ADC-----------------------------------
  cfg_ADC.canal=1;
  cfg_ADC.callback=getLuxVals;
  ini_ADC=0;
  //--------------------------------------------------------

  //SETEO FUNCIONES DE CALLBACK------------------------------
  key_up_callback(&TeclaUpUp,TECLA_UP);
  key_up_callback(&TeclaDownUp,TECLA_DOWN);
  key_up_callback(&TeclaLeftUp,TECLA_LEFT);
  key_up_callback(&TeclaRightUp,TECLA_RIGHT);
  key_up_callback(&TeclaSelectUp,TECLA_SELECT);
  key_up_callback(&BotonA2Up,BOTON_A2);

  key_down_callback(&TeclaUpDown,TECLA_UP);
  key_down_callback(&TeclaDownDown,TECLA_DOWN);
  key_down_callback(&TeclaLeftDown,TECLA_LEFT);
  key_down_callback(&TeclaRightDown,TECLA_RIGHT);
  key_down_callback(&TeclaSelectDown,TECLA_SELECT);
  key_down_callback(&BotonA2Down,BOTON_A2);
  //----------------------------------------------------------

  //SETEO TIMER-------------------------------------------------
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
  //--------------------------------------------------------------

  //SETEO I2C ----------------------------------------------------
  Wire.begin(8); //SETEO LA DIRECCION
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  //--------------------------------------------------------------
  //INICIALIZO TECLADO--------------------------------------------
  vKey = -1;  
  teclado_init(&vKey);
  cant=0;
  //--------------------------------------------------------------
}


void loop()
{
  //Serial.println(vKey);
  if(timer){
    if(!ini_ADC){
      adc_init(&cfg_ADC);
      delay(2);
      ini_ADC=1;}
    adc_loop();
  }else{
    if(ini_ADC){
      teclado_init(&vKey);
      delay(2);
      ini_ADC=0;}
    teclado_loop();
  }
  receiveCommand();//SOLO PARA TESTING POR SERIAL 
}
//PARA TESTING POR SERIAL SOLAMENTE -----------------------------------------------
void limpiarBUF(){
  for(int w=0; w<32; w++)
  recibido[w]='\0';
}
void receiveCommand()
{
  //Serial.println("entre");
  int recvInProgress = 0;
  int i = 0;
  int readData = 0;
  int saltear=0;
  recibido[i]='\0';
  int termine=0;
  while(Serial.available()&&!termine)
  {
    readData = 1;
    char c = Serial.read(); // receive byte as a character
    delay(3);
    if (recvInProgress)
    {
      if (c == ')') 
      {
        if(!saltear){
          recvInProgress=0;
          termine=1;
          recibido[i] = '\0';
          break;
        }else{
           recibido[i] = c;
           i++;
           saltear=0;}   
      }else
          if( c == '/')
            saltear=1;
          else
            if(c == '('){
              if(!saltear){
                i=0;
                limpiarBUF();}
               else{
                recibido[i] = c;
                i++;
                saltear=0;
               }            
          }else{
            recibido[i] = c;
            i++;
          }
    }
    else
      if (c == '(')
      {
        recvInProgress = 1;
      }
  }
  if(!termine){
    limpiarBUF();
    readData=0;
    while(Serial.available())
      Serial.read();
  }
  Serial.flush(); //LIMPIO BASURA QUE HAYA QUEDADO EN EL SERIAL
  if(readData){
    parseAndReply();
    Serial.print("Recibido: ");
    Serial.println((char*)recibido);}
    //obtComandos();
}
//----------------------------------------------------------------------------------

//EVENTO DE LA COMUNICACION I2C-----------------------------------------------------
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
        recibido[i] = c;
        i++;     
      }
      else
      {
        recibido[i] = '\0'; // terminate the string
        recvInProgress = 0;
      }
    }
    else
      if (c == '(')
      {
        recvInProgress = 1;
      }
  }
  //Serial.println(recibido);
  parseAndReply();
  //obtComandos();
}
//-------------------------------------------------------------------------------------
void parseAndReply()
{
  char delim[2] = "$";
  char *str = recibido;
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
      vKey=-1;
      break;
    case OBTENER_MIN:
      minYMax();
      strMin = String(luxVals[indexMin], 2);
      strMin.toCharArray(parsedMin, 10);
      sizeReply = strlen(parsedMin) + 7;
      sprintf(reply, "(%d$%d$%s)", RESPONDER_MIN, sizeReply, parsedMin);
      vKey=-1;
      break;
    case OBTENER_MAX:
      minYMax();
      strMax = String(luxVals[indexMax], 2);
      strMax.toCharArray(parsedMax, 10);
      sizeReply = strlen(parsedMax) + 7;
      sprintf(reply, "(%d$%d$%s)", RESPONDER_MAX, sizeReply, parsedMax);
      vKey=-1;
      break;
    case OBTENER_PROM:
      prom = promedio();
      strProm = String(prom, 2);
      strProm.toCharArray(parsedProm, 10);
      sizeReply = strlen(parsedProm) + 7;
      sprintf(reply, "(%d$%d$%s)", RESPONDER_PROM, sizeReply, parsedProm);
      vKey=-1;
      break;      
    case OBTENER_TODO:
      minYMax();
      prom = promedio();
      Serial.print("Promedio: ");
      Serial.println(prom);
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
      vKey=-1;
      break;
     case TECLA_RIGHT:
      vKey=0;
      break;
     case TECLA_UP:
      vKey=1;
      break;
     case TECLA_DOWN:
      vKey=2;
      break;
     case TECLA_LEFT:
      vKey=3;
      break;
     case TECLA_SELECT:
      vKey=4;
      break;
     case BOTON_A2:
      vKey=5;
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
//FUNCIONES DE CALLBACK ADC------------------------------------------------
void getLuxVals()
{
  
 if(timer){
  
  int valorLeido = cfg_ADC.analogValue;
  float voltaje=(5/1024.0)*valorLeido;
  
  float R1=((5.0-voltaje)*10.0)/voltaje;
  convertir(R1);
  
  cant++;
  cant = min(cant, 100);
  actualLux = lux;
  luxVals[index] = lux;
  index = (index + 1) % 100;
  timer = 0;
  
  }
}
//FUNCIONES DE CALLBACK TECLADO-------------------------------------------
void TeclaUpUp(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla UP soltada");
  Serial.println("Tecla UP");
  
}
void TeclaUpDown(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla UP pulsada");
}
void TeclaDownUp(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla DOWN soltada");
  Serial.println("Tecla Down");
}
void TeclaDownDown(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla DOWN pulsada");
}
void TeclaRightUp(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla RIGHT soltada");
}
void TeclaRightDown(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla RIGHT pulsada");
  Serial.println("Tecla Right");
}
void TeclaLeftUp(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla LEFT soltada");
}
void TeclaLeftDown(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla LEFT pulsada");
  Serial.println("Tecla Left");
}
void TeclaSelectUp(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla SELECT soltada");
}
void TeclaSelectDown(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Tecla SELECT pulsada");
  Serial.println("Tecla Select");
}
void BotonA2Up(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Boton A2 soltado");
}
void BotonA2Down(){
  lcd.setCursor(0, 1);      //line=1, x=0
  lcd.print("Boton A2 pulsada");
  Serial.println("Tecla A2");
}
//--------------------------------------------------------------------------------------

//INTERRUPCION TIMER--------------------------------------------------------------------
ISR(TIMER2_COMPA_vect)
{
  cuenta++;
  if(cuenta == 6)
  {
    timer = 1;
    cuenta = 0;
  }
}

