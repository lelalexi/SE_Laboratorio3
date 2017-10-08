#include "driver_teclado.h"
#include "Arduino.h"
#include <LiquidCrystal.h>
#include <Wire.h>

const int numRows = 2;
const int numCols = 16;
int vKey;
char comandos[6][7] = {"right", "up", "down", "left", "select", "A2"};
char recibido[7];

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup()
{
  Serial.begin(9600);
  lcd.begin(numCols,numRows);
  analogWrite(10, 100); //Controla intensidad backlight
  lcd.setCursor(0, 0);
  lcd.print("Presione una");
  lcd.setCursor(0, 1);
  lcd.print("tecla");

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
  
  Wire.begin(8); //SETEO LA DIRECCION
  Wire.onReceive(receiveEvent);
  
  vKey = -1;
  teclado_init(&vKey);
}

void loop()
{
  receiveCommand();
  teclado_loop();
}
//PARA TESTING POR SERIAL SOLAMENTE -----------------------------------------------
void receiveCommand()
{
  int recvInProgress = 0;
  int i = 0;
  int readData = 0;
  while(Serial.available())
  {
    readData = 1;
    char c = Serial.read(); // receive byte as a character
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
  if(readData)
    obtComandos();
}
//----------------------------------------------------------------------------------

//EVENTO DE LA COMUNICACION I2C-----------------------------------------------------
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
  
  obtComandos();
}
//-------------------------------------------------------------------------------------
void obtComandos()
{
  int i = 0;
  while(i < 6 && strcmp(recibido, comandos[i]) != 0)
    i++;
  
  if(i < 6)
    vKey = i;
  else 
    vKey = -1; 
}

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
