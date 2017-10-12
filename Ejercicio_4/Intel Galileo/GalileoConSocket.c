#include "mraa.hpp"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
using namespace std;

#define TECLA_RIGHT 0
#define TECLA_UP 1
#define TECLA_DOWN 2
#define TECLA_LEFT 3
#define TECLA_SELECT 4
#define BOTON_A2 5
#define OBTENER_TODO 10
#define RESPONDER_TODO 15

#define numCommands 6
#define port 3400

/*CREACION DE UN SOCKET
 * La secuencia resumida de llamadas al sistema es:
 * 
 * 1. --> socket()
 * 2. --> bind()
 * 3. --> listen()
 * 4. --> accept()
 */

int handleError(const char *s)
{
	printf("%s: %s\n", s, strerror(errno));
	exit(1);
}

void ArduinoToProcessing(char* rep)
{
	char* sub;
	int index;
	char *aux;

	//Elimino hasta el segundo '$'
	aux = rep + 6;

	//Elimino parentesis final
	sub = strchr(aux, ')');
	index = (int)(sub - aux);
	aux[index] = '\0';
	strcpy(rep, aux);
}

int main()
{
	mraa::Gpio* d_pin = NULL;
	d_pin = new mraa::Gpio(3, true, true);
	mraa::I2c* i2c;
	i2c = new mraa::I2c(0);
    i2c->address(8);

    char request[32];
    char clientIP[16];
    uint8_t rx_tx_buf[32];
    int tipo = OBTENER_TODO;
    int tam = 6;

	int sockfd, newsockfd, sin_size, numbytes;
	struct sockaddr_in sv_addr;
	struct sockaddr_in cl_addr;
	//INICIALIZO SOCKET
	//
	//DECLARACION --> int socket(int domain,int type,int protocol)
	//
	//DOMAIN --> AF_INET O AF_UNIX
	//AF_INET --> Para usar protocolos ARPA de internet
	//AF_UNIX --> Para crear sockets de comunicacion interna del sistema
	//
	//
	//TYPE --> SOCK_STREAM O SOCK_DGRAM
	//SOCK_STREAM --> Setea que la clase de socket sea de tipo Stream (de flujo)
	//SOCK_DGRAM  --> Setea que la clase de socket sea de Datagramas
	//
	//PROTOCOL -->PROXIMAMENTE
	//-->Por el momento asignarle 0
	//
	// En caso de que la funcion de creacion del socket nos devuelva -1, nos indica un error.
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		handleError("Error al crear el socket");
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(port);
	sv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(sv_addr.sin_zero), 8);
	int op = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&op, sizeof(op));
	/*BIND
	 * 
	 * -->Su función esencial es asociar un socket con un puerto
	 * -->devolverá -1 en caso de error. 
	 * 
	 * DECLARACION --> int bind(int fd, struct sockaddr *my_addr,int addrlen)
	 * 
	 * FD --> Es el descriptor de fichero socket devuelto por la llamada a socket(). 
	 * 
	 * MY_ADDR --> es un puntero a una estructura sockaddr
	 * 
	 * ADDRLEN --> contiene la longitud de la estructura sockaddr a la cuál apunta el puntero my_addr. Se debe establecer como sizeof(struct sockaddr).
	 * 
	*/
	if(bind(sockfd, (struct sockaddr*)&sv_addr, sizeof(struct sockaddr)) < 0)
		handleError("Error en el bind");
	/*	LISTEN
	 * 
	 * La función listen() se usa si se están esperando conexiones entrantes
	 * --> listen() devolverá -1 en caso de error 
	 * 
	 * DECLARACION --> int listen(int fd,int backlog)
	 * 
	 * FD -->  Es el fichero descriptor del socket, el cual fue devuelto por la llamada a socket()
	 * 
	 * BACKLOG --> Es el número de conexiones permitidas
	 */
	if(listen(sockfd, 1) < 0)
		handleError("Error en el listen");
	
	
	printf("Esperando conexion..\n");
	/*
	 * ACCEPT
	 * 
	 * Acepta una solicitud de conexion entrante (cliente usa "connect()")
	 * --> 
	 * 
	 * FD --> Es el fichero descriptor del socket, que fue devuelto por la llamada a listen(). 
	 * 
	 * ADDR --> Es un puntero a una estructura sockaddr_in en la quel se pueda determinar qué nodo nos está contactando y desde qué puerto
	 * 
	 * ADDRLEN --> Es la longitud de la estructura a la que apunta el argumento addr, por lo que conviene establecerlo como sizeof(struct sockaddr_in)
	 * 
	 * 
	 */
	sin_size = sizeof(struct sockaddr_in);
	if((newsockfd = accept(sockfd, (struct sockaddr*)&cl_addr, (socklen_t*)&sin_size)) < 0)
		handleError("Error al aceptar conexion");
	sprintf(clientIP, "%s", inet_ntoa(cl_addr.sin_addr));
	printf("Conexion establecida con %s\n", clientIP);

	
	for(;;)
	{

		//RECIBO DE PROCESSING EL TIPO DEL MENSAJE
		if((numbytes = recv(newsockfd, &tipo, sizeof(int), 0)) < 0)
			handleError("Error al recibir datos");

		//SI EL TIPO DEL MENSAJE NO ES EL DE OBTENER_TODO (SIMULAR TECLA)
		if(tipo != OBTENER_TODO)
		{
			sprintf(request, "(%i$$)", tipo);
			i2c->write((uint8_t*)request, strlen(request));
			tipo=OBTENER_TODO; //Para armar paquete de OBTENER_TODO
		}

		//ARMO EL MENSAJE A ENVIAR AL ARDUINO
		sprintf(request, "(%d$%d$)", tipo, tam);

		//ENVIO EL MENSAJE A ARDUINO POR I2C
		i2c->write((uint8_t*)request, tam);


		//RECIBO LOS VALORES ENVIADOS POR ARDUINO I2C
		//sleep(1);
		i2c->read(rx_tx_buf, 32);

		//sprintf((char*)rx_tx_buf, "(%d$%d$%.2f$%.2f$%.2f$%.2f)sdd", RESPONDER_TODO, 30, actual, min, max, prom);
		//printf("Recibido: %s\n", toProcessing);
		printf("Lei: %s",rx_tx_buf);
		ArduinoToProcessing((char*)rx_tx_buf);

		//ENVIO VALORES RECIBIDOS A PROCESSING
		if(send(newsockfd, rx_tx_buf, strlen((char*)rx_tx_buf), 0) < 0)
			handleError("Error al enviar mensaje");


	}

	return 0;
}
