
#ifndef __HEADER_H
#define __HEADER_H

//puerto en el que trabajara nuestro servidor
#define PORT 8008

//tamano del buffer para recibir mensajes -- mas grande que cualquier mensaje posible
#define BUFFER_SIZE 128

//tamano maximo del mensaje a mandar -- definido por mi, no viene definido en el protoclo, 
//      ya que este valor es decidido por el programador
#define MAX_MSG_CONTENT_SIZE 100

#endif