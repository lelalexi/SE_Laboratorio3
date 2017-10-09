#include "mraa.hpp"
#include <iostream>
using namespace std;

#define numComandos 6

int buscarComando(char *cmd, const char **list)
{
	int i;
	for(i = 0; i < numComandos; i++)
		if(strcmp(cmd, list[i]) == 0) //COMPARO SI ALGUNOS DE LOS COMANDOS DEFINIDOS COINCIDE CON EL LEIDO POR CONSOLA
			return i; //ES VALIDO
	return -1; //NO ES VALIDO
}
int main()
{
	mraa::Gpio* d_pin = NULL;
	d_pin = new mraa::Gpio(3, true, true);
    mraa::I2c* i2c;
    i2c = new mraa::I2c(0);
    i2c->address(8);


	const char *comandos[numComandos] = {"TECLA_RIGHT", "TECLA_UP", "TECLA_DOWN", "TECLA_LEFT", "TECLA_SELECT", "BOTON_A2"};
	int valid = -1;
	string line;
	char request[8];
	for(;;)
	{
		d_pin->write(0);
		printf("Ingrese una tecla: ");
		while((valid==-1))
		{
			getline(cin, line); //PARA LEER DE CONSOLA
			valid = buscarComando((char*)line.c_str(), comandos);
		}
		sleep(1);

		sprintf(request, "(%i$%i$)", valid,6);
		valid = -1;
		i2c->write((uint8_t*)request, strlen(request));
		d_pin->write(1);

		//fflush(stdout);
	}
	return 0;
}
