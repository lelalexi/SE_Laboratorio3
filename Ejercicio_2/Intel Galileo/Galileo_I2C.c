#include "mraa.hpp"

#include <iostream>
using namespace std;

#define OBTENER_LUX 0
#define OBTENER_MIN 1
#define OBTENER_MAX	2
#define OBTENER_PROM 3
#define OBTENER_TODO 4
#define RESPONDER_LUX 5
#define RESPONDER_MIN 6
#define RESPONDER_MAX 7
#define RESPONDER_PROM 8
#define RESPONDER_TODO 9

void parseReply(char* rep)
{
	//printf("Rep: %s\n", rep);
	char delim[1];
	strcpy(delim, "$");
	char *token;
	char* sub;

	//Elimino parentesis
	rep++;
	sub = strchr(rep, ')');
	//printf("Rep: %s\n", rep);
	int indBracket = (int)(sub - rep);
	rep[indBracket] = '\0';

	//Obtengo tipo, tamaño y payload
	token = strtok(rep, delim);
	int tipo = atoi(token);
	token = strtok(NULL, delim);
	int tam = atoi(token); //YA QUE TAMAÑO ES FIJO 	(EN ESTE CASO)
	token = strtok(NULL, delim);
	char *payload = token;

	//Segun tipo de mensaje, imprimo respuesta
	switch(tipo)
	{
		case RESPONDER_LUX:
			printf("Actual: %s\n", payload);
			break;
		case RESPONDER_MIN:
			printf("Minimo: %s\n", payload);
			break;
		case RESPONDER_MAX:
			printf("Maximo: %s\n", payload);
			break;
		case RESPONDER_PROM:
			printf("Promedio: %s\n", payload);
			break;
		case RESPONDER_TODO:
			printf("Actual: %s | ", token);
			token = strtok(NULL, delim);
			printf("Minimo: %s | ", token);
			token = strtok(NULL, delim);
			printf("Maximo: %s | ", token);
			token = strtok(NULL, delim);
			printf("Promedio: %s\n", token);
			break;
	}

}


int main()
{
	mraa::Gpio* d_pin = NULL;
	d_pin = new mraa::Gpio(3, true, true);
    mraa::I2c* i2c;
    i2c = new mraa::I2c(0);
    i2c->address(8);

	char request[32];
	uint8_t rx_tx_buf[32];
	int tipo;
	int tam = 6;

	for(;;)
	{
		cin >> tipo;
		sprintf(request, "(%d$%d$)", tipo, tam);
		i2c->write((uint8_t*)request, tam);
		d_pin->write(0);

		sleep(1);
		i2c->read(rx_tx_buf, 32);
		d_pin->write(1);
		parseReply((char*)rx_tx_buf);
		//printf("%s\n", (char *) rx_tx_buf);

		fflush(stdout);
	}
	return 0;
}
