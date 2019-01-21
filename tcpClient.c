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
#include <ctype.h> 
#include "header.h"
#include "frameConstruction.h"

void clientImpl(int s);

int main (int argc, char * argv[]){
	int s;
	struct addrinfo resolutionConfig, *resolutionData= NULL;
	struct sockaddr_in serverData;	

	//Limpiar las estructuras que vamos a usar durante el programa
	// NOTA: esto lo hacemos para evitar problemas por los campos que no rellenamos y dejamos por defecto
	memset (&resolutionConfig, 0, sizeof (resolutionConfig));
	memset (&serverData, 0, sizeof(serverData));
			
	//Resolución de la dirección IP del host a conectar
	// NOTA: para realizar la conexión necesitamos la IP, no el nombre de dominio, por eso necesitamos 
	//  realizar una resolución, para obtener dicha dirección a partir del nombre del servidor.
	resolutionConfig.ai_family = AF_INET;
	if(0 != getaddrinfo (argv[1], NULL, &resolutionConfig, &resolutionData)){
		exit(EXIT_FAILURE);
	}

	//Creación del socket
	//  Con esta operación no estamos conectando, sino diciéndole al sistema operativo que nos asigne un socket
	//   en este caso de TCP (SOCK_STREAM), y para trabajar con IPV4 (AF_INET), el cual usaremos para realizar 
	//   conexiones más tarde
	if(-1 == (s = socket (AF_INET, SOCK_STREAM, 0))){
		exit(EXIT_FAILURE);
	}
			
	//Rellenar la estructura del servidor
	//  NOTA: en familia establecemos que trabajaremos con IPv4 (AF_INET), en la dirección la IP del servidor 
	//    con el que queremos conectar (está en la estructura con la que hicimos la resolución)
	serverData.sin_family = AF_INET;
	serverData.sin_addr = ((struct sockaddr_in *) resolutionData->ai_addr)->sin_addr;
	serverData.sin_port = htons(PORT);

	//Una vez rellena la estructura del servidor, podemos liberar la memoria
	//  utilizada para la resolución de su dirección IP
	freeaddrinfo(resolutionData);
	resolutionData = NULL;

	//Establecimiento de la conexión con el servidor
	//  NOTA: aquí estamos estableciendo la conexion en si de TCP 
	//     (Cliente: SYN -> Servidor: SYN,ACK -> Cliente: ACK), tras lo cual, se podrá comenzar
	//     a intercambiar mensajes.
	if(-1 == connect(s, (const struct sockaddr *)&serverData, sizeof(serverData))){
		exit(EXIT_FAILURE);
	}
		
	//A partir de aquí viene la implementación del cliente
	clientImpl(s);

	//cerrar la conexion y liberar el socket
	close(s);
}


void clientImpl(int s){
	char response;
	int end, frameSize,contentSize;
	char buffer[BUFFER_SIZE], content[MAX_MSG_CONTENT_SIZE];
	MessageType type;

	//Bucle para gestionar la conversacion 
	end = 0;
	do{
		//Limpiar el buffer, pedir el mensaje, e indicar que solo se enviaran los primeros N caracteres
		memset(content,0,MAX_MSG_CONTENT_SIZE);
		fprintf(stdout,"\n[CLIENTE]Introduzca mensaje: ");
		fflush(stdin);
		contentSize = scanf(" %[^\n]s",content);
		if(contentSize > MAX_MSG_CONTENT_SIZE)
			fprintf(stdout, "\n[CLIENTE]ATENCION: solo se enviaran los primeros %d caracteres",MAX_MSG_CONTENT_SIZE);

		//Enviar el mensaje
		frameSize = buildMsgFrame(buffer,content);
		if(-1 == send(s, buffer, frameSize, 0)){
			fprintf(stderr, "\n[CLIENTE]ERROR: error en rcv");
		}

		//Recibir respuesta del servidor
		memset(buffer,0,BUFFER_SIZE);
		frameSize = recv(s, buffer, BUFFER_SIZE, 0);
		if(-1 == frameSize){
			fprintf(stderr, "\n[CLIENTE]ERROR: error en rcv");
		}

		//Obtener el tipo de trama
		type = getMessageType(buffer);

		//si es una trama de mensaje, se imprime el emnsaje
		if(MSG_FRAME == type){
			memset(content,0,MAX_MSG_CONTENT_SIZE);
			contentSize = getMsgFrameContent(buffer, content, frameSize);
			fprintf(stdout,"\n[CLIENTE]Recibido: %s| Tam:%d",content, contentSize);
		}else{
			fprintf(stdout,"[CLIENTE]ERROR: trama desconocida");
		}

		//preguntar si se quiere continuar
		fprintf(stdout, "\n[CLIENTE]Enviar otro mensaje? (S/N)");
		do{
			fflush(stdin);
			scanf(" %c",&response);
		}while(toupper(response) != 'N' && toupper(response) !='S');

		//Si se quiere acabar se pone a 1 el flag de terminar y se envia un mensaje de tipo END
		if('N' == toupper(response) ){
			end = 1;
			frameSize = buildEndFrame(buffer);
			if(-1 == send(s, buffer, frameSize, 0)){
				fprintf(stderr, "\n[CLIENTE]ERROR: error en rcv");
			}
		}

	}while(!end);
}