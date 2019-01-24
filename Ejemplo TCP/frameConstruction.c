#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include <arpa/inet.h>
#include "frameConstruction.h"

#define MSG_HEADER 1
#define END_HEADER 2

#define FRAME_SIZE(size) ( (size) + 3 )
#define MSG_SIZE(size) ( (size) - 3)

MessageType getMessageType(char * buffer){
	uint16_t header;

	//obtener la cabecera
	memcpy(&header,buffer,sizeof(uint16_t));
	header = ntohs(header);

	switch(header){
		case MSG_HEADER: 
			return MSG_FRAME;
		case END_HEADER:
			return END_FRAME;
		default: 
			return UNKNOWN_FRAME;
	}
}

int buildMsgFrame(char * buffer, char * content){
	uint16_t header = htons(MSG_HEADER);

	//limpiar el buffer
	memset(buffer,0,BUFFER_SIZE);
	//escribir en el buffer el valor de la cabecera y el mensaje
	memcpy(buffer,&header,sizeof(uint16_t));
	strcpy(&buffer[2],content);

	//devolver el tamano de la trama
	return FRAME_SIZE(strlen(content));
}

int buildEndFrame(char * buffer){
	uint16_t header = htons(END_HEADER);

	//limpiar el buffer
	memset(buffer,0,BUFFER_SIZE);
	//escribir en el buffer el valor de la cabecera
	memcpy(buffer,&header,sizeof(uint16_t));

	//devolver el tamano de la trama
	return 2;
}

int getMsgFrameContent(char * buffer, char * content, int msgSize){
	//copiar el tamano
	memcpy(content,&buffer[2],msgSize-3);
	//devolver el tamano de la trama
	return MSG_SIZE(msgSize);
}