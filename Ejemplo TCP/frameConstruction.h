#include <stdio.h>
#include <stdlib.h>

#ifndef __FRAME_CONSTRUCTION_H
#define __FRAME_CONSTRUCTION_H

/*
	PROTOCOLO:
		
		CLIENTE:
		while(Cliente quiera){
			Cliente envia un mensaje 
			Esperar respuesta Servidor
		}

		SERVIDOR:
		while(1){
			esperar conexion 
			while(no se reciba mensaje de terminacion (tipo=2)){
				Leer mensaje del cliente
				Enviar respuesta
			}
		}

	TRAMAS:

		Mensaje normal 
			|0|1|mensaje (N bytes)|0| -- total: 3+N bytes

		Mensaje de terminacion 
			|0|2| -- total: 2 bytes
*/


//Para trabajar con los tipos de mensajes; NO PARA PONERLO EN LA CABECERA
typedef enum {MSG_FRAME, END_FRAME, UNKNOWN_FRAME} MessageType;

//Obtiene el tipo de mensaje
MessageType getMessageType(char * buffer);

//Estas funciones construyen una trama y  devuelven el tamano
int buildMsgFrame(char * buffer, char * content);
int buildEndFrame(char * buffer);

//Esta funcion obtiene el contenido de una trama y devuelve el tamano de este
int getMsgFrameContent(char * buffer, char * content, int msgSize);

#endif