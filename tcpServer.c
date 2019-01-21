#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "header.h"
#include "frameConstruction.h"

void serverImpl(int s);

int main (int argc, char * argv[]){

	int sListen, s;
	int sockaddrSize;
	struct sockaddr_in serverData, clientData;

	//Limpiar las estructuras que vamos a usar durante el programa
	// NOTA: esto lo hacemos para evitar problemas por los campos que no rellenamos y dejamos por defecto
	memset (&clientData, 0, sizeof(clientData));
	memset (&serverData, 0, sizeof(serverData));

	//Creación del socket de escucha
	//  Con esta operación no estamos conectando, sino diciéndole al sistema operativo que nos asigne un socket
	//   en este caso de TCP (SOCK_STREAM), y para trabajar con IPv4 (AF_INET), el cual usaremos para realizar 
	//   conexiones más tarde
	if(-1 == (sListen= socket (AF_INET, SOCK_STREAM, 0))){
		exit(EXIT_FAILURE);
	}

	//Rellenar la estructura del servidor 
	//  NOTA: en familia establecemos que trabajaremos con IPv4 (AF_INET), y en la dirección IP del servidor,
	//     establecemos INADDR_ANY, la cual es una dirección especial, que se corresponde con cualquiera,
	//     permitiéndonos no tener que hallar el valor de la dirección IP de nuestro servidor para configurar
	//     esta estructura.
	//  NOTA2: en puerto hemos puesto un valor determinado, y no 0, porque no queremos que el sistema operativo 
	//      nos asigne un puerto disponible, sino que queremos escuchar en un puerto establecido por nosotros,
	//      para que los clientes sepan con que puerto conectar.
	serverData.sin_family = AF_INET;
	serverData.sin_addr.s_addr = INADDR_ANY;
	serverData.sin_port = htons(PORT);

	//Bindear el socket a nuestra dirección
	// NOTA: esto es para unir el socket a los datos de nuestra IP, y el puerto que hemos asignado
	if (-1 == bind(sListen, (const struct sockaddr *) &serverData, sizeof(serverData))) {
		exit(EXIT_FAILURE);
	}

	//Creamos la cola de clientes en escucha
	//  NOTA: esto crea una cola, para que los clientes según van realizando peticiones, vayan siendo
	//     encolados, para ser atendidos posteriormente
	//     En este caso hemos establecido 5, que es el número máximo de clientes en espera, ya que solo se
	//     pueden asignar valores entre 1 y 5.
	if (-1 == listen(sListen, 5)) {
		exit(EXIT_FAILURE);
	}

	//A partir de aquí, ya hemos terminado de crear el socket de escucha para el servidor, y entramos en el
	//  bucle, en el cual iremos recibiendo y tratando las peticiones
	while(1){
		//Bloquearse hasta recibir una conexión
		// NOTA: Con accept, nos bloqueamos hasta recibir una conexión de un cliente, y para comunicarnos
		//    con el, crea un nuevo socket, y rellena clientData, con los datos del cliente para que podamos
		//    consultar dichos datos.
		if(-1 == (s= accept(sListen, (struct sockaddr *) &clientData, &sockaddrSize))){
			exit(EXIT_FAILURE);
		}

		//A partir de aqui viene la implementacion del servidor
		// cuando recibe una peticion la atiende, y cuando termina, espera por la siguiente
		serverImpl(s);

		//cerrar el socket creado por accept
		close(s);
	}
}

void serverImpl(int s){
	MessageType type;
	int end, contentSize, frameSize;
	char buffer[BUFFER_SIZE], content[MAX_MSG_CONTENT_SIZE];

	fprintf(stdout,"\n[SERVIDOR]Sesion iniciada");

	//Bucle para atender la sesion 
	//   Se entra en el bucle, y solo se sale cuando el cliente nos 
	//   manda un mensaje de end 
	end = 0;
	do{
		//recibir mensaje del cliente:
		//Limpiar el buffer y recibir
		memset(buffer,0,BUFFER_SIZE);
		frameSize = recv(s, buffer, BUFFER_SIZE, 0);
		if(-1 == frameSize){
			fprintf(stderr, "\n[SERVIDOR]ERROR: error en rcv");
		}

		//obtenter el tipo del mensaje
		type = getMessageType(buffer);

		switch(type){
			//si es un mensaje normal, se lee y se responde
			case MSG_FRAME: 
				//Limpiar el buffer y obtener el mensaje
				memset(content,0,MAX_MSG_CONTENT_SIZE);
				contentSize = getMsgFrameContent(buffer, content, frameSize);
				fprintf(stdout,"\n[SERVIDOR]Recibido: %s| Tam:%d",content,contentSize);

				//Leer la respuesta
				fprintf(stdout,"\n[SERVIDOR]Introduzca mensaje de respuesta: ");
				memset(content,0,MAX_MSG_CONTENT_SIZE);
				contentSize = scanf(" %[^\n]s",content);
				if(contentSize > MAX_MSG_CONTENT_SIZE)
					fprintf(stdout, "\n[SERVIDOR]ATENCION: solo se enviaran los primeros %d caracteres",MAX_MSG_CONTENT_SIZE);

				//Construir la trama de mensaje de respuesta, y enviarla
				frameSize = buildMsgFrame(buffer,content);
				if(-1 == send(s, buffer, frameSize, 0)){
					fprintf(stderr, "\n[SERVIDOR]ERROR: error en rcv");
				}
			break;
			//si es un mensaje de terminacion, se finaliza el bucle
			case END_FRAME:
				end = 1;
			break;
			//si no se conoce el tipo se sale del bucle
			default:
				fprintf(stderr,"[SERVIDOR]ERROR: trama desconocida");
				end = 1;
		}
	}while(!end);

	fprintf(stdout,"\n[SERVIDOR]Sesion terminada");
	fflush(stdout);
}